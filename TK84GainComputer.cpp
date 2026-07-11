#include "TK84GainComputer.h"
#include <cmath>
#include <algorithm>

namespace TEAL::TK84 {

TK84GainComputer::TK84GainComputer()
{
    reset();
}

void TK84GainComputer::prepare(double newSampleRate)
{
    sampleRate = newSampleRate;
    updateTimingCoefficients();
    reset();
}

void TK84GainComputer::reset()
{
    currentGR = 0.0f;
    targetGR = 0.0f;
    fastReleaseState = 0.0f;
    slowReleaseState = 0.0f;
}

void TK84GainComputer::setRatio(Ratio ratio)
{
    currentRatio = ratio;
}

void TK84GainComputer::setAttack(Attack attack)
{
    currentAttack = attack;
    updateTimingCoefficients();
}

void TK84GainComputer::setRelease(Release releaseMode)
{
    currentRelease = releaseMode;
    updateTimingCoefficients();
}

void TK84GainComputer::updateTimingCoefficients()
{
    // Attack time from parameter
    float attackMs = kAttackTimesMs[static_cast<size_t>(currentAttack)];
    float attackTime = attackMs / 1000.0f;

    attackCoeff = 1.0f - std::exp(-1.0f / (attackTime * sampleRate));

    // Release time (non-AUTO)
    if (currentRelease != Release::Auto)
    {
        float releaseMs = kReleaseTimesMs[static_cast<size_t>(currentRelease)];
        float releaseTime = releaseMs / 1000.0f;
        releaseCoeff = 1.0f - std::exp(-1.0f / (releaseTime * sampleRate));
    }
    // AUTO coefficients are calculated inside processSample for clarity
}

float TK84GainComputer::applyKnee(float overThreshold) const
{
    float ratio = ratioValue(currentRatio);
    float kneeWidth = 6.0f; // dB, narrows at higher ratios

    if (currentRatio == Ratio::Limit)
        kneeWidth = 1.5f;

    if (overThreshold <= 0.0f)
        return 0.0f;

    if (overThreshold < kneeWidth)
    {
        // Soft knee region
        float t = overThreshold / kneeWidth;
        return overThreshold * (1.0f - (1.0f - t) * (1.0f - t)); // simple quadratic soft knee
    }

    // Above knee
    return overThreshold;
}

float TK84GainComputer::calculateStaticGR(float detectionLevelDB) const
{
    float threshold = 0.0f; // detection level is already relative
    float over = detectionLevelDB - threshold; // how much above "0" (will be adjusted by actual threshold later)

    // Note: Actual threshold comparison happens outside or we receive already-thresholded level.
    // For now we assume detectionLevelDB is the amount over threshold.

    float ratio = ratioValue(currentRatio);

    if (over <= 0.0f)
        return 0.0f;

    float gr = applyKnee(over);

    if (currentRatio == Ratio::Limit)
    {
        gr = over; // hard limiting above knee
    }
    else
    {
        gr = gr * (1.0f - 1.0f / ratio);
    }

    return -gr; // negative = gain reduction
}

float TK84GainComputer::processSample(float detectionLevelDB)
{
    // For this first version we treat detectionLevelDB as "dB over threshold"
    // (the actual threshold subtraction will be done in the Engine when we have the full parameter set)

    float staticGR = calculateStaticGR(detectionLevelDB);

    targetGR = staticGR;

    // Attack / Release smoothing in log (dB) domain
    if (targetGR < currentGR)
    {
        // More reduction needed → attack
        currentGR = currentGR * (1.0f - attackCoeff) + targetGR * attackCoeff;
    }
    else
    {
        // Less reduction → release
        if (currentRelease == Release::Auto)
        {
            // === Calibrated Program-Dependent AUTO Release ===
            // Fast integrator: ~120 ms (quick recovery after transients)
            // Slow integrator: ~1.2 s  (maintains density on sustained material)
            constexpr float fastReleaseMs  = 120.0f;
            constexpr float slowReleaseMs  = 1200.0f;

            float fastCoeff = 1.0f - std::exp(-1.0f / (fastReleaseMs / 1000.0f * sampleRate));
            float slowCoeff = 1.0f - std::exp(-1.0f / (slowReleaseMs / 1000.0f * sampleRate));

            // Run both integrators
            fastReleaseState = fastReleaseState * (1.0f - fastCoeff) + targetGR * fastCoeff;
            slowReleaseState = slowReleaseState * (1.0f - slowCoeff) + targetGR * slowCoeff;

            // Program-dependent blend:
            // - High crest / recent big GR → favor fast release (quick recovery)
            // - Steady GR → favor slow release (glue / density)
            float crestFactor = std::abs(fastReleaseState - slowReleaseState);
            float blend = std::clamp(crestFactor * 0.8f + (currentGR / -25.0f) * 0.4f, 0.0f, 1.0f);

            float autoReleasedGR = fastReleaseState * (1.0f - blend) + slowReleaseState * blend;

            // Smooth the final GR toward the auto behavior
            currentGR = currentGR * 0.88f + autoReleasedGR * 0.12f;
        }
        else
        {
            currentGR = currentGR * (1.0f - releaseCoeff) + targetGR * releaseCoeff;
        }
    }

    // Clamp to max 40 dB GR
    currentGR = std::max(currentGR, -40.0f);

    return currentGR;
}

} // namespace TEAL::TK84