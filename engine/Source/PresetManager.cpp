#include "PresetManager.h"

void PresetManager::scanPresets (const juce::File& presetsDir, const juce::File& shadersDir)
{
    shadersDirectory = shadersDir;
    presets.clear();

    if (! presetsDir.isDirectory())
    {
        DBG ("PresetManager: presets directory not found: " << presetsDir.getFullPathName());
        return;
    }

    auto jsonFiles = presetsDir.findChildFiles (juce::File::findFiles, false, "*.json");
    jsonFiles.sort();

    for (auto& file : jsonFiles)
    {
        bool ok = false;
        juce::String error;
        auto preset = Preset::loadFromFile (file, ok, error);

        if (ok)
            presets.add (preset);
        else
            DBG ("PresetManager: failed to load " << file.getFileName() << " - " << error);
    }

    DBG ("PresetManager: loaded " << presets.size() << " preset(s) from " << presetsDir.getFullPathName());
}

bool PresetManager::selectPreset (int index, juce::OpenGLContext& context)
{
    if (presets.isEmpty())
        return false;

    index = ((index % presets.size()) + presets.size()) % presets.size();
    auto& preset = presets.getReference (index);

    if (preset.isEffectChain())
    {
        auto newChain = std::make_unique<EffectChain>();

        if (! newChain->load (preset.effectChain, shadersDirectory, context))
        {
            DBG ("PresetManager: could not load effect chain for preset '" << preset.name << "'");
            return false;
        }

        if (currentShader != nullptr)      currentShader->releaseGLObjects();
        if (currentEffectChain != nullptr) currentEffectChain->releaseGLObjects();

        currentShader.reset();
        currentEffectChain = std::move (newChain);
        currentIndex = index;

        DBG ("PresetManager: switched to preset '" << preset.name << "' (" << (index + 1) << "/" << presets.size()
             << ", " << currentEffectChain->getNumStages() << " effect stage(s))");
        return true;
    }

    auto shaderFile = shadersDirectory.getChildFile (preset.shaderFile);
    auto newShader = std::make_unique<ISFShader>();

    if (! newShader->loadFromFile (shaderFile))
    {
        DBG ("PresetManager: could not load shader for preset '" << preset.name << "': " << newShader->getLastError());
        return false;
    }

    if (! newShader->compile (context))
    {
        DBG ("PresetManager: could not compile shader for preset '" << preset.name << "': " << newShader->getLastError());
        return false;
    }

    for (auto& override : preset.paramOverrides)
        newShader->setValue (override.name.toString(), override.value);

    if (currentShader != nullptr)      currentShader->releaseGLObjects();
    if (currentEffectChain != nullptr) currentEffectChain->releaseGLObjects();

    currentEffectChain.reset();
    currentShader = std::move (newShader);
    currentIndex = index;

    DBG ("PresetManager: switched to preset '" << preset.name << "' (" << (index + 1) << "/" << presets.size() << ")");
    return true;
}

void PresetManager::nextPreset (juce::OpenGLContext& context)
{
    if (! presets.isEmpty())
        selectPreset (currentIndex + 1, context);
}

void PresetManager::previousPreset (juce::OpenGLContext& context)
{
    if (! presets.isEmpty())
        selectPreset (currentIndex - 1, context);
}

juce::String PresetManager::getCurrentName() const
{
    if (juce::isPositiveAndBelow (currentIndex, presets.size()))
        return presets.getReference (currentIndex).name;

    return "(no preset)";
}

int PresetManager::getNumEffectStages() const
{
    return currentEffectChain != nullptr ? currentEffectChain->getNumStages() : 0;
}

void PresetManager::toggleEffectStage (int stageIndex)
{
    if (currentEffectChain != nullptr)
        currentEffectChain->toggleStageBypassed (stageIndex);
}

void PresetManager::render (juce::OpenGLContext& context, float timeSeconds, int pixelWidth, int pixelHeight,
                             float level, float bass, float mid, float high, float beatphase,
                             unsigned int videoTexture)
{
    if (! juce::isPositiveAndBelow (currentIndex, presets.size()))
        return;

    if (currentEffectChain != nullptr)
    {
        currentEffectChain->render (context, videoTexture, timeSeconds, pixelWidth, pixelHeight,
                                     level, bass, mid, high, beatphase);
        return;
    }

    if (currentShader != nullptr)
    {
        auto& preset = presets.getReference (currentIndex);

        for (auto& mappingEntry : preset.audioMappings)
        {
            auto& paramName = mappingEntry.first;
            auto& mapping = mappingEntry.second;

            auto raw = resolveAudioMappingSource (mapping.source, level, bass, mid, high, beatphase);
            currentShader->setValue (paramName, raw * mapping.scale + mapping.offset);
        }

        currentShader->render (context, timeSeconds, pixelWidth, pixelHeight,
                               level, bass, mid, high, beatphase, videoTexture, 0);
    }
}

void PresetManager::releaseGLObjects()
{
    if (currentShader != nullptr)      currentShader->releaseGLObjects();
    if (currentEffectChain != nullptr) currentEffectChain->releaseGLObjects();
}
