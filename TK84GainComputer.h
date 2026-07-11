#pragma once
#include "TEAL_TK84_Parameters.h"
#include <array>

namespace TEAL::TK84 {

/**
 * TK84GainComputer
 * Calculates gain reduction in dB from detection level.
 * Implements soft knee, all ratios (including LIMIT), log-domain ballistics,
 * and program-dependent AUTO release.
 */
class TK84GainComputer
{
public:
    TK84GainComputer();
    ~TK84GainComputer() = default;

    void prepare(double sampleRate);
    void reset();

    void setRatio(Ratio ratio);
    void setAttack(Attack attack);
    void setRelease(Release releaseMode);

    // Main processing: takes detection level in dB, returns gain reduction in dB (negative)
    float processSample(float detectionLevelDB);

    float getCurrentGR() const { return currentGR; }

private:
    double sampleRate = 44100.0;

    Ratio currentRatio = kRatioDefault;
    Attack currentAttack = kAttackDefault;
    Release currentRelease = kReleaseDefault;

    float currentGR = 0.0f;           // current gain reduction in dB
    float targetGR = 0.0f;

    // Attack/Release coefficients (per sample)
    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;

    // AUTO release state
    float fastReleaseState = 0.0f;
    float slowReleaseState = 0.0f;

    void updateTimingCoefficients();
    float calculateStaticGR(float detectionLevelDB) const;
    float applyKnee(float overThreshold) const;
};

} // namespace TEAL::TK84