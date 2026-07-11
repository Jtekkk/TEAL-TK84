#ifndef TEAL_TK84_PARAMETER_IDS_H
#define TEAL_TK84_PARAMETER_IDS_H

#include <JuceHeader.h>
#include "TEAL_TK84_Parameters.h"

// NOTE: named ParamIDs (plural) to avoid clashing with the `enum class ParamID`
// declared in TEAL_TK84_Parameters.h.
namespace TEAL::TK84::ParamIDs
{
    //==========================================================================
    // Parameter IDs (used with AudioProcessorValueTreeState)
    //==========================================================================

    static const juce::String threshold          = "threshold";
    static const juce::String ratio              = "ratio";
    static const juce::String attack             = "attack";
    static const juce::String release            = "release";
    static const juce::String transientLevel     = "transientLevel";
    static const juce::String transientIntensity = "transientIntensity";
    static const juce::String outputGain         = "outputGain";

    static const juce::String scHPF              = "scHPF";
    static const juce::String stereoLink         = "stereoLink";
    static const juce::String inputTrim          = "inputTrim";
    static const juce::String vuRef              = "vuRef";
    static const juce::String grMeterRange       = "grMeterRange";
    static const juce::String oversampling       = "oversampling";
    static const juce::String vcaMode            = "vcaMode";
    static const juce::String peakHold           = "peakHold";
    static const juce::String bypass             = "bypass";

    //==========================================================================
    // Helper to get default index for choice-based parameters
    //==========================================================================

    inline int getDefaultIndex(const juce::String& paramID)
    {
        if (paramID == ratio)              return static_cast<int>(kRatioDefault);
        if (paramID == attack)             return static_cast<int>(kAttackDefault);
        if (paramID == release)            return static_cast<int>(kReleaseDefault);
        if (paramID == scHPF)              return static_cast<int>(kSCHPFDefault);
        if (paramID == stereoLink)         return static_cast<int>(kStereoLinkDefault);
        if (paramID == vuRef)              return static_cast<int>(kVURefDefault);
        if (paramID == grMeterRange)       return static_cast<int>(kGRMeterRangeDefault);
        if (paramID == oversampling)       return static_cast<int>(kOversamplingDefault);
        if (paramID == vcaMode)            return static_cast<int>(kVCAModeDefault);
        if (paramID == peakHold)           return 1; // true
        if (paramID == bypass)             return 0; // false

        // For stepped float-style parameters, calculate index from default value
        if (paramID == threshold)          return thresholdIndexFromValue(kThresholdDefault);
        if (paramID == transientLevel)     return static_cast<int>((kTransientLevelDefault - kTransientLevelMin) / kTransientLevelStep);
        if (paramID == transientIntensity) return static_cast<int>(kTransientIntensityDefault / kTransientIntensityStep);
        if (paramID == outputGain)         return static_cast<int>((kOutputDefault - kOutputMin) / kOutputStep);
        if (paramID == inputTrim)          return static_cast<int>((kInputTrimDefault - kInputTrimMin) / kInputTrimStep);

        return 0;
    }
}

#endif // TEAL_TK84_PARAMETER_IDS_H
