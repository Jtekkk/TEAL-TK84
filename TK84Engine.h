#pragma once
#include "TK84Detector.h"
#include "TK84GainComputer.h"
#include "TK84VCA.h"
#include "TK84Transient.h"
#include "TEAL_TK84_Parameters.h"
#include <array>

namespace TEAL::TK84 {

/**
 * TK84Engine
 * Top-level DSP engine for the TEAL TK84 Stereo VCA Compressor.
 * Wires together Detector, GainComputer, VCA, and Transient stages.
 */
class TK84Engine
{
public:
    TK84Engine();
    ~TK84Engine() = default;

    void prepare(double sampleRate, int samplesPerBlock);
    void reset();

    // Apply all parameters (call this every block or when parameters change)
    void setParameters(
        float thresholdDB,
        Ratio ratio,
        Attack attack,
        Release release,
        float transientLevelDB,
        float transientIntensity,
        float outputGainDB,
        SCHPF scHPF,
        StereoLink stereoLink,
        float inputTrimDB,
        VURef vuRef,
        GRMeterRange grRange,
        Oversampling oversampling,
        VCAMode vcaMode,
        bool bypass
    );

    void processBlock(const float* inL, const float* inR,
                      float* outL, float* outR,
                      int numSamples);

    // Metering
    float getCurrentGR() const { return currentGR; }
    float getCurrentVU() const { return currentVU; }

private:
    double sampleRate = 44100.0;

    // Core modules
    TK84Detector detector;
    TK84GainComputer gainComputer;
    TK84VCA vcaL, vcaR;
    TK84Transient transientL, transientR;

    // Current parameter state
    float thresholdDB = -10.0f;
    Ratio ratio = kRatioDefault;
    Attack attack = kAttackDefault;
    Release releaseMode = kReleaseDefault;
    float transientLevelDB = 0.0f;
    float transientIntensity = 0.5f;
    float outputGainDB = 0.0f;
    SCHPF scHPF = kSCHPFDefault;
    StereoLink stereoLink = kStereoLinkDefault;
    float inputTrimDB = 0.0f;
    VURef vuRef = kVURefDefault;
    GRMeterRange grRange = kGRMeterRangeDefault;
    Oversampling oversampling = kOversamplingDefault;
    VCAMode vcaMode = kVCAModeDefault;
    bool bypassActive = false;

    // Smoothing / state
    float currentGR = 0.0f;
    float currentVU = 0.0f;
    float peakGR = 0.0f;           // for peak hold
    float inputTrimGain = 1.0f;
    float outputGain = 1.0f;
    float vuRefOffset = -18.0f;    // dBFS that equals 0 VU

    // Simple bypass crossfade state (10 ms)
    float bypassFade = 1.0f; // 1.0 = processed, 0.0 = bypassed
    float bypassFadeCoeff = 0.0f;

    void updateGains();
    void updateDetectorAndGainComputer();
    void updateMetering(float grDB, float vuLevel);
    float applyBypass(float processed, float dry, float& fadeState);
};

} // namespace TEAL::TK84