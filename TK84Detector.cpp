#include "TK84Detector.h"
#include <cmath>
#include <algorithm>

namespace TEAL::TK84 {

TK84Detector::TK84Detector()
{
    reset();
}

void TK84Detector::prepare(double newSampleRate, int /*samplesPerBlock*/)
{
    sampleRate = newSampleRate;
    internalSampleRate = newSampleRate;
    use4xOversampling = false;

    updateCoefficients();
    reset();
}

void TK84Detector::reset()
{
    hpfStateL = hpfStateR = 0.0f;
    rmsStateL = rmsStateR = 0.0f;
    peakStateL = peakStateR = 0.0f;
    currentDetectionDB = -100.0f;
}

void TK84Detector::setAttack(Attack attackMode)
{
    currentAttack = attackMode;
    use4xOversampling = isFastAttack(attackMode);

    if (use4xOversampling)
        internalSampleRate = sampleRate * 4.0;
    else
        internalSampleRate = sampleRate;

    updateCoefficients();
}

void TK84Detector::setSCHPF(SCHPF hpfMode)
{
    float cutoff = kSCHPFCutoffs[static_cast<size_t>(hpfMode)];

    if (cutoff <= 1.0f)
    {
        hpfCoeff = 0.0f; // bypass
    }
    else
    {
        // Simple 1-pole high-pass (can be upgraded to proper 12 dB/oct later)
        float omega = 2.0f * kPi * cutoff / static_cast<float>(internalSampleRate);
        hpfCoeff = omega / (1.0f + omega);
    }
}

void TK84Detector::setStereoLink(StereoLink linkMode)
{
    stereoLinkAmount = kStereoLinkAmounts[static_cast<size_t>(linkMode)];
}

void TK84Detector::updateCoefficients()
{
    // RMS time constant ~5 ms
    float rmsTime = 0.005f;
    rmsCoeff = 1.0f - std::exp(-1.0f / (rmsTime * internalSampleRate));

    // Peak follower — fast attack, medium release
    float peakAttackTime = 0.0005f; // 0.5 ms
    peakCoeff = 1.0f - std::exp(-1.0f / (peakAttackTime * internalSampleRate));
}

float TK84Detector::applyHPF(float input, float& state)
{
    if (hpfCoeff <= 0.0f)
        return input;

    float highpassed = input - state;
    state = input - highpassed * hpfCoeff;
    return highpassed;
}

float TK84Detector::processOneChannel(float input, float& rmsState, float& peakState, float& hpfState)
{
    // Apply HPF using the correct per-channel state
    float hp = applyHPF(input, hpfState);

    // Convert to linear
    float absInput = std::abs(hp);

    // RMS follower
    rmsState = rmsState * (1.0f - rmsCoeff) + absInput * absInput * rmsCoeff;
    float rms = std::sqrt(std::max(rmsState, 1e-12f));

    // Peak follower with light release
    if (absInput > peakState)
        peakState = absInput;
    else
        peakState = peakState * (1.0f - peakCoeff * 0.35f) + absInput * peakCoeff * 0.35f;

    // Hybrid blend: more peak weighting on fast attack settings
    float peakWeight = isFastAttack(currentAttack) ? 0.82f : 0.38f;
    float rmsWeight = 1.0f - peakWeight;

    float hybrid = rmsWeight * rms + peakWeight * peakState;

    // Convert to dB relative to detector reference (-18 dBFS)
    float levelDB = 20.0f * std::log10(std::max(hybrid, 1e-12f)) - kDetectorReferenceDB;

    return levelDB;
}

float TK84Detector::processSample(float inputL, float inputR)
{
    // Process each channel with its own HPF state
    float detL = processOneChannel(inputL, rmsStateL, peakStateL, hpfStateL);
    float detR = processOneChannel(inputR, rmsStateR, peakStateR, hpfStateR);

    // Stereo linking
    float linked = 0.0f;

    if (stereoLinkAmount >= 0.999f)
    {
        // 100% link — use maximum (best imaging)
        linked = std::max(detL, detR);
    }
    else if (stereoLinkAmount <= 0.001f)
    {
        // Dual mono
        linked = (detL + detR) * 0.5f;
    }
    else
    {
        // Partial link — blend between max and average
        float maxVal = std::max(detL, detR);
        float avgVal = (detL + detR) * 0.5f;
        linked = maxVal * stereoLinkAmount + avgVal * (1.0f - stereoLinkAmount);
    }

    currentDetectionDB = linked;
    return linked;
}

void TK84Detector::processBlock(const float* inL, const float* inR, float* detectionOut, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        detectionOut[i] = processSample(inL[i], inR[i]);
    }
}

} // namespace TEAL::TK84