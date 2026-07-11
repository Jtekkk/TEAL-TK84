#ifndef TEAL_TK84_PARAMETER_LAYOUT_H
#define TEAL_TK84_PARAMETER_LAYOUT_H

#include <JuceHeader.h>
#include "TEAL_TK84_Parameters.h"

namespace TEAL::TK84 {

/**
 * Creates the complete AudioProcessorValueTreeState parameter layout
 * for the TEAL TK84 Stereo VCA Compressor.
 *
 * All primary controls use AudioParameterChoice to enforce stepped behavior.
 */
inline AudioProcessorValueTreeState::ParameterLayout createTK84ParameterLayout()
{
    using AP = AudioProcessorValueTreeState;
    std::vector<std::unique_ptr<RangedAudioParameter>> params;

    //==========================================================================
    // PRIMARY CONTROLS
    //==========================================================================

    // --- THRESHOLD (-40 ... +10 dB, 2 dB steps, 26 positions) ---
    {
        juce::StringArray labels;
        for (float v : kThresholdValues)
            labels.add(juce::String(v, 0) + " dB");

        params.push_back(std::make_unique<AudioParameterChoice>(
            ParameterID{ "threshold", 1 },
            "Threshold",
            labels,
            thresholdIndexFromValue(kThresholdDefault)
        ));
    }

    // --- RATIO (8 positions) ---
    params.push_back(std::make_unique<AudioParameterChoice>(
        ParameterID{ "ratio", 1 },
        "Ratio",
        StringArray{ "1.5:1", "2:1", "3:1", "4:1", "6:1", "8:1", "10:1", "LIMIT" },
        static_cast<int>(kRatioDefault)
    ));

    // --- ATTACK (8 positions) ---
    params.push_back(std::make_unique<AudioParameterChoice>(
        ParameterID{ "attack", 1 },
        "Attack",
        StringArray{ "0.05 ms", "0.1 ms", "0.3 ms", "1 ms", "3 ms", "10 ms", "30 ms", "100 ms" },
        static_cast<int>(kAttackDefault)
    ));

    // --- RELEASE (8 positions including AUTO) ---
    params.push_back(std::make_unique<AudioParameterChoice>(
        ParameterID{ "release", 1 },
        "Release",
        StringArray{ "50 ms", "100 ms", "200 ms", "400 ms", "800 ms", "1.6 s", "2.4 s", "AUTO" },
        static_cast<int>(kReleaseDefault)
    ));

    // --- TRANSIENT LEVEL (-12 ... +12 dB, 1 dB steps) ---
    {
        juce::StringArray labels;
        for (float v : kTransientLevelValues)
            labels.add(juce::String(v, 0) + " dB");

        params.push_back(std::make_unique<AudioParameterChoice>(
            ParameterID{ "transientLevel", 1 },
            "Transient Level",
            labels,
            static_cast<int>((kTransientLevelDefault - kTransientLevelMin) / kTransientLevelStep)
        ));
    }

    // --- TRANSIENT INTENSITY (0 ... 100%, 5% steps) ---
    {
        juce::StringArray labels;
        for (float v : kTransientIntensityValues)
            labels.add(juce::String(static_cast<int>(v)) + "%");

        params.push_back(std::make_unique<AudioParameterChoice>(
            ParameterID{ "transientIntensity", 1 },
            "Transient Intensity",
            labels,
            static_cast<int>(kTransientIntensityDefault / kTransientIntensityStep)
        ));
    }

    // --- OUTPUT GAIN (-20 ... +20 dB, 1 dB steps) ---
    {
        juce::StringArray labels;
        for (float v : kOutputValues)
            labels.add(juce::String(v, 0) + " dB");

        params.push_back(std::make_unique<AudioParameterChoice>(
            ParameterID{ "outputGain", 1 },
            "Output",
            labels,
            static_cast<int>((kOutputDefault - kOutputMin) / kOutputStep)
        ));
    }

    //==========================================================================
    // SECONDARY / SHIFT-LAYER CONTROLS
    //==========================================================================

    // --- SIDECHAIN HPF ---
    params.push_back(std::make_unique<AudioParameterChoice>(
        ParameterID{ "scHPF", 1 },
        "SC HPF",
        StringArray{ "OFF", "60 Hz", "90 Hz", "120 Hz", "185 Hz" },
        static_cast<int>(kSCHPFDefault)
    ));

    // --- STEREO LINK ---
    params.push_back(std::make_unique<AudioParameterChoice>(
        ParameterID{ "stereoLink", 1 },
        "Stereo Link",
        StringArray{ "100%", "50%", "DUAL MONO" },
        static_cast<int>(kStereoLinkDefault)
    ));

    // --- INPUT TRIM (±6 dB, 0.5 dB steps) ---
    {
        juce::StringArray labels;
        for (int i = 0; i < kNumInputTrimSteps; ++i)
        {
            float val = kInputTrimMin + i * kInputTrimStep;
            labels.add(juce::String(val, 1) + " dB");
        }

        params.push_back(std::make_unique<AudioParameterChoice>(
            ParameterID{ "inputTrim", 1 },
            "Input Trim",
            labels,
            static_cast<int>((kInputTrimDefault - kInputTrimMin) / kInputTrimStep)
        ));
    }

    // --- VU REFERENCE ---
    params.push_back(std::make_unique<AudioParameterChoice>(
        ParameterID{ "vuRef", 1 },
        "VU Reference",
        StringArray{ "−20 dBFS", "−18 dBFS", "−14 dBFS" },
        static_cast<int>(kVURefDefault)
    ));

    // --- GR METER RANGE ---
    params.push_back(std::make_unique<AudioParameterChoice>(
        ParameterID{ "grMeterRange", 1 },
        "GR Meter Range",
        StringArray{ "Full (0.5–20 dB)", "High-Res (0.25–10 dB)" },
        static_cast<int>(kGRMeterRangeDefault)
    ));

    // --- OVERSAMPLING ---
    params.push_back(std::make_unique<AudioParameterChoice>(
        ParameterID{ "oversampling", 1 },
        "Oversampling",
        StringArray{ "Off", "2×", "4×" },
        static_cast<int>(kOversamplingDefault)
    ));

    // --- VCA MODE ---
    params.push_back(std::make_unique<AudioParameterChoice>(
        ParameterID{ "vcaMode", 1 },
        "VCA Mode",
        StringArray{ "Clean", "Vintage" },
        static_cast<int>(kVCAModeDefault)
    ));

    // --- GR PEAK HOLD ---
    params.push_back(std::make_unique<AudioParameterBool>(
        ParameterID{ "peakHold", 1 },
        "GR Peak Hold",
        true
    ));

    // --- BYPASS ---
    params.push_back(std::make_unique<AudioParameterBool>(
        ParameterID{ "bypass", 1 },
        "Bypass",
        false
    ));

    return { params.begin(), params.end() };
}

} // namespace TEAL::TK84

#endif // TEAL_TK84_PARAMETER_LAYOUT_H
