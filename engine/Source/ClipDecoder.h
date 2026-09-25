#pragma once

#include <JuceHeader.h>
#include <atomic>
#include <memory>
#include <vector>

// A short video clip decoded once, completely, into memory - so playback is
// just "upload frame N": any speed, freeze, reverse, jumps, no decoder in the
// show loop (the research: grainy H.264 decodes at ~9 fps in software here).
//
// Frames are stored as 8-bit BGR, bottom row first (ready for glTexImage2D
// like every other source), downscaled so the whole clip fits a memory
// budget, at no more than 30 fps. The clip is playable while it decodes:
// `decoded` counts the frames that are ready.
struct Clip
{
    int width = 0, height = 0;
    double fps = 30.0;
    int capacity = 0;                                  // frames allocated (the budget)
    std::vector<std::unique_ptr<uint8_t[]>> frames;    // `capacity` slots, filled in order
    std::atomic<int> decoded { 0 };
    std::atomic<bool> finished { false }, failed { false }, cancelled { false };

    size_t frameBytes() const noexcept { return (size_t) width * (size_t) height * 3; }
};

namespace ClipDecoder
{
    // Blocking (run it on a background thread). Opens the file with Media
    // Foundation, sizes the clip to `budgetBytes` (long edge <= maxEdge) and
    // decodes into `clip`. Returns false if the file can't be read.
    bool decode (const juce::File& file, Clip& clip, size_t budgetBytes, int maxEdge);

    bool isSupportedVideo (const juce::File& file);
}
