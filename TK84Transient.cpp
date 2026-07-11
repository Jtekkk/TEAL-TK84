#include "TK84Transient.h"
#include <cmath>
#include <algorithm>

namespace TEAL::TK84 {

TK84Transient::TK84Transient()
{
    reset();
}

void TK84Transient::prepare(double newSampleRate)
{
    sampleRate = newSampleRate;
    updateCoefficients();
    reset();
}

void TK84Transient::reset()
{
    fastEnv = 0.0f;
    slowEnv = 0.0f;
    lastTransientDiff = 0.0f;
}

void TK84Transient::setIntensity(float intensity01)
{
    intensity = std::clamp(intensity01, 0.0f, 1.0f);
    updateCoefficients();
}

void TK84Transient::setLevel(float levelDB_)
{
    levelDB = std::clamp(levelDB_, -12.0f, 12.0f);
    transientGain = std::pow(10.0f, levelDB / 20.0f);
}

void TK84Transient::updateCoefficients()
{
    // Map Intensity (0..1) to time windows
    // Low intensity  = short/fast window (surgical on transients)
    // High intensity = longer window (captures more of the attack body)

    float fastMs = 0.1f + intensity * 1.9f;   // 0.1 → 2.0 ms
    float slowMs = 20.0f + intensity * 60.0f;  // 20  → 80 ms

    float fastTime = fastMs / 1000.0f;
    float slowTime = slowMs / 1000.0f;

    fastAttackCoeff = 1.0f - std::exp(-1.0f / (fastTime * sampleRate));
    slowAttackCoeff = 1.0f - std::exp(-1.0f / (slowTime * sampleRate));
}

float TK84Transient::softSaturate(float x) const
{
    // Gentle soft clipper around +6 dBFS to protect against transient boost overshoot
    constexpr float ceiling = 2.0f; // ~ +6 dBFS linear
    if (std::abs(x) <= ceiling)
        return x;

    float sign = x > 0.0f ? 1.0f : -1.0f;
    float excess = std::abs(x) - ceiling;
    return sign * (ceiling + excess / (1.0f + excess * 0.8f));
}

float TK84Transient::processSample(float input)
{
    float absInput = std::abs(input);

    // Fast envelope follower
    if (absInput > fastEnv)
        fastEnv = absInput;
    else
        fastEnv = fastEnv * (1.0f - fastAttackCoeff * 0.6f) + absInput * fastAttackCoeff * 0.6f;

    // Slow envelope follower
    if (absInput > slowEnv)
        slowEnv = absInput;
    else
        slowEnv = slowEnv * (1.0f - slowAttackCoeff * 0.3f) + absInput * slowAttackCoeff * 0.3f;

    // Transient differential (only positive differences count)
    float transientDiff = std::max(fastEnv - slowEnv, 0.0f);
    lastTransientDiff = transientDiff;

    // Apply gain only to the transient component
    float transientComponent = transientDiff * transientGain;

    // Recombine: sustain (input) + shaped transient
    float output = input + transientComponent;

    // Soft saturation when boosting transients to prevent overs
    if (levelDB > 0.5f)
        output = softSaturate(output);

    return output;
}

} // namespace TEAL::TK84