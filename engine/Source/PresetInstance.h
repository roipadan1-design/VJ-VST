#pragma once

#include <JuceHeader.h>
#include "EffectChain.h"
#include "FinishPass.h"
#include "GLHelpers.h"
#include "ISFShader.h"
#include "LookPass.h"
#include "Modulation.h"
#include "Preset.h"
#include "PresetV2.h"
#include "Signals.h"
#include "SourceLibrary.h"

// Everything a preset needs to draw one frame.
struct FrameContext
{
    juce::OpenGLContext& gl;
    float time = 0.0f;       // seconds since engine start
    double now = 0.0;        // same clock, double precision
    double dt = 0.0;         // seconds since the previous frame
    const Signals& signals;
    const Clock& clock;
    const MacroBank& macros;
    const LookSettings* look = nullptr; // global look (grade, grain, cut rate...), owned by the caller
    unsigned int videoTexture = 0;
    int width = 0, height = 0;

    // Legacy uniforms (level/bass/mid/high/beatphase/onset), derived from signals.
    float level = 0, bass = 0, mid = 0, high = 0, beatphase = 0, onset = 0;
};

// A loaded, compiled preset that renders a display-referred image into a target.
class PresetInstance
{
public:
    virtual ~PresetInstance() = default;
    virtual void render (const FrameContext&, const GLRenderTarget& output) = 0;
    virtual void releaseGLObjects() = 0;
    virtual void toggleStage (int) {}
    virtual void setParam (int stageIndex, const juce::String& name, const juce::var& value) = 0;
    virtual bool usesDuoPalette() const { return false; }
};

// The original formats: one ISF shader with {source, scale, offset}
// audioMappings, or a video effectChain. Behaviour is unchanged.
class LegacyInstance : public PresetInstance
{
public:
    static std::unique_ptr<PresetInstance> create (const Preset&, const juce::File& shadersDir,
                                                   juce::OpenGLContext&, juce::String& error);

    void render (const FrameContext&, const GLRenderTarget& output) override;
    void releaseGLObjects() override;
    void toggleStage (int index) override;
    void setParam (int stageIndex, const juce::String& name, const juce::var& value) override;

private:
    explicit LegacyInstance (const Preset& p) : preset (p) {}
    const Preset& preset;
    std::unique_ptr<ISFShader> shader;
    std::unique_ptr<EffectChain> chain;
};

// Schema-2: ISF stages driven by a ModulationRuntime, rendered linear/HDR
// into float targets and finished by the engine (bloom, tone map, sRGB).
class V2Instance : public PresetInstance
{
public:
    static std::unique_ptr<PresetInstance> create (const PresetV2&, const juce::File& shadersDir, SourceLibrary&,
                                                   juce::OpenGLContext&, juce::String& error);

    void render (const FrameContext&, const GLRenderTarget& output) override;
    void releaseGLObjects() override;
    void toggleStage (int index) override;
    void setParam (int stageIndex, const juce::String& name, const juce::var& value) override;
    bool usesDuoPalette() const override { return preset.post.duoPalette; }

private:
    V2Instance (const PresetV2& p, SourceLibrary& lib) : preset (p), modulation (p), sources (lib) {}

    struct SourceState
    {
        int stage = 0;
        V2Source def;
        int index = 0, counter = 0;
    };

    void advanceSources (const FrameContext&);
    void bindSources();

    SourceLibrary& sources;
    juce::Array<SourceState> sourceStates;
    juce::Random random;

    const PresetV2& preset;
    ModulationRuntime modulation;
    juce::OwnedArray<ISFShader> stages;
    juce::Array<bool> bypassed;
    std::array<GLRenderTarget, 2> stageTargets; // ping-pong between stages
    FinishPass finish;
    std::unique_ptr<juce::OpenGLShaderProgram> copyProgram;
    FullscreenQuad quad;
};
