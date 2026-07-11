#ifndef TEAL_TK84_PLUGIN_PROCESSOR_H
#define TEAL_TK84_PLUGIN_PROCESSOR_H

#include <JuceHeader.h>
#include "TK84Engine.h"
#include "JUCE_ParameterLayout.h"

class TEALTK84AudioProcessor : public juce::AudioProcessor
{
public:
    TEALTK84AudioProcessor();
    ~TEALTK84AudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Public access to parameters (useful for editor)
    juce::AudioProcessorValueTreeState apvts;

    // Metering access for the editor
    float getCurrentGR() const;
    float getCurrentVU() const;

private:
    TEAL::TK84::TK84Engine engine;
    int currentProgramIndex = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TEALTK84AudioProcessor)
};

#endif // TEAL_TK84_PLUGIN_PROCESSOR_H
