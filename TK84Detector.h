#pragma once
#include "TEAL_TK84_Parameters.h"
#include <array>
#include <vector>

namespace TEAL::TK84 {

/**
 * TK84Detector
 * Hybrid RMS + Peak detector with linked stereo sidechain,
 * automatic fast-attack 4× internal oversampling, and SC HPF.
 *
 * Output is detection level in dB (negative values = below threshold reference).
 * The GainComputer consumes this value.
 */
class TK84Detector
{
public:
    TK84Detector();
    ~TK84Detector() = default;

    void prepare(double newSampleRate, int samplesPerBlock);
    void reset();

    // Parameter updates (call from audio thread or with smoothing if needed)
    void setAttack(Attack attackMode);
    void setSCHPF(SCHPF hpfMode);
    void setStereoLink(StereoLink linkMode);

    // Main processing — returns the linked (or per-channel) detection level in dB
    // Call once per sample (or per block with internal loop)
    float processSample(float inputL, float inputR);

    // Block processing helper (recommended for performance)
    void processBlock(const float* inL, const float* inR, float* detectionOut, int numSamples);

    // Current detection level (for metering / debug)
    float getCurrentDetectionLevel() const { return currentDetectionDB; }

private:
    // Internal processing rate (normal or 4× for fast attack)
    double sampleRate = 44100.0;
    double internalSampleRate = 44100.0;
    bool use4xOversampling = false;

    // HPF state (simple 1-pole for now — can upgrade to 12 dB/oct)
    float hpfCoeff = 0.0f;
    float hpfStateL = 0.0f;
    float hpfStateR = 0.0f;

    // RMS follower (5 ms window equivalent)
    float rmsCoeff = 0.0f;
    float rmsStateL = 0.0f;
    float rmsStateR = 0.0f;

    // Peak follower (fast envelope)
    float peakCoeff = 0.0f;
    float peakStateL = 0.0f;
    float peakStateR = 0.0f;

    // Stereo link amount (0.0 = dual mono, 1.0 = 100% max link)
    float stereoLinkAmount = 1.0f;

    // Current attack setting (affects weighting and oversampling)
    Attack currentAttack = kAttackDefault;

    // Final detection output (in dB)
    float currentDetectionDB = -100.0f;

    // Reference level: 0 dB detection = -18 dBFS sine RMS
    static constexpr float kDetectorReferenceDB = -18.0f;

    void updateCoefficients();
    float applyHPF(float input, float& state);
    float processOneChannel(float input, float& rmsState, float& peakState);
};

} // namespace TEAL::TK84