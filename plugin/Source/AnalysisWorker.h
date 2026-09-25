#pragma once

#include <JuceHeader.h>
#include "vj/Analyzer.h"
#include "vj/Protocol.h"
#include "vj/SpscFifo.h"

// Everything between the audio callback and the network (ENGINEERING_SPEC 3):
//
//   audio thread  --SpscFifo (samples, MIDI notes)-->  worker thread
//   worker: Analyzer::process -> OSC v2 frames (~120 Hz), spectrum (~60 Hz),
//           onset/MIDI events immediately, controls when they change, hello 1 Hz
//   engine status (/v2/status, /v2/presets) arrives on an OSCReceiver bound to
//   a free local port, advertised in /v2/hello.
//
// The audio thread only copies samples and writes a few atomics - no locks,
// no allocation, no sockets. If the worker falls >100 ms behind, stale audio
// is dropped (analysis discontinuity) rather than ever delaying audio.
class AnalysisWorker : private juce::Thread,
                       private juce::OSCReceiver::Listener<juce::OSCReceiver::RealtimeCallback>
{
public:
    struct Controls
    {
        int role = 0;
        float sensitivity = 1.0f, trimDb = 0.0f;
        bool lockedNormalizer = false;
        bool sendControls = true;
        std::array<float, 8> macros {};
        int preset = 0;            // 0 = engine's choice, n = engine preset n-1
        bool blackout = false;
        std::array<float, 14> look {};    // /v2/look slots 0-13
        std::array<float, 9> palette {};  // 3 x rgb, 0-1
        float paletteMix = 1.0f;
        std::array<bool, 5> react { true, true, true, true, true }; // kick snare hat bass level
        std::array<float, 6> move { 0.2f, 0.3f, 0.5f, 0.0f, 0.0f, 0.0f }; // /v2/move: drift push softness sync reverse freeze
        juce::String mediaPath;      // image in media slot 1 ("" = none)
        bool useMedia = false;       // USE MEDIA: the image replaces the forms in every scene
    };

    struct Meters // UI snapshot, copied under a spin lock at ~30 Hz
    {
        vj::FeatureFrame frame;
        std::array<double, 3> lastOnset { -10.0, -10.0, -10.0 };
        double lastMidi = -10.0;
        vj::protocol::Transport transport;
        bool overflowed = false;
    };

    struct EngineStatus
    {
        bool connected = false;
        int presetIndex = -1, numPresets = 0;
        juce::String presetName;
        juce::StringArray presetNames;
        bool blackout = false, followingHost = false, demo = false;
        float fps = 0.0f, bpm = 0.0f, renderScale = 1.0f;
        float speed = 1.0f;                 // live scene-clock speed (Speed x Push, signed)
        juce::StringArray macroLabels;      // the live scene's names for the 8 macros ("" = generic)
        juce::String sceneDescription;
        juce::String mediaName;             // media slot 1 as the engine has it
        int mediaWidth = 0, mediaHeight = 0;
        bool mediaLoading = false;
    };

    AnalysisWorker();
    ~AnalysisWorker() override;

    void prepare (double sampleRate, int maxBlockSize);
    void release();

    // --- audio thread ---
    void pushAudio (const float* left, const float* right, int numSamples) noexcept;
    void pushNote (int note, int velocity) noexcept;
    void setTransport (const vj::protocol::Transport& t) noexcept;

    // --- message thread ---
    void setControls (const Controls& c);
    void sendUserTrigger() { userTriggerPending = true; }
    void selectPresetNow (int index) { presetOverride = index; }
    void stepScene (int direction) { sceneStep += direction; }
    void toggleEngineFullscreen() { fullscreenPending = true; }
    Meters getMeters() const;
    EngineStatus getEngineStatus() const;
    void setTarget (const juce::String& host, int port);
    int getReplyPort() const noexcept { return replyPort; }

private:
    void run() override;
    void oscMessageReceived (const juce::OSCMessage&) override;
    void sendPacket (std::string_view packet);
    void sendControlsIfChanged();

    struct Listener; // Analyzer::Listener adapter, defined in the .cpp
    friend struct Listener;

    struct Note { int note, velocity; };

    vj::Analyzer analyzer;
    vj::SpscFifo<float> fifoL, fifoR;
    vj::SpscFifo<Note> notes;
    std::vector<float> scratchL, scratchR;
    std::atomic<bool> overflowFlag { false };
    std::atomic<double> preparedRate { 0.0 };

    // Transport, written by the audio thread (seqlock-free: individual atomics
    // are fine - the fields change slowly and are reassembled each frame).
    std::atomic<double> tPpq { 0.0 }, tBpm { 120.0 };
    std::atomic<int> tNum { 4 }, tDen { 4 }, tEpoch { 0 };
    std::atomic<bool> tValid { false }, tPlaying { false };

    juce::SpinLock controlsLock;
    Controls controls, sentControls;
    bool controlsNeverSent = true, presetNeverSent = true;
    std::atomic<bool> userTriggerPending { false };
    std::atomic<int> presetOverride { -1 };
    std::atomic<int> sceneStep { 0 };
    std::atomic<bool> fullscreenPending { false };

    mutable juce::SpinLock metersLock;
    Meters meters;

    mutable juce::SpinLock statusLock;
    EngineStatus status;
    double lastStatusTime = -10.0;

    juce::DatagramSocket socket { false };
    juce::SpinLock targetLock;
    juce::String targetHost = "127.0.0.1";
    int targetPort = vj::protocol::defaultPort;

    juce::OSCReceiver receiver;
    int replyPort = 0;

    vj::protocol::OscWriter writer;
    int32_t sourceId = 0, frameSeq = 0, eventSeq = 0;
    double lastFrameSent = 0.0, lastSpectrumSent = 0.0, lastHelloSent = -10.0;
};
