#pragma once
#include "TEAL_TK84_Parameters.h"

namespace TEAL::TK84 {

/**
 * TK84VCA
 * Precision VCA gain cell model.
 * - Exponential (dB-linear) control law (6.1 mV/dB modeled)
 * - Optional "Vintage" mode with subtle level-dependent odd harmonic generation
 *   under gain reduction (matching THAT 2181-class behavior, ≤0.05% THD @ 10 dB GR)
 */
class TK84VCA
{
public:
    TK84VCA();
    ~TK84VCA() = default;

    void prepare(double sampleRate);
    void reset();

    void setMode(VCAMode mode);

    /**
     * Process one audio sample with control voltage.
     * @param input   Audio sample (-1.0 ... +1.0 range expected)
     * @param cvDB    Control voltage in dB (negative = gain reduction)
     * @return        Processed output sample
     */
    float processSample(float input, float cvDB);

    // Current gain (linear) for metering/debug
    float getCurrentGain() const { return currentGain; }

private:
    VCAMode currentMode = kVCAModeDefault;
    float currentGain = 1.0f;

    // Slight DC offset removal for vintage mode stability
    float dcBlockState = 0.0f;

    float applyVintageNonlinearity(float input, float cvDB) const;
};

} // namespace TEAL::TK84