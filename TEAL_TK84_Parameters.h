#pragma once
#include <array>
#include <string_view>
#include <cstdint>
#include <algorithm>
#include <cmath>

namespace TEAL::TK84 {

//==============================================================================
// TEAL TK84 Stereo VCA Compressor — Parameter Definitions (Rev. A Spec)
// All controls are stepped for perfect recall. No continuous values.
//==============================================================================

//------------------------------------------------------------------------------
// THRESHOLD: −40 dBFS to +10 dBFS, 2 dB steps (26 detents)
// Default: −10 dBFS
//------------------------------------------------------------------------------
constexpr int kNumThresholdSteps = 26;
constexpr float kThresholdMin = -40.0f;
constexpr float kThresholdMax = 10.0f;
constexpr float kThresholdStep = 2.0f;
constexpr float kThresholdDefault = -10.0f;

inline constexpr std::array<float, kNumThresholdSteps> kThresholdValues = [] {
    std::array<float, kNumThresholdSteps> arr{};
    for (int i = 0; i < kNumThresholdSteps; ++i)
        arr[i] = kThresholdMin + i * kThresholdStep;
    return arr;
}();

inline int thresholdIndexFromValue(float value) {
    // Snap to nearest step
    float clamped = std::clamp(value, kThresholdMin, kThresholdMax);
    int idx = static_cast<int>(std::round((clamped - kThresholdMin) / kThresholdStep));
    return std::clamp(idx, 0, kNumThresholdSteps - 1);
}

inline float thresholdValueFromIndex(int index) {
    return kThresholdValues[std::clamp(index, 0, kNumThresholdSteps - 1)];
}

//------------------------------------------------------------------------------
// RATIO: 8-position switch
// 1.5:1, 2:1, 3:1, 4:1, 6:1, 8:1, 10:1, ∞:1 (LIMIT)
// Default: 4:1
//------------------------------------------------------------------------------
enum class Ratio : uint8_t {
    Ratio_1_5 = 0,
    Ratio_2,
    Ratio_3,
    Ratio_4,
    Ratio_6,
    Ratio_8,
    Ratio_10,
    Limit,          // ∞:1
    Count
};

constexpr int kNumRatioSteps = static_cast<int>(Ratio::Count);
constexpr Ratio kRatioDefault = Ratio::Ratio_4;

inline constexpr std::array<std::string_view, kNumRatioSteps> kRatioLabels = {
    "1.5:1", "2:1", "3:1", "4:1", "6:1", "8:1", "10:1", "LIMIT"
};

inline constexpr std::array<float, kNumRatioSteps> kRatioValues = {
    1.5f, 2.0f, 3.0f, 4.0f, 6.0f, 8.0f, 10.0f, 1000.0f   // 1000 treated as infinity internally
};

inline float ratioValue(Ratio r) {
    return kRatioValues[static_cast<size_t>(r)];
}

//------------------------------------------------------------------------------
// ATTACK: 8-position switch (ms)
// 0.05, 0.1, 0.3, 1, 3, 10, 30, 100 ms
// Default: 10 ms
// Fast positions (≤0.1 ms) trigger 4× sidechain oversampling
//------------------------------------------------------------------------------
enum class Attack : uint8_t {
    Attack_0_05 = 0,
    Attack_0_1,
    Attack_0_3,
    Attack_1,
    Attack_3,
    Attack_10,
    Attack_30,
    Attack_100,
    Count
};

constexpr int kNumAttackSteps = static_cast<int>(Attack::Count);
constexpr Attack kAttackDefault = Attack::Attack_10;

inline constexpr std::array<float, kNumAttackSteps> kAttackTimesMs = {
    0.05f, 0.1f, 0.3f, 1.0f, 3.0f, 10.0f, 30.0f, 100.0f
};

inline constexpr std::array<std::string_view, kNumAttackSteps> kAttackLabels = {
    "0.05 ms", "0.1 ms", "0.3 ms", "1 ms", "3 ms", "10 ms", "30 ms", "100 ms"
};

inline bool isFastAttack(Attack a) {
    return a == Attack::Attack_0_05 || a == Attack::Attack_0_1;
}

//------------------------------------------------------------------------------
// RELEASE: 7-position + AUTO
// 50, 100, 200, 400, 800, 1600, 2400 ms + AUTO
// Default: AUTO
//------------------------------------------------------------------------------
enum class Release : uint8_t {
    Release_50 = 0,
    Release_100,
    Release_200,
    Release_400,
    Release_800,
    Release_1600,
    Release_2400,
    Auto,
    Count
};

constexpr int kNumReleaseSteps = static_cast<int>(Release::Count);
constexpr Release kReleaseDefault = Release::Auto;

inline constexpr std::array<float, 7> kReleaseTimesMs = {
    50.0f, 100.0f, 200.0f, 400.0f, 800.0f, 1600.0f, 2400.0f
};

inline constexpr std::array<std::string_view, kNumReleaseSteps> kReleaseLabels = {
    "50 ms", "100 ms", "200 ms", "400 ms", "800 ms", "1.6 s", "2.4 s", "AUTO"
};

//------------------------------------------------------------------------------
// TRANSIENT LEVEL: −12 dB to +12 dB, 1 dB steps (25 detents)
// Default: 0 dB (transparent)
//------------------------------------------------------------------------------
constexpr int kNumTransientLevelSteps = 25;
constexpr float kTransientLevelMin = -12.0f;
constexpr float kTransientLevelMax = 12.0f;
constexpr float kTransientLevelStep = 1.0f;
constexpr float kTransientLevelDefault = 0.0f;

inline constexpr std::array<float, kNumTransientLevelSteps> kTransientLevelValues = [] {
    std::array<float, kNumTransientLevelSteps> arr{};
    for (int i = 0; i < kNumTransientLevelSteps; ++i)
        arr[i] = kTransientLevelMin + i * kTransientLevelStep;
    return arr;
}();

//------------------------------------------------------------------------------
// TRANSIENT INTENSITY: 0 % to 100 %, 5 % steps (21 detents)
// Default: 50 %
//------------------------------------------------------------------------------
constexpr int kNumTransientIntensitySteps = 21;
constexpr float kTransientIntensityMin = 0.0f;
constexpr float kTransientIntensityMax = 100.0f;
constexpr float kTransientIntensityStep = 5.0f;
constexpr float kTransientIntensityDefault = 50.0f;

inline constexpr std::array<float, kNumTransientIntensitySteps> kTransientIntensityValues = [] {
    std::array<float, kNumTransientIntensitySteps> arr{};
    for (int i = 0; i < kNumTransientIntensitySteps; ++i)
        arr[i] = kTransientIntensityMin + i * kTransientIntensityStep;
    return arr;
}();

//------------------------------------------------------------------------------
// OUTPUT (Make-up Gain): −20 dB to +20 dB, 1 dB steps (41 detents)
// Default: 0 dB
//------------------------------------------------------------------------------
constexpr int kNumOutputSteps = 41;
constexpr float kOutputMin = -20.0f;
constexpr float kOutputMax = 20.0f;
constexpr float kOutputStep = 1.0f;
constexpr float kOutputDefault = 0.0f;

inline constexpr std::array<float, kNumOutputSteps> kOutputValues = [] {
    std::array<float, kNumOutputSteps> arr{};
    for (int i = 0; i < kNumOutputSteps; ++i)
        arr[i] = kOutputMin + i * kOutputStep;
    return arr;
}();

//------------------------------------------------------------------------------
// SECONDARY / SHIFT-LAYER CONTROLS
//------------------------------------------------------------------------------

// Sidechain HPF: Off, 60, 90, 120, 185 Hz
enum class SCHPF : uint8_t {
    Off = 0,
    Hz60,
    Hz90,
    Hz120,
    Hz185,
    Count
};

constexpr int kNumSCHPFSteps = static_cast<int>(SCHPF::Count);
constexpr SCHPF kSCHPFDefault = SCHPF::Off;

inline constexpr std::array<float, kNumSCHPFSteps> kSCHPFCutoffs = {
    0.0f, 60.0f, 90.0f, 120.0f, 185.0f
};

inline constexpr std::array<std::string_view, kNumSCHPFSteps> kSCHPFLabels = {
    "OFF", "60 Hz", "90 Hz", "120 Hz", "185 Hz"
};

// Stereo Link: 100%, 50%, 0% (dual mono)
enum class StereoLink : uint8_t {
    Link100 = 0,
    Link50,
    DualMono,   // 0%
    Count
};

constexpr int kNumStereoLinkSteps = static_cast<int>(StereoLink::Count);
constexpr StereoLink kStereoLinkDefault = StereoLink::Link100;

inline constexpr std::array<float, kNumStereoLinkSteps> kStereoLinkAmounts = {
    1.0f, 0.5f, 0.0f
};

inline constexpr std::array<std::string_view, kNumStereoLinkSteps> kStereoLinkLabels = {
    "100%", "50%", "DUAL MONO"
};

// Input Trim: ±6 dB, 0.5 dB steps
constexpr int kNumInputTrimSteps = 25; // -6.0 to +6.0 inclusive
constexpr float kInputTrimMin = -6.0f;
constexpr float kInputTrimMax = 6.0f;
constexpr float kInputTrimStep = 0.5f;
constexpr float kInputTrimDefault = 0.0f;

// VU Reference: −20 / −18 / −14 dBFS = 0 VU
enum class VURef : uint8_t {
    RefMinus20 = 0,
    RefMinus18,
    RefMinus14,
    Count
};

constexpr int kNumVURefSteps = static_cast<int>(VURef::Count);
constexpr VURef kVURefDefault = VURef::RefMinus18;

inline constexpr std::array<float, kNumVURefSteps> kVURefValues = {
    -20.0f, -18.0f, -14.0f
};

inline constexpr std::array<std::string_view, kNumVURefSteps> kVURefLabels = {
    "−20 dBFS", "−18 dBFS", "−14 dBFS"
};

// GR Meter Range: ×1 (0.5–20 dB) or ×0.5 (0.25–10 dB)
enum class GRMeterRange : uint8_t {
    Full = 0,   // ×1
    Half,       // ×0.5 high-res
    Count
};

constexpr GRMeterRange kGRMeterRangeDefault = GRMeterRange::Full;

// Oversampling (signal path, not sidechain)
enum class Oversampling : uint8_t {
    Off = 0,
    X2,
    X4,
    Count
};

constexpr Oversampling kOversamplingDefault = Oversampling::Off;

// VCA Model
enum class VCAMode : uint8_t {
    Clean = 0,      // No added nonlinearity
    Vintage,        // THAT 2181-class odd harmonic model under GR
    Count
};

constexpr VCAMode kVCAModeDefault = VCAMode::Vintage;

// Peak Hold for GR meter
enum class PeakHold : uint8_t {
    On = 0,
    Off,
    Count
};

constexpr PeakHold kPeakHoldDefault = PeakHold::On;

//==============================================================================
// Parameter ID enum for host automation / JUCE
//==============================================================================
enum class ParamID : uint32_t {
    Threshold = 0,
    Ratio,
    Attack,
    Release,
    TransientLevel,
    TransientIntensity,
    OutputGain,
    SCHPF,
    StereoLink,
    InputTrim,
    VURef,
    GRMeterRange,
    Oversampling,
    VCAMode,
    PeakHold,
    Bypass,          // Hard bypass
    Count
};

constexpr int kNumParameters = static_cast<int>(ParamID::Count);

// Human-readable names for host
inline constexpr std::array<std::string_view, kNumParameters> kParamNames = {
    "Threshold",
    "Ratio",
    "Attack",
    "Release",
    "Transient Level",
    "Transient Intensity",
    "Output",
    "SC HPF",
    "Stereo Link",
    "Input Trim",
    "VU Reference",
    "GR Meter Range",
    "Oversampling",
    "VCA Mode",
    "GR Peak Hold",
    "Bypass"
};

} // namespace TEAL::TK84