#pragma once

#include <atomic>
#include <cstddef>
#include <vector>

namespace vj
{

// Single-producer / single-consumer lock-free ring of T. Storage is
// allocated in prepare() only; push/pop never allocate, lock or wait, so the
// producer side is safe to call from an audio callback. Exactly one thread may
// push and exactly one other thread may pop.
template <typename T>
class SpscFifo
{
public:
    void prepare (size_t capacity)
    {
        size_t size = 1;
        while (size < capacity + 1)
            size <<= 1;
        buffer.assign (size, T {});
        mask = size - 1;
        head.store (0);
        tail.store (0);
    }

    size_t capacity() const noexcept { return buffer.empty() ? 0 : buffer.size() - 1; }

    size_t available() const noexcept
    {
        return (head.load (std::memory_order_acquire) - tail.load (std::memory_order_acquire)) & mask;
    }

    size_t freeSpace() const noexcept { return capacity() - available(); }

    // Writes up to n items; returns how many fit (the rest are dropped - the
    // caller counts that as an analysis discontinuity, audio is unaffected).
    size_t push (const T* items, size_t n) noexcept
    {
        auto h = head.load (std::memory_order_relaxed);
        auto t = tail.load (std::memory_order_acquire);
        auto space = capacity() - ((h - t) & mask);
        auto count = n < space ? n : space;

        for (size_t i = 0; i < count; ++i)
            buffer[(h + i) & mask] = items[i];

        head.store ((h + count) & mask, std::memory_order_release);
        return count;
    }

    size_t pop (T* out, size_t n) noexcept
    {
        auto t = tail.load (std::memory_order_relaxed);
        auto h = head.load (std::memory_order_acquire);
        auto avail = (h - t) & mask;
        auto count = n < avail ? n : avail;

        for (size_t i = 0; i < count; ++i)
            out[i] = buffer[(t + i) & mask];

        tail.store ((t + count) & mask, std::memory_order_release);
        return count;
    }

    // Consumer-side: drop everything currently queued (stale-backlog recovery).
    void discardAll() noexcept { tail.store (head.load (std::memory_order_acquire), std::memory_order_release); }

private:
    std::vector<T> buffer;
    size_t mask = 0;
    std::atomic<size_t> head { 0 }, tail { 0 };
};

} // namespace vj
