#include "PresetInstance.h"
#include "Diagnostics.h"

using namespace juce::gl;

//==============================================================================
std::unique_ptr<PresetInstance> LegacyInstance::create (const Preset& p, const juce::File& shadersDir,
                                                        juce::OpenGLContext& context, juce::String& error)
{
    std::unique_ptr<LegacyInstance> instance (new LegacyInstance (p));

    if (p.isEffectChain())
    {
        instance->chain = std::make_unique<EffectChain>();
        if (! instance->chain->load (p.effectChain, shadersDir, context))
        {
            error = "could not load effect chain";
            return nullptr;
        }
        return instance;
    }

    instance->shader = std::make_unique<ISFShader>();
    if (! instance->shader->loadFromFile (shadersDir.getChildFile (p.shaderFile)) || ! instance->shader->compile (context))
    {
        error = instance->shader->getLastError();
        return nullptr;
    }

    for (auto& o : p.paramOverrides)
        instance->shader->setValue (o.name.toString(), o.value);

    return instance;
}

void LegacyInstance::render (const FrameContext& f, const GLRenderTarget& output)
{
    if (chain != nullptr)
    {
        chain->render (f.gl, f.videoTexture, f.time, output.width, output.height,
                       f.level, f.bass, f.mid, f.high, f.beatphase, f.onset, output.fbo);
        return;
    }

    for (auto& [param, mapping] : preset.audioMappings)
    {
        auto raw = resolveAudioMappingSource (mapping.source, f.level, f.bass, f.mid, f.high, f.beatphase, f.onset);
        shader->setValue (param, raw * mapping.scale + mapping.offset);
    }

    shader->setEngineUniforms (0.0f, 0.0f, (float) f.clock.beat(),
                               (float) f.motion.sceneTime, (float) f.motion.speed, (float) f.motion.sceneDt);
    shader->render (f.gl, f.time, output.width, output.height,
                    f.level, f.bass, f.mid, f.high, f.beatphase, f.onset, f.videoTexture, output.fbo);
}

void LegacyInstance::releaseGLObjects()
{
    if (shader != nullptr) shader->releaseGLObjects();
    if (chain != nullptr)  chain->releaseGLObjects();
}

void LegacyInstance::toggleStage (int index)
{
    if (chain != nullptr)
        chain->toggleStageBypassed (index);
}

void LegacyInstance::setParam (int stageIndex, const juce::String& name, const juce::var& value)
{
    if (chain != nullptr)       chain->setStageParam (stageIndex, name, value);
    else if (shader != nullptr) shader->setValue (name, value);
}

//==============================================================================
std::unique_ptr<PresetInstance> V2Instance::create (const PresetV2& p, const juce::File& shadersDir, SourceLibrary& library,
                                                    juce::OpenGLContext& context, juce::String& error)
{
    std::unique_ptr<V2Instance> instance (new V2Instance (p, library));
    instance->random.setSeed ((juce::int64) p.seed);

    for (auto& warning : instance->modulation.getWarnings())
        logDiagnostic ("Preset '" + p.name + "': " + warning);

    for (auto& stageDef : p.stages)
    {
        auto shader = std::make_unique<ISFShader>();
        if (! shader->loadFromFile (shadersDir.getChildFile (stageDef.shader)) || ! shader->compile (context))
        {
            error = "stage '" + stageDef.id + "': " + shader->getLastError();
            return nullptr;
        }

        // Every preset parameter must exist as an ISF input, or the route
        // would silently do nothing - fail loudly at load instead.
        for (auto& param : stageDef.parameters)
        {
            bool found = false;
            for (auto& input : shader->getInputs())
                found = found || input.name == param.name;
            if (! found)
            {
                error = "stage '" + stageDef.id + "' shader has no input named '" + param.name + "'";
                return nullptr;
            }
        }

        for (auto& src : stageDef.sources)
        {
            bool isImageInput = false;
            for (auto& input : shader->getInputs())
                isImageInput = isImageInput || (input.name == src.input && input.type == "image");
            if (! isImageInput)
            {
                error = "stage '" + stageDef.id + "' has no image input named '" + src.input + "' for its source";
                return nullptr;
            }
            instance->sourceStates.add ({ instance->stages.size(), src, 0, 0 });
        }

        instance->stages.add (shader.release());
        instance->bypassed.add (false);
    }

    juce::String copyError;
    instance->copyProgram = buildFullscreenProgram (context, R"(
        uniform sampler2D source;
        void main() { gl_FragColor = vec4 (clamp (texture2D (source, uv).rgb, 0.0, 1.0), 1.0); }
    )", copyError);

    return instance;
}

void V2Instance::advanceSources (const FrameContext& f)
{
    // Freeze / Speed 0 holds the picture: no new images or words either.
    if (std::abs (f.motion.speed) < 0.02)
        return;

    for (auto& s : sourceStates)
    {
        int steps = 0;
        switch (s.def.advance)
        {
            case V2Source::Advance::beat:  steps = f.clock.crossedBeat() ? 1 : 0; break;
            case V2Source::Advance::bar:   steps = f.clock.crossedBar() ? 1 : 0; break;
            case V2Source::Advance::event:
                for (auto& e : f.signals.events)
                    steps += e.type == s.def.event ? 1 : 0;
                break;
            case V2Source::Advance::none:  break;
        }

        for (int i = 0; i < steps; ++i)
        {
            if (++s.counter < s.def.every)
                continue;
            s.counter = 0;

            auto count = s.def.type == V2Source::Type::text ? s.def.words.size()
                                                            : (int) sources.imageFolder (s.def.folder).size();
            if (count > 1)
                s.index = s.def.random ? (s.index + 1 + random.nextInt (count - 1)) % count : (s.index + 1) % count;
        }
    }
}

void V2Instance::bindSources()
{
    for (auto& s : sourceStates)
    {
        SourceLibrary::Texture tex;
        if (s.def.type == V2Source::Type::text)
        {
            tex = sources.text (s.def.words[s.index % s.def.words.size()], s.def.font);
        }
        else
        {
            auto& list = sources.imageFolder (s.def.folder);
            if (list.empty())
                continue; // shader sees black: missing media shows, never crashes
            tex = list[(size_t) (s.index % (int) list.size())];
        }
        stages[s.stage]->setImageTexture (s.def.input, tex.id, tex.width, tex.height);
    }
}

void V2Instance::render (const FrameContext& f, const GLRenderTarget& output)
{
    modulation.process (f.signals, f.clock, f.macros, f.motion, f.dt, f.now);
    advanceSources (f);
    bindSources();
    auto& values = modulation.getValues();

    int readIndex = -1; // which stageTargets entry holds the latest image
    int writeIndex = 0;
    int lastWidth = output.width, lastHeight = output.height;

    for (int s = 0; s < stages.size(); ++s)
    {
        if (bypassed[s] && s > 0)
            continue;

        auto& def = preset.stages.getReference (s);
        auto* shader = stages[s];

        for (int p = 0; p < def.parameters.size(); ++p)
            shader->setValue (def.parameters.getReference (p).name, values.getReference (s)[p]);

        shader->setEngineUniforms (modulation.getPalette(), modulation.getSeed(), (float) f.clock.beat(),
                                   (float) f.motion.sceneTime, (float) f.motion.speed, (float) f.motion.sceneDt);

        auto w = juce::jmax (1, juce::roundToInt (output.width * def.scale));
        auto h = juce::jmax (1, juce::roundToInt (output.height * def.scale));
        auto& target = stageTargets[(size_t) writeIndex];
        target.ensure (w, h, def.floatTarget ? GL_RGBA16F : GL_RGBA8);

        auto input = readIndex >= 0 ? stageTargets[(size_t) readIndex].texture : f.videoTexture;
        shader->render (f.gl, f.time, w, h, f.level, f.bass, f.mid, f.high, f.beatphase, f.onset,
                        def.generator ? f.videoTexture : input, target.fbo);

        readIndex = writeIndex;
        writeIndex = 1 - writeIndex;
        lastWidth = w;
        lastHeight = h;
    }

    if (readIndex < 0)
        return;

    auto& scene = stageTargets[(size_t) readIndex];
    if (! finish.render (f.gl, scene.texture, lastWidth, lastHeight, preset.post, f.time, output) && copyProgram != nullptr)
    {
        // Finish unavailable: a clamped copy keeps the picture on screen.
        output.bind();
        copyProgram->use();
        glActiveTexture (GL_TEXTURE0);
        glBindTexture (GL_TEXTURE_2D, scene.texture);
        copyProgram->setUniform ("source", 0);
        quad.draw (*copyProgram);
    }
}

void V2Instance::releaseGLObjects()
{
    for (auto* s : stages)
        s->releaseGLObjects();
    for (auto& t : stageTargets)
        t.release();
    finish.release();
    quad.release();
    copyProgram.reset();
}

void V2Instance::toggleStage (int index)
{
    // Stage 0 is the generator - bypassing it would leave nothing to show.
    if (index > 0 && index < bypassed.size())
        bypassed.set (index, ! bypassed[index]);
}

void V2Instance::setParam (int stageIndex, const juce::String& name, const juce::var& value)
{
    modulation.setBaseValue (stageIndex, name, (float) (double) value);
}
