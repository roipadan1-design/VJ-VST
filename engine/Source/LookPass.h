#pragma once

#include <JuceHeader.h>
#include "GLHelpers.h"
#include "Signals.h"

// The performer's global "look" - one set of controls that sits on top of
// every scene (OSC /v2/look <slot> <0-1>, /v2/palette r g b x3 [mix]):
//
//   slot 0 Grain     film grain + dust
//   slot 1 Crush     soft S-curve -> posterised -> hard two-tone threshold
//   slot 2 Flash     chance that a kick drops a black frame or a full-colour frame
//   slot 3 Glitch    horizontal row tears, pixel blocks, RGB split (hat/snare pulsed)
//   slot 4 Trails    frame memory with a slight push-in (motion smear)
//   slot 5 Symbols   braille-cell strips scattered over the image, re-dealt on snares
//   slot 6 Cut Rate  automatic cuts between scenes (off .. every kick), see PresetManager
//
// The 3-colour palette is a gradient map on luminance (shadow / mid / light);
// `paletteMix` 0 shows the scene's own colours.
struct LookSettings
{
    static constexpr int numSlots = 7;
    enum Slot { grain = 0, crush, flash, glitch, trails, symbols, cutRate };

    std::array<float, numSlots> values { 0.25f, 0.35f, 0.2f, 0.1f, 0.1f, 0.2f, 0.0f };
    std::array<juce::Colour, 3> palette { juce::Colour (0xff050000), juce::Colour (0xffe01008), juce::Colour (0xffff9a86) };
    float paletteMix = 1.0f;

    float get (Slot s) const noexcept { return values[(size_t) s]; }
};

// Final pass of the engine (replaces the plain output copy): trails at the
// internal resolution, then glitch -> grade -> symbols -> flash -> grain ->
// blackout gain straight into the output framebuffer.
class LookPass
{
public:
    // Call once per frame before render(): consumes this frame's hits.
    void update (const Signals& signals, const LookSettings& look, double dt);

    // Returns false if the shaders failed to build (caller falls back to a plain copy).
    bool render (juce::OpenGLContext& context, unsigned int sceneTexture, int sceneWidth, int sceneHeight,
                 unsigned int outputFbo, int outputWidth, int outputHeight,
                 const LookSettings& look, float timeSeconds, float gain);

    void release();

    // Drops the trail memory - called on every scene switch so the previous
    // scene never lingers (or smears over) the new one.
    void clearTrails() noexcept { trailsCleared = true; }

private:
    bool ensurePrograms (juce::OpenGLContext&);

    std::unique_ptr<juce::OpenGLShaderProgram> trailProgram, finalProgram;
    std::array<GLRenderTarget, 2> history;
    int historyIndex = 0;
    FullscreenQuad quad;
    bool buildFailed = false;
    double lastDt = 1.0 / 60.0;
    bool trailsCleared = false;

    juce::Random random;
    float kickEnv = 0.0f, snareEnv = 0.0f, hatEnv = 0.0f;
    float flashTimer = 0.0f, flashType = 0.0f; // type 0 black, 1 mid colour, 2 light colour
    float symbolSeed = 1.0f, glitchSeed = 1.0f;
};
