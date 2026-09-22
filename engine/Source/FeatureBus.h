#pragma once

#include <JuceHeader.h>
#include "Signals.h"
#include <map>

// Collects every analysis source (VJ Analyzer plugin instances, analyze_wav,
// the legacy M4L analyzer's /audio/* messages, the built-in demo) and turns
// them into one Signals snapshot per rendered frame.
//
//  - Protocol v2 (/v2/hello, /v2/frame, /v2/spectrum, /v2/event) and the
//    legacy /audio/level|bass|mid|high|beatphase|onset messages are both
//    accepted, so old devices keep driving new presets and vice versa.
//  - Role resolution: a "kick" source's bass transients (or its MIDI notes -
//    MIDI wins for 2 s after a note) become event.kick; with no live kick
//    source, the mix source's bass transients stand in. Same for snare/hat.
//  - Continuous values are latest-wins; a source silent for 250 ms fades out
//    over 300 ms and is dropped after 2 s.
//  - Events are de-duplicated by (source, id) and expire after 150 ms - a late
//    flash is worse than a missed one.
//
// handleMessage() runs on the OSC/message thread, takeSnapshot() on the GL
// thread; a single short critical section guards the shared state.
class FeatureBus
{
public:
    // Returns true if the message was a feature/event message it consumed.
    bool handleMessage (const juce::OSCMessage& message, double nowSeconds);

    // Injects a manual hit (keyboard / /v2/trigger).
    void pushUserTrigger (double nowSeconds);

    Signals takeSnapshot (double nowSeconds);

    void setDemoEnabled (bool shouldBeEnabled) noexcept { demoEnabled = shouldBeEnabled; }
    bool isDemoEnabled() const noexcept { return demoEnabled; }

    // Local UDP ports that asked for engine status (5th /v2/hello argument),
    // seen within the last 3 s.
    juce::Array<int> getReplyPorts (double nowSeconds) const;

    // For the status log: "2 sources: mix (analyze_wav), kick (VJ Analyzer)".
    juce::String describeSources (double nowSeconds) const;

    static constexpr double eventExpirySeconds = 0.15;
    static constexpr double staleAfterSeconds = 0.25, fadeSeconds = 0.3, dropAfterSeconds = 2.0;

private:
    struct Source
    {
        SourceRole role = SourceRole::mix;
        juce::String name;
        double lastFrameTime = -1000.0, lastMidiNoteTime = -1000.0;
        int lastSeq = -1, lastEventId = -1;

        float levelRel = 0, levelAbs = 0, bassRel = 0, midRel = 0, highRel = 0, bassAbs = 0, midAbs = 0, highAbs = 0;
        std::array<float, 6> bandRel {};
        std::array<float, 32> spectrum {};
        float centroid = 0, flatness = 0, rolloff = 0, flux = 0, energyTrend = 0;

        bool transportValid = false, playing = false;
        double bpm = 120.0, beatPosition = 0.0, transportReceivedAt = -1000.0;
        int meterNumerator = 4, meterDenominator = 4, epoch = 0;
    };

    struct PendingEvent
    {
        EventType rawType;   // bassTransient / midTransient / highTransient / midiNote / userTrigger
        SourceRole role;
        bool midiRecent;     // source played MIDI notes in the last 2 s
        float strength;
        int note, velocity;
        double time;
    };

    void handleFrame (const juce::OSCMessage&, double now);
    void handleSpectrum (const juce::OSCMessage&);
    void handleEvent (const juce::OSCMessage&, double now);
    void handleLegacy (const juce::String& address, const juce::OSCMessage&, double now);
    void synthesizeDemo (Signals&, double now);

    static bool isNewer (int incoming, int last) noexcept;
    static SourceRole parseRole (const juce::String&) noexcept;

    mutable juce::CriticalSection lock;
    std::map<int, Source> sources;
    std::map<int, double> replyPorts; // port -> last hello time
    juce::Array<PendingEvent> pending;
    double lastImpactTime = -1000.0;

    // Legacy /audio/beatphase has no beat counter; count wraps ourselves.
    double legacyBeatCount = 0.0, legacyLastPhase = 0.0, legacyLastWrapTime = -1.0, legacyBpm = 120.0;

    std::atomic<bool> demoEnabled { false };
    double demoLastBeat = -1.0;

    static constexpr int legacySourceId = std::numeric_limits<int>::min() + 1;
    static constexpr int demoSourceId = std::numeric_limits<int>::min() + 2;
};
