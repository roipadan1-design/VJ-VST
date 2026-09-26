#pragma once

#include <JuceHeader.h>

// Exactly one VJ Analyzer instance sends the knobs, look, scene and blackout
// to the engine: the "lead". Live runs every instance in one process, so a
// process-wide registry elects it - no more instances overwriting each other
// once a second ("Send macros" fights). Rule, re-evaluated on every call:
//   1. an instance pinned with MAKE LEAD (the most recent pin wins)
//   2. otherwise, among instances allowed to lead (parameter "Send Controls"):
//      role MIX first, then a track called Master / Main, then the oldest
// Instances that don't lead show the small SOURCE view.
class LeadRegistry
{
public:
    struct Info
    {
        bool canLead = true;
        int role = 0;              // 0 = MIX
        juce::String trackName;
        double pinTime = 0.0;      // > 0: pinned with MAKE LEAD
    };

    int add (const void* owner)
    {
        const juce::SpinLock::ScopedLockType l (lock);
        entries.push_back ({ owner, ++serialCounter, {} });
        return serialCounter;
    }

    void remove (const void* owner)
    {
        const juce::SpinLock::ScopedLockType l (lock);
        entries.erase (std::remove_if (entries.begin(), entries.end(), [owner] (auto& e) { return e.owner == owner; }), entries.end());
    }

    void update (const void* owner, const Info& info)
    {
        const juce::SpinLock::ScopedLockType l (lock);
        for (auto& e : entries)
            if (e.owner == owner)
                e.info = info;
    }

    const void* lead() const
    {
        const juce::SpinLock::ScopedLockType l (lock);
        const Entry* best = nullptr;
        auto score = [] (const Entry& e) {
            const auto name = e.info.trackName.toLowerCase();
            const bool master = name.contains ("master") || name.contains ("main");
            return (e.info.role == 0 ? 2 : 0) + (master ? 1 : 0);
        };
        for (auto& e : entries)
        {
            if (! e.info.canLead)
                continue;
            if (best == nullptr)
            {
                best = &e;
                continue;
            }
            if (e.info.pinTime != best->info.pinTime)
            {
                if (e.info.pinTime > best->info.pinTime)
                    best = &e;
                continue;
            }
            if (score (e) > score (*best) || (score (e) == score (*best) && e.serial < best->serial))
                best = &e;
        }
        return best != nullptr ? best->owner : nullptr;
    }

    bool isLead (const void* owner) const { return lead() == owner; }

    // Name of the lead's track (for the SOURCE view's "played in the lead on ..."), or "".
    juce::String leadTrackName() const
    {
        auto* l = lead();
        const juce::SpinLock::ScopedLockType sl (lock);
        for (auto& e : entries)
            if (e.owner == l)
                return e.info.trackName;
        return {};
    }

    int count() const
    {
        const juce::SpinLock::ScopedLockType l (lock);
        return (int) entries.size();
    }

private:
    struct Entry
    {
        const void* owner;
        int serial;
        Info info;
    };
    mutable juce::SpinLock lock;
    std::vector<Entry> entries;
    int serialCounter = 0;
};
