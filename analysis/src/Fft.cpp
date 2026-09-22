#include "vj/Fft.h"

#include <cassert>
#include <cmath>
#include <utility>

namespace vj
{

namespace
{
    constexpr double twoPi = 6.283185307179586476925286766559;

    bool isPowerOfTwo (int v) noexcept { return v > 0 && (v & (v - 1)) == 0; }
}

Fft::Fft (int size) : n (size)
{
    assert (isPowerOfTwo (size));

    int bits = 0;
    while ((1 << bits) < n)
        ++bits;

    bitReversed.resize ((size_t) n);
    for (int i = 0; i < n; ++i)
    {
        int r = 0;
        for (int b = 0; b < bits; ++b)
            if (i & (1 << b))
                r |= 1 << (bits - 1 - b);
        bitReversed[(size_t) i] = r;
    }

    twiddles.resize ((size_t) (n / 2));
    for (int k = 0; k < n / 2; ++k)
    {
        auto angle = -twoPi * (double) k / (double) n;
        twiddles[(size_t) k] = { (float) std::cos (angle), (float) std::sin (angle) };
    }
}

void Fft::transform (std::complex<float>* data) const noexcept
{
    for (int i = 0; i < n; ++i)
    {
        auto j = bitReversed[(size_t) i];
        if (j > i)
            std::swap (data[i], data[j]);
    }

    for (int len = 2; len <= n; len <<= 1)
    {
        auto half = len / 2;
        auto stride = n / len;

        for (int start = 0; start < n; start += len)
        {
            for (int k = 0; k < half; ++k)
            {
                auto w = twiddles[(size_t) (k * stride)];
                auto a = data[start + k];
                auto b = data[start + k + half] * w;
                data[start + k] = a + b;
                data[start + k + half] = a - b;
            }
        }
    }
}

std::vector<float> makePeriodicHann (int size)
{
    std::vector<float> w ((size_t) size);
    for (int i = 0; i < size; ++i)
        w[(size_t) i] = (float) (0.5 - 0.5 * std::cos (twoPi * (double) i / (double) size));
    return w;
}

} // namespace vj
