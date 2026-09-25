#pragma once

#include <JuceHeader.h>
#include "GLHelpers.h"
#include "PresetInstance.h"

// Owns the preset library (legacy + schema-2 JSON files), the active preset
// instance(s) and the composition of the final frame:
//
//   current preset  -> currentTarget  \
//                                      > transition blend -> compositeTarget -> output gain/blackout -> framebuffer 0
//   outgoing preset -> outgoingTarget /       (or a frozen snapshot when a switch interrupts a fade)
//
// Switches are musical: a schema-2 preset can ask for its entrance to wait
// for the next beat or bar (transition.quantize) while the host transport is
// running, and choose cut / crossfade / dip-to-background / luma-wipe.
// Switching again mid-fade freezes the current composite as the new
// "outgoing" image, so at most two presets ever render at once.
class PresetManager
{
public:
    PresetManager() = default;
    ~PresetManager();

    void scanPresets (const juce::File& presetsDir, const juce::File& shadersDir, const juce::File& mediaDir);

    // Requests a switch; it happens inside the next render() (on the GL
    // thread), possibly deferred to a beat/bar boundary.
    void requestPreset (int index);
    void requestNext()     { requestPreset (requestedOrCurrent() + 1); }
    void requestPrevious() { requestPreset (requestedOrCurrent() - 1); }

    void render (FrameContext& frame, unsigned int finalTargetFbo);
    void releaseGLObjects();

    int getNumPresets() const noexcept { return entries.size(); }
    int getCurrentIndex() const noexcept { return currentIndex.load(); }
    int getFirstSchema2Index() const noexcept;
    juce::String getCurrentName() const;
    juce::String getPresetName (int index) const;
    bool isSchema2 (int index) const;

    // The live scene's own name for each macro slot (empty = the generic
    // name) and its one-line description - shown by the plug-in under the knobs.
    juce::StringArray getCurrentMacroLabels() const;
    int getReframeCount() const noexcept { return lookPass.getReframeCount(); }
    juce::String getCurrentDescription() const;

    void toggleEffectStage (int stageIndex);
    void setEffectParam (int stageIndex, const juce::String& name, const juce::var& value);

    // Global override of every preset's transition length (OSC
    // /preset/transitionduration); < 0 restores the per-preset values.
    void setTransitionDurationOverride (double ms) noexcept { durationOverrideMs = ms; }

    // Internal render resolution as a fraction of the output (0.4-1). The
    // composite is upscaled in the final pass - the first rung of the quality
    // ladder on integrated GPUs, it never changes timing or mappings.
    void setRenderScale (float s) noexcept { renderScale = juce::jlimit (0.4f, 1.0f, s); }
    float getRenderScale() const noexcept { return renderScale.load(); }
    int getSwitchCount() const noexcept { return switchCount.load(); }

    // Output gate: fades the final image to/from black over ~120 ms.
    void setBlackout (bool shouldBeBlack) noexcept { blackout = shouldBeBlack; }
    bool isBlackout() const noexcept { return blackout; }

private:
    struct Entry
    {
        juce::String name;
        juce::File file;
        Preset legacy;
        std::shared_ptr<PresetV2> v2;
    };

    struct Transition
    {
        V2Transition::Type type = V2Transition::Type::crossfade;
        double durationMs = 600.0, startTime = 0.0;
        bool active = false;
        bool fromFrozen = false;
    };

    int requestedOrCurrent() const noexcept { return pendingIndex >= 0 ? pendingIndex : currentIndex; }
    bool activate (int index, FrameContext& frame);
    void prebuild (int index, FrameContext& frame);
    bool ensurePrograms (juce::OpenGLContext&);

    void updateAutoCut (const FrameContext& frame, const LookSettings& look);

    juce::Array<Entry> entries;
    juce::File shadersDirectory;
    SourceLibrary sourceLibrary;

    std::unique_ptr<PresetInstance> current, outgoing;
    std::atomic<int> currentIndex { -1 };   // read by the status timer on the message thread
    int pendingIndex = -1;
    // A requested scene is built (shaders compiled) as soon as it is asked
    // for and swapped in on the grid, so the cut lands exactly on the beat.
    std::unique_ptr<PresetInstance> prebuilt;
    int prebuiltIndex = -1;
    double lastBeatTime = -10.0, lastBarTime = -10.0;
    std::atomic<int> requestedIndex { -1 };

    Transition transition;
    double durationOverrideMs = -1.0;
    std::atomic<bool> blackout { false };
    std::atomic<float> renderScale { 1.0f };
    std::atomic<int> switchCount { 0 };
    float outputGain = 1.0f;

    GLRenderTarget currentTarget, outgoingTarget, frozenTarget, compositeTarget;
    std::unique_ptr<juce::OpenGLShaderProgram> blendProgram, outputProgram;
    FullscreenQuad quad;
    bool programsFailed = false;

    LookPass lookPass;
    LookSettings defaultLook;
    // Auto-cut (look slot "Cut Rate"): counts beats and jumps to a random
    // other scene with a hard cut, bypassing the preset's own transition.
    int beatsSinceCut = 0;
    double lastKickCut = -10.0;
    bool forceCut = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetManager)
};
