#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "TK84ParameterIDs.h"

namespace
{
    // Factory presets based on the TK84 engineering spec recommendations
    const juce::StringArray factoryPresetNames = {
        "Init",
        "Bus Glue",
        "Drum Bus Punch",
        "Parallel Crush",
        "Limit",
        "Transient Restore"
    };

    struct PresetValues
    {
        float threshold;
        int   ratioIndex;
        int   attackIndex;
        int   releaseIndex;
        float transientLevel;
        float transientIntensity;
        float outputGain;
        int   scHPFIndex;
        int   stereoLinkIndex;
    };

    const std::vector<PresetValues> factoryPresets = {
        // 0. Init
        { -10.0f, 3, 5, 7,  0.0f, 50.0f,  0.0f, 0, 0 },
        // 1. Bus Glue (from spec)
        { -10.0f, 1, 6, 7,  2.0f, 40.0f,  0.0f, 2, 0 },
        // 2. Drum Bus Punch (from spec)
        { -8.0f,  3, 5, 2,  6.0f, 65.0f,  0.0f, 0, 0 },
        // 3. Parallel Crush
        { -6.0f,  5, 2, 3,  4.0f, 80.0f, -3.0f, 0, 0 },
        // 4. Limit
        { -4.0f,  7, 0, 1,  0.0f, 30.0f,  0.0f, 3, 0 },
        // 5. Transient Restore
        { -12.0f, 2, 5, 7,  5.0f, 55.0f,  0.0f, 0, 0 },
        // 6. Vocal Bus
        { -14.0f, 2, 4, 6,  3.0f, 45.0f,  1.0f, 2, 0 },
        // 7. Master Transparent
        { -8.0f,  1, 7, 7,  1.0f, 35.0f,  0.0f, 0, 0 },
        // 8. Master Aggressive
        { -5.0f,  4, 3, 4,  2.0f, 60.0f, -2.0f, 0, 0 },
        // 9. Kick & Bass Glue
        { -6.0f,  3, 6, 5,  4.0f, 50.0f, -1.0f, 3, 0 },
        // 10. Full Mix Gentle
        { -12.0f, 1, 7, 7,  2.0f, 30.0f,  0.0f, 0, 0 },
        // 11. Full Mix Pump
        { -7.0f,  3, 2, 3,  5.0f, 70.0f, -4.0f, 0, 0 },
        // 12. Snare Enhancer
        { -9.0f,  4, 1, 2,  7.0f, 75.0f,  2.0f, 0, 0 },
        // 13. Overhead Tame
        { -11.0f, 2, 5, 6,  3.0f, 40.0f, -2.0f, 2, 0 },
        // 14. Parallel Drums
        { -4.0f,  6, 2, 2,  8.0f, 85.0f, -6.0f, 0, 0 },
        // 15. Low End Control
        { -5.0f,  3, 6, 5,  2.0f, 55.0f, -3.0f, 3, 0 },
        // 16. High End Tame
        { -10.0f, 2, 4, 6,  1.0f, 45.0f, -2.0f, 4, 0 },
        // 17. Midrange Focus
        { -9.0f,  3, 3, 4,  4.0f, 50.0f,  1.0f, 2, 0 },
        // 18. Vintage Style
        { -7.0f,  4, 5, 5,  3.0f, 65.0f, -1.0f, 0, 0 },
        // 19. Modern Clean
        { -13.0f, 1, 7, 7,  2.0f, 25.0f,  0.0f, 0, 0 },
        // 20. Broadcast
        { -8.0f,  2, 4, 3,  3.0f, 55.0f, -2.0f, 2, 0 },
        // 21. Podcast/Stream
        { -11.0f, 1, 6, 6,  2.0f, 40.0f,  0.0f, 0, 0 },
        // 22. EDM Sidechain
        { -3.0f,  5, 0, 1,  6.0f, 90.0f, -5.0f, 0, 0 },
        // 23. Acoustic Bus
        { -14.0f, 1, 7, 7,  3.0f, 35.0f,  1.0f, 0, 0 },
        // 24. Orchestral
        { -12.0f, 1, 7, 7,  2.0f, 30.0f,  0.0f, 0, 0 },
        // 25. Hip Hop Drums
        { -5.0f,  4, 2, 2,  7.0f, 80.0f, -4.0f, 0, 0 },
        // 26. Rock Bus
        { -7.0f,  3, 3, 4,  5.0f, 60.0f, -2.0f, 0, 0 },
        // 27. Finalizer
        { -6.0f,  3, 4, 5,  4.0f, 50.0f,  0.0f, 0, 0 }
    };
}

TEALTK84AudioProcessor::TEALTK84AudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMETERS", TEAL::TK84::createTK84ParameterLayout())
{
}

void TEALTK84AudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    engine.prepare(sampleRate, samplesPerBlock);
}

void TEALTK84AudioProcessor::releaseResources()
{
}

bool TEALTK84AudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void TEALTK84AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Read parameters from APVTS using clean ParamID namespace
    using namespace TEAL::TK84;
    using namespace TEAL::TK84::ParamID;

    auto getChoice = [&](const juce::String& id) {
        return static_cast<int>(apvts.getRawParameterValue(id)->load());
    };

    float thresholdDB = kThresholdValues[getChoice(threshold)];
    auto ratio        = static_cast<Ratio>(getChoice(ratio));
    auto attack       = static_cast<Attack>(getChoice(attack));
    auto releaseMode  = static_cast<Release>(getChoice(release));

    float transientLevel     = kTransientLevelValues[getChoice(transientLevel)];
    float transientIntensity = kTransientIntensityValues[getChoice(transientIntensity)];
    float outputGainDB       = kOutputValues[getChoice(outputGain)];

    auto scHPF      = static_cast<SCHPF>(getChoice(scHPF));
    auto stereoLink = static_cast<StereoLink>(getChoice(stereoLink));
    float inputTrim = kInputTrimMin + getChoice(inputTrim) * kInputTrimStep;

    auto vuRef        = static_cast<VURef>(getChoice(vuRef));
    auto grRange      = static_cast<GRMeterRange>(getChoice(grMeterRange));
    auto oversampling = static_cast<Oversampling>(getChoice(oversampling));
    auto vcaMode      = static_cast<VCAMode>(getChoice(vcaMode));
    bool peakHold     = getChoice(peakHold) > 0.5f;
    bool bypass       = getChoice(bypass) > 0.5f;

    // Update DSP engine
    engine.setParameters(
        thresholdDB,
        ratio,
        attack,
        releaseMode,
        transientLevel,
        transientIntensity,
        outputGainDB,
        scHPF,
        stereoLink,
        inputTrim,
        vuRef,
        grRange,
        oversampling,
        vcaMode,
        bypass
    );

    // Process audio
    float* channelDataL = buffer.getWritePointer(0);
    float* channelDataR = buffer.getWritePointer(1);

    engine.processBlock(channelDataL, channelDataR,
                        channelDataL, channelDataR,
                        buffer.getNumSamples());
}

juce::AudioProcessorEditor* TEALTK84AudioProcessor::createEditor()
{
    return new TEALTK84AudioProcessorEditor(*this);
}

void TEALTK84AudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void TEALTK84AudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// Preset / Program Handling
//==============================================================================

int TEALTK84AudioProcessor::getNumPrograms()
{
    return static_cast<int>(factoryPresetNames.size());
}

int TEALTK84AudioProcessor::getCurrentProgram()
{
    return currentProgramIndex;
}

void TEALTK84AudioProcessor::setCurrentProgram(int index)
{
    if (index < 0 || index >= getNumPrograms())
        return;

    currentProgramIndex = index;

    if (index == 0)
        return; // "Init" - do nothing special

    const auto& preset = factoryPresets[index];

    auto setChoice = [this](const juce::String& id, int valueIndex)
    {
        if (auto* param = apvts.getParameter(id))
            param->setValueNotifyingHost(param->getNormalisableRange().convertTo0to1(static_cast<float>(valueIndex)));
    };

    auto setSteppedFloat = [this](const juce::String& id, float value)
    {
        int index = 0;

        if (id == "threshold")
            index = TEAL::TK84::thresholdIndexFromValue(value);
        else if (id == "transientLevel")
            index = static_cast<int>((value - TEAL::TK84::kTransientLevelMin) / TEAL::TK84::kTransientLevelStep);
        else if (id == "transientIntensity")
            index = static_cast<int>(value / TEAL::TK84::kTransientIntensityStep);
        else if (id == "outputGain")
            index = static_cast<int>((value - TEAL::TK84::kOutputMin) / TEAL::TK84::kOutputStep);

        setChoice(id, index);
    };

    setSteppedFloat("threshold", preset.threshold);
    setChoice("ratio", preset.ratioIndex);
    setChoice("attack", preset.attackIndex);
    setChoice("release", preset.releaseIndex);
    setSteppedFloat("transientLevel", preset.transientLevel);
    setSteppedFloat("transientIntensity", preset.transientIntensity);
    setSteppedFloat("outputGain", preset.outputGain);
    setChoice("scHPF", preset.scHPFIndex);
    setChoice("stereoLink", preset.stereoLinkIndex);
}

const juce::String TEALTK84AudioProcessor::getProgramName(int index)
{
    return factoryPresetNames[index];
}

void TEALTK84AudioProcessor::changeProgramName(int /*index*/, const juce::String& /*newName*/)
{
    // Factory presets are read-only for now
}

float TEALTK84AudioProcessor::getCurrentGR() const
{
    return engine.getCurrentGR();
}

float TEALTK84AudioProcessor::getCurrentVU() const
{
    return engine.getCurrentVU();
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TEALTK84AudioProcessor();
}
