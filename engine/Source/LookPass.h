#pragma once

#include <JuceHeader.h>
#include "GLHelpers.h"
#include "Signals.h"

// The performer's global "look" - one set of controls that sits on top of
// every scene (OSC /v2/look <slot> <0-1>, /v2/palette r g b x3 [mix]):
//
//   0 Grain      band-limited film grain, luminance-dependent, stepped at 24 fps
//   1 Crush      soft S-curve -> posterised -> hard two-tone (grain dithers the threshold)
//   2 Flash      chance a kick drops a black / colour frame (max 3 per second)
//   3 Glitch     horizontal row tears, pixel blocks, RGB split (hat/snare pulsed)
//   4 Trails     frame memory with a slight push-in (motion smear)
//   5 Symbols    braille-cell strips scattered over the image, re-dealt on snares
//   6 Cut Rate   automatic cuts between scenes (off .. every kick), see PresetManager
//   7 Smear      VHS-style per-row tracking drift in bursts, with dropout dashes
//   8 Halation   red-biased glow bleeding out of the highlights
//   9 Weave      film gate weave, frame slips, flicker and soft colour fringing
//  10 Dust       dust specks, hairs and scratches - rare and correlated, like real film
//  11 Blacks     lifted, tinted blacks (the film base)
//  12 Reactivity master amount of every audio-driven reaction (1 = as designed)
//  13 Calm       0/1: fades every audio reaction out over one bar (and back in)
//  14 Shots      cut grammar (after the un_source reel): selected strong hits
//                become short cuts - a new framing held until the next shot,
//                a 2-4 frame negative, a circle / crescent / lens / half-disc
//                punch (white, or a window onto an inverted close-up), a black
//                frame. Density grows with the knob; max 3 per second.
//  15 HUD        measuring-instrument overlay: crosshairs, small squares linked
//                by hairlines, large faint circles; re-dealt on every shot
//
// The 3-colour palette is a gradient map on luminance (shadow / mid / light);
// `paletteMix` 0 shows the scene's own colours.
struct LookSettings
{
    static constexpr int numSlots = 16;
    enum Slot { grain = 0, crush, flash, glitch, trails, symbols, cutRate,
                smear, halation, weave, dust, blacks, reactivity, calm, shots, hud };

    // Ambient defaults: film on, every digital disturbance (flash, glitch,
    // symbols, smear, auto cuts) off until the performer asks for it.
    std::array<float, numSlots> values { 0.3f, 0.35f, 0.0f, 0.0f, 0.1f, 0.0f, 0.0f,
                                         0.0f, 0.35f, 0.3f, 0.3f, 0.35f, 1.0f, 0.0f, 0.0f, 0.0f };
    std::array<juce::Colour, 3> palette { juce::Colour (0xff050000), juce::Colour (0xffe01008), juce::Colour (0xffff9a86) };
    float paletteMix = 1.0f;
    // Set per frame from the live scene (not a performer control): 1 = the
    // scene is two layers (red body, green detail) - see V2Post::duoPalette.
    float duo = 0.0f;
    // Set per frame from MOVE (not look slots): Drift amount and the scene
    // clock it runs on (so Freeze / Speed 0 stop the drift too).
    float drift = 0.0f, driftTime = 0.0f;

    float get (Slot s) const noexcept { return values[(size_t) s]; }
};

// Final pass of the engine (replaces the plain output copy):
//   trails (internal res) -> halation extract + blur (quarter res)
//   -> film composite straight into the output framebuffer:
//      weave -> smear/glitch -> fetch + fringing -> acutance -> grain (before
//      the threshold) -> flicker -> crush -> palette -> lifted blacks ->
//      halation -> symbols -> dust/hairs/scratches -> flash -> blue-noise dither
class LookPass
{
public:
    // Call once per frame before render(): consumes this frame's hits.
    void update (const Signals& signals, const LookSettings& look, double dt, double timeSeconds);

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
    void ensureNoiseTextures();

    std::unique_ptr<juce::OpenGLShaderProgram> trailProgram, extractProgram, blurProgram, finalProgram;
    std::array<GLRenderTarget, 2> history;
    std::array<GLRenderTarget, 2> halo; // quarter res ping-pong
    int historyIndex = 0;
    unsigned int grainTexture = 0, blueTexture = 0;
    FullscreenQuad quad;
    bool buildFailed = false;
    double lastDt = 1.0 / 60.0;
    bool trailsCleared = false;

    juce::Random random;
    float reactAmount = 1.0f;
    float kickEnv = 0.0f, snareEnv = 0.0f, hatEnv = 0.0f;
    float flashTimer = 0.0f, flashType = 0.0f; // type 0 black, 1 mid colour, 2 light colour
    double lastFlashTime = -10.0;
    float symbolSeed = 1.0f, glitchSeed = 1.0f;

    // SHOTS state (see slot 14).
    float shotZoom = 1.0f;
    juce::Point<float> shotOffset;
    float invertTimer = 0.0f, blackTimer = 0.0f, shapeTimer = 0.0f;
    float shapeType = 0.0f, shapeFill = 0.0f;
    std::array<float, 4> shapeParams { 0.5f, 0.5f, 0.3f, 0.0f }; // centre x, y (uv), radius (height units), angle
    float hudSeed = 1.0f;
    double lastShotTime = -10.0;
    void fireShot (float shots, double timeSeconds);
    std::atomic<int> reframeCount { 0 };
public:
    // Bumps on every SHOTS reframe - the media bin cuts a playing clip to a
    // new section with it, so clips and framing jump together.
    int getReframeCount() const noexcept { return reframeCount.load(); }
private:

    // Film state, advanced at 24 fps (the grain / weave / flicker clock).
    double filmTime = 0.0;
    int filmFrame = 0;
    juce::Point<float> weaveOffset;
    float flicker = 0.0f, slipFrames = 0.0f;
    struct Scratch { float x = 0.0f, drift = 0.0f, life = 0.0f, age = 0.0f, width = 1.0f; };
    struct Hair { float x = 0.0f, y = 0.0f, angle = 0.0f, length = 0.0f, seed = 0.0f, life = 0.0f, age = 0.0f; };
    std::array<Scratch, 2> scratches;
    Hair hair;
};
