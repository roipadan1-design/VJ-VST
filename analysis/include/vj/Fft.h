#pragma once

#include <complex>
#include <vector>

namespace vj
{

// Iterative radix-2 complex FFT with precomputed twiddles and bit-reversal
// table. Sizes are fixed at construction so transform() never allocates -
// the analyzer runs it on a worker thread at a few hundred hops per second,
// where a per-call allocation would be the only non-deterministic cost.
class Fft
{
public:
    explicit Fft (int size);

    int size() const noexcept { return n; }

    // In-place forward transform of `data` (must hold size() elements).
    void transform (std::complex<float>* data) const noexcept;

private:
    int n = 0;
    std::vector<int> bitReversed;
    std::vector<std::complex<float>> twiddles; // exp(-2*pi*i*k/n), k < n/2
};

// Periodic Hann window (the analysis-frame convention: w[n] = 0.5 - 0.5*cos(2*pi*n/N)).
std::vector<float> makePeriodicHann (int size);

} // namespace vj
