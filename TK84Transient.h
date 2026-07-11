#pragma once
#include "TEAL_TK84_Parameters.h"

namespace TEAL::TK84 {

/**
 * TK84Transient
 * Post-compressor transient processor.
 *
 * Uses dual envelope-follower differential (fast vs slow) to isolate transients
 * in a level-independent way. Only the transient component is gained by
 * Transient Level. Sustain always passes at unity.
 *
 * Intensity controls detection window and sensitivity.
 */
class TK84Transient
{
public:
    TK84Transient();
    ~TK84Transient() = default;

    void prepare(double sampleRate);
    void reset();

    void setIntensity(float intensity01);   // 0.0 = surgical, 1.0 = broad
    void setLevel(float levelDB);           // -12 .. +12 dB

    /**
     * Process one sample.
     * @param input   Post-VCA audio sample
     * @return        Processed output with transient shaping applied
     */
    float processSample(float input);

    // For metering / debug
    float getTransientAmount() const { return lastTransientDiff; }

private:
    double sampleRate = 44100.0;

    float intensity = 0.5f;     // 0..1
    float levelDB = 0.0f;       // -12 .. +12
    float transientGain = 1.0f; // linear gain from levelDB

    // Envelope states
    float fastEnv = 0.0f;
    float slowEnv = 0.0f;

    // Coefficients (updated when Intensity changes)
    float fastAttackCoeff = 0.0f;
    float slowAttackCoeff = 0.0f;

    float lastTransientDiff = 0.0f;

    void updateCoefficients();
    float softSaturate(float x) const;
};

} // namespace TEAL::TK84