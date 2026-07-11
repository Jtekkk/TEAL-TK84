#include "TK84Engine.h"
#include <cmath>
#include <algorithm>

namespace TEAL::TK84 {

TK84Engine::TK84Engine()
{
    reset();
}

void TK84Engine::prepare(double newSampleRate, int /*samplesPerBlock*/)
{
    sampleRate = newSampleRate;

    detector.prepare(sampleRate, 0);
    gainComputer.prepare(sampleRate);
    vcaL.prepare(sampleRate);
    vcaR.prepare(sampleRate);
    transientL.prepare(sampleRate);
    transientR.prepare(sampleRate);

    // Bypass crossfade coefficient for ~10 ms
    float fadeTime = 0.010f;
    bypassFadeCoeff = 1.0f - std::exp(-1.0f / (fadeTime * sampleRate));

    reset();
}

void TK84Engine::reset()
{
    detector.reset();
    gainComputer.reset();
    vcaL.reset();
    vcaR.reset();
    transientL.reset();
    transientR.reset();

    currentGR = 0.0f;
    currentVU = 0.0f;
    bypassFade = 1.0f;
}

void TK84Engine::updateGains()
{
    inputTrimGain = std::pow(10.0f, inputTrimDB / 20.0f);
    outputGain    = std::pow(10.0f, outputGainDB / 20.0f);
}

void TK84Engine::updateDetectorAndGainComputer()
{
    detector.setAttack(attack);
    detector.setSCHPF(scHPF);
    detector.setStereoLink(stereoLink);

    gainComputer.setRatio(ratio);
    gainComputer.setAttack(attack);
    gainComputer.setRelease(releaseMode);
}

void TK84Engine::setParameters(
    float thresholdDB_,
    Ratio ratio_,
    Attack attack_,
    Release release_,
    float transientLevelDB_,
    float transientIntensity_,
    float outputGainDB_,
    SCHPF scHPF_,
    StereoLink stereoLink_,
    float inputTrimDB_,
    VURef /*vuRef_*/,
    GRMeterRange /*grRange_*/,
    Oversampling /*oversampling_*/,
    VCAMode vcaMode_,
    bool bypass_
)
{
    thresholdDB       = thresholdDB_;
    ratio             = ratio_;
    attack            = attack_;
    releaseMode       = release_;
    transientLevelDB  = transientLevelDB_;
    transientIntensity = transientIntensity_;
    outputGainDB      = outputGainDB_;
    scHPF             = scHPF_;
    stereoLink        = stereoLink_;
    inputTrimDB       = inputTrimDB_;
    vuRef             = vuRef_;
    grRange           = grRange_;
    oversampling      = oversampling_;
    vcaMode           = vcaMode_;
    bypassActive      = bypass_;

    // Update VU reference offset
    vuRefOffset = kVURefValues[static_cast<size_t>(vuRef)];

    updateGains();
    updateDetectorAndGainComputer();

    vcaL.setMode(vcaMode);
    vcaR.setMode(vcaMode);

    transientL.setLevel(transientLevelDB);
    transientR.setLevel(transientLevelDB);
    transientL.setIntensity(transientIntensity);
    transientR.setIntensity(transientIntensity);
}

float TK84Engine::applyBypass(float processed, float dry, float& fadeState)
{
    float target = bypassActive ? 0.0f : 1.0f;

    if (target > fadeState)
        fadeState = std::min(1.0f, fadeState + bypassFadeCoeff);
    else
        fadeState = std::max(0.0f, fadeState - bypassFadeCoeff);

    return processed * fadeState + dry * (1.0f - fadeState);
}

void TK84Engine::updateMetering(float grDB, float vuLevel)
{
    // GR with peak hold (1.5 s)
    currentGR = grDB;

    if (grDB < peakGR)
        peakGR = grDB;
    else
        peakGR = peakGR * 0.9995f + grDB * 0.0005f; // slow release on peak

    // Apply GR meter range scaling
    float scale = (grRange == GRMeterRange::Half) ? 0.5f : 1.0f;
    currentGR *= scale;

    // VU metering with proper ballistics (very simplified ANSI-style)
    float vuDB = 20.0f * std::log10(std::max(vuLevel, 1e-6f)) - vuRefOffset;
    currentVU = currentVU * 0.97f + vuDB * 0.03f; // ~300 ms integration feel
}

void TK84Engine::processBlock(const float* inL, const float* inR,
                              float* outL, float* outR,
                              int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        float dryL = inL[i];
        float dryR = inR[i];

        // Input trim
        float xL = dryL * inputTrimGain;
        float xR = dryR * inputTrimGain;

        // === Detection ===
        float detectionDB = detector.processSample(xL, xR);

        // Apply threshold (detection level is relative, subtract threshold)
        float overThreshold = detectionDB - thresholdDB;

        // === Gain Computer ===
        float grDB = gainComputer.processSample(overThreshold);
        currentGR = grDB;

        // === VCA stage (left + right) ===
        float vcaOutL = vcaL.processSample(xL, grDB);
        float vcaOutR = vcaR.processSample(xR, grDB);

        // === Transient stage (post VCA) ===
        float transOutL = transientL.processSample(vcaOutL);
        float transOutR = transientR.processSample(vcaOutR);

        // Output gain
        float processedL = transOutL * outputGain;
        float processedR = transOutR * outputGain;

        // === Bypass with crossfade ===
        outL[i] = applyBypass(processedL, dryL, bypassFade);
        outR[i] = applyBypass(processedR, dryR, bypassFade);

        // Update metering
        float peakLevel = std::max(std::abs(outL[i]), std::abs(outR[i]));
        updateMetering(grDB, peakLevel);
    }
}

} // namespace TEAL::TK84