#include "AnalysisWorker.h"

namespace
{
    double nowSeconds() { return juce::Time::getMillisecondCounterHiRes() * 0.001; }
}

struct AnalysisWorker::Listener : vj::Analyzer::Listener
{
    explicit Listener (AnalysisWorker& w) : worker (w) {}

    void featureFrame (const vj::FeatureFrame& f) override
    {
        vj::protocol::Transport t;
        t.valid = worker.tValid.load();
        t.playing = worker.tPlaying.load();
        t.bpm = worker.tBpm.load();
        t.ppqPosition = worker.tPpq.load();
        t.meterNumerator = worker.tNum.load();
        t.meterDenominator = worker.tDen.load();
        t.epoch = worker.tEpoch.load();

        {
            const juce::SpinLock::ScopedLockType sl (worker.metersLock);
            worker.meters.frame = f;
            worker.meters.transport = t;
            worker.meters.overflowed = worker.overflowFlag.load();
        }

        // Frames come in bursts (one host block = several hops). Keep only the
        // newest; run() sends it once the burst is analysed, so the engine
        // never gets the oldest frame of a block.
        latest = f;
        latestTransport = t;
        hasLatest = true;
    }

    // Called from run() after each burst: latest-wins snapshots, ~120 Hz.
    void sendLatest (double now)
    {
        if (! hasLatest || now - worker.lastFrameSent < 1.0 / 120.0)
            return;

        hasLatest = false;
        worker.lastFrameSent = now;
        worker.sendPacket (vj::protocol::encodeFrame (worker.writer, worker.sourceId, role, ++worker.frameSeq, latest, latestTransport));

        if (now - worker.lastSpectrumSent >= 1.0 / 60.0)
        {
            worker.lastSpectrumSent = now;
            worker.sendPacket (vj::protocol::encodeSpectrum (worker.writer, worker.sourceId, worker.frameSeq, latest));
        }
    }

    void onset (const vj::OnsetEvent& e) override
    {
        auto rate = worker.analyzer.getSampleRate();
        auto ageMs = (float) ((double) (worker.analyzer.getSamplesProcessed() - e.sampleIndex) / rate * 1000.0);
        worker.sendPacket (vj::protocol::encodeEvent (worker.writer, worker.sourceId, role, ++worker.eventSeq,
                                                      vj::protocol::onsetTypeName (e.region), e.strength, -1, 0,
                                                      juce::jmax (0.0f, ageMs)));

        const juce::SpinLock::ScopedLockType sl (worker.metersLock);
        worker.meters.lastOnset[(size_t) e.region] = nowSeconds();
    }

    AnalysisWorker& worker;
    int role = 0;
    vj::FeatureFrame latest;
    vj::protocol::Transport latestTransport;
    bool hasLatest = false;
};

AnalysisWorker::AnalysisWorker() : juce::Thread ("VJ Analyzer worker")
{
    sourceId = (int32_t) (juce::Random::getSystemRandom().nextInt() & 0x7fffffff);

    // Reply port for engine status: first free port in a small range.
    for (int port = 9100; port < 9300; ++port)
    {
        if (receiver.connect (port))
        {
            replyPort = port;
            receiver.addListener (this);
            break;
        }
    }
}

AnalysisWorker::~AnalysisWorker()
{
    release();
    receiver.removeListener (this);
    receiver.disconnect();
}

void AnalysisWorker::prepare (double sampleRate, int maxBlockSize)
{
    release();

    analyzer.prepare (sampleRate);
    auto capacity = (size_t) (sampleRate * 0.5) + (size_t) maxBlockSize * 4;
    fifoL.prepare (capacity);
    fifoR.prepare (capacity);
    notes.prepare (256);
    scratchL.assign (4096, 0.0f);
    scratchR.assign (4096, 0.0f);
    preparedRate = sampleRate;
    startThread (juce::Thread::Priority::normal);
}

void AnalysisWorker::release()
{
    stopThread (1000);
}

void AnalysisWorker::pushAudio (const float* left, const float* right, int numSamples) noexcept
{
    if (preparedRate.load() <= 0.0)
        return;

    auto pushedL = fifoL.push (left, (size_t) numSamples);
    auto pushedR = fifoR.push (right != nullptr ? right : left, (size_t) numSamples);
    if (pushedL < (size_t) numSamples || pushedR < (size_t) numSamples)
        overflowFlag = true; // worker stalled: analysis gap, audio untouched
}

void AnalysisWorker::pushNote (int note, int velocity) noexcept
{
    Note n { note, velocity };
    notes.push (&n, 1);
}

void AnalysisWorker::setTransport (const vj::protocol::Transport& t) noexcept
{
    tValid = t.valid;
    tPlaying = t.playing;
    tBpm = t.bpm;
    tPpq = t.ppqPosition;
    tNum = t.meterNumerator;
    tDen = t.meterDenominator;
    tEpoch = t.epoch;
}

void AnalysisWorker::setControls (const Controls& c)
{
    const juce::SpinLock::ScopedLockType sl (controlsLock);
    controls = c;
}

AnalysisWorker::Meters AnalysisWorker::getMeters() const
{
    const juce::SpinLock::ScopedLockType sl (metersLock);
    return meters;
}

AnalysisWorker::EngineStatus AnalysisWorker::getEngineStatus() const
{
    const juce::SpinLock::ScopedLockType sl (statusLock);
    auto s = status;
    s.connected = (nowSeconds() - lastStatusTime) < 1.5;
    return s;
}

void AnalysisWorker::setTarget (const juce::String& host, int port)
{
    const juce::SpinLock::ScopedLockType sl (targetLock);
    targetHost = host;
    targetPort = port;
}

void AnalysisWorker::sendPacket (std::string_view packet)
{
    if (packet.empty())
        return;

    juce::String host;
    int port;
    {
        const juce::SpinLock::ScopedLockType sl (targetLock);
        host = targetHost;
        port = targetPort;
    }
    socket.write (host, port, packet.data(), (int) packet.size());
}

void AnalysisWorker::sendControlsIfChanged()
{
    Controls c;
    {
        const juce::SpinLock::ScopedLockType sl (controlsLock);
        c = controls;
    }

    analyzer.setOnsetSensitivity (c.sensitivity);
    analyzer.setInputTrimDb (c.trimDb);
    analyzer.setNormalizerMode (c.lockedNormalizer ? vj::NormalizerMode::locked : vj::NormalizerMode::balanced);

    if (userTriggerPending.exchange (false))
    {
        writer.begin ("/v2/trigger");
        sendPacket (writer.finish());
    }

    if (fullscreenPending.exchange (false))
    {
        writer.begin ("/fullscreen"); // no argument = toggle
        sendPacket (writer.finish());
    }

    for (auto step = sceneStep.exchange (0); step != 0; step += step > 0 ? -1 : 1)
    {
        writer.begin (step > 0 ? "/v2/preset/next" : "/v2/preset/previous");
        sendPacket (writer.finish());
    }

    if (auto index = presetOverride.exchange (-1); index >= 0)
    {
        writer.begin ("/v2/preset");
        writer.addInt (index);
        sendPacket (writer.finish());
    }

    if (! c.sendControls)
        return;

    for (int i = 0; i < 8; ++i)
        if (controlsNeverSent || std::abs (c.macros[(size_t) i] - sentControls.macros[(size_t) i]) > 1.0e-4f)
            sendPacket (vj::protocol::encodeMacro (writer, i, c.macros[(size_t) i]));

    for (int i = 0; i < (int) c.look.size(); ++i)
        if (controlsNeverSent || std::abs (c.look[(size_t) i] - sentControls.look[(size_t) i]) > 1.0e-4f)
        {
            writer.begin ("/v2/look");
            writer.addInt (i);
            writer.addFloat (c.look[(size_t) i]);
            sendPacket (writer.finish());
        }

    if (controlsNeverSent || c.palette != sentControls.palette || c.paletteMix != sentControls.paletteMix)
    {
        writer.begin ("/v2/palette");
        for (auto v : c.palette)
            writer.addFloat (v);
        writer.addFloat (c.paletteMix);
        sendPacket (writer.finish());
    }

    if (controlsNeverSent || c.react != sentControls.react)
    {
        writer.begin ("/v2/react");
        for (auto open : c.react)
            writer.addInt (open ? 1 : 0);
        sendPacket (writer.finish());
    }

    for (int i = 0; i < (int) c.move.size(); ++i)
        if (controlsNeverSent || std::abs (c.move[(size_t) i] - sentControls.move[(size_t) i]) > 1.0e-4f)
        {
            writer.begin ("/v2/move");
            writer.addInt (i);
            writer.addFloat (c.move[(size_t) i]);
            sendPacket (writer.finish());
        }

    // Preset only on change (never re-asserted): the engine keyboard, other
    // devices or scene links may have moved on and must not be overridden.
    // preset 0 means "leave the engine's choice alone"; 1..64 select index-1.
    // A value restored with the Live set is sent once on load (state recall).
    if (c.preset > 0 && (presetNeverSent || c.preset != sentControls.preset))
    {
        writer.begin ("/v2/preset");
        writer.addInt (c.preset - 1);
        sendPacket (writer.finish());
    }

    if (controlsNeverSent || c.blackout != sentControls.blackout)
    {
        writer.begin ("/v2/blackout");
        writer.addInt (c.blackout ? 1 : 0);
        sendPacket (writer.finish());
    }

    sentControls = c;
    controlsNeverSent = false;
    presetNeverSent = false;
}

void AnalysisWorker::run()
{
    Listener listener (*this);

    while (! threadShouldExit())
    {
        auto now = nowSeconds();
        auto rate = preparedRate.load();

        {
            const juce::SpinLock::ScopedLockType sl (controlsLock);
            listener.role = controls.role;
        }

        // Never replay a stale backlog: skip it and carry on from "now".
        if ((double) fifoL.available() > rate * 0.1)
        {
            fifoL.discardAll();
            fifoR.discardAll();
            overflowFlag = true;
        }

        for (;;)
        {
            auto n = juce::jmin (fifoL.available(), fifoR.available(), scratchL.size());
            if (n == 0)
                break;
            fifoL.pop (scratchL.data(), n);
            fifoR.pop (scratchR.data(), n);
            analyzer.process (scratchL.data(), scratchR.data(), (int) n, listener);
        }
        listener.sendLatest (nowSeconds());

        Note note;
        while (notes.pop (&note, 1) == 1)
        {
            sendPacket (vj::protocol::encodeEvent (writer, sourceId, listener.role, ++eventSeq, "note",
                                                   (float) note.velocity / 127.0f, note.note, note.velocity, 0.0f));
            const juce::SpinLock::ScopedLockType sl (metersLock);
            meters.lastMidi = now;
        }

        sendControlsIfChanged();

        if (now - lastHelloSent >= 1.0)
        {
            lastHelloSent = now;
            sendPacket (vj::protocol::encodeHello (writer, sourceId, listener.role, "VJ Analyzer", replyPort));
            controlsNeverSent = true; // macros/blackout re-sent once a second: survives lost datagrams
        }

        wait (2);
    }
}

void AnalysisWorker::oscMessageReceived (const juce::OSCMessage& m)
{
    auto address = m.getAddressPattern().toString();
    const juce::SpinLock::ScopedLockType sl (statusLock);

    if (address == "/v2/status" && m.size() >= 8)
    {
        auto asInt = [&m] (int i) { return m[i].isInt32() ? m[i].getInt32() : (int) m[i].getFloat32(); };
        auto asFloat = [&m] (int i) { return m[i].isFloat32() ? m[i].getFloat32() : (float) m[i].getInt32(); };
        status.presetIndex = asInt (0);
        status.presetName = m[1].isString() ? m[1].getString() : juce::String();
        status.numPresets = asInt (2);
        status.blackout = asInt (3) != 0;
        status.fps = asFloat (4);
        status.bpm = asFloat (5);
        status.followingHost = asInt (6) != 0;
        status.demo = asInt (7) != 0;
        status.renderScale = m.size() > 8 ? asFloat (8) : 1.0f;
        status.speed = m.size() > 9 ? asFloat (9) : 1.0f;
        lastStatusTime = nowSeconds();
    }
    else if (address == "/v2/macros")
    {
        status.macroLabels.clearQuick();
        for (int i = 0; i < m.size() && i < 8; ++i)
            status.macroLabels.add (m[i].isString() ? m[i].getString() : juce::String());
        status.sceneDescription = m.size() > 8 && m[8].isString() ? m[8].getString() : juce::String();
    }
    else if (address == "/v2/presets")
    {
        status.presetNames.clearQuick();
        for (auto& arg : m)
            if (arg.isString())
                status.presetNames.add (arg.getString());
    }
}
