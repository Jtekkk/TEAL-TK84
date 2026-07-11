#ifndef TEAL_TK84_PLUGIN_EDITOR_H
#define TEAL_TK84_PLUGIN_EDITOR_H

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "SteppedRotarySlider.h"
#include "GRLEDMeter.h"
#include "IlluminatedVUMeter.h"
#include "TK84LookAndFeel.h"

class TEALTK84AudioProcessorEditor : public juce::AudioProcessorEditor,
                                       private juce::Timer
{
public:
    TEALTK84AudioProcessorEditor(TEALTK84AudioProcessor&);
    ~TEALTK84AudioProcessorEditor() override
    {
        setLookAndFeel(nullptr);
    }

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

private:
    TEALTK84AudioProcessor& audioProcessor;

    // Custom components
    SteppedRotarySlider thresholdSlider, ratioSlider, attackSlider, releaseSlider;
    SteppedRotarySlider transientLevelSlider, transientIntensitySlider, outputSlider;

    GRLEDMeter grMeter;
    IlluminatedVUMeter vuMeter;

    juce::Label titleLabel{"title", "TEAL TK84"};

    TK84LookAndFeel lookAndFeel;

    // Section labels
    juce::Label compressorLabel{"comp", "COMPRESSOR"};
    juce::Label transientLabel{"trans", "TRANSIENT"};
    juce::Label metersLabel{"meters", "METERS"};

    // Control labels
    juce::Label thresholdLabel, ratioLabel, attackLabel, releaseLabel;
    juce::Label transientLevelLabel, transientIntensityLabel, outputLabel;

    // APVTS Attachments (must be declared after the sliders they attach to)
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    std::unique_ptr<SliderAttachment> thresholdAttachment;
    std::unique_ptr<SliderAttachment> ratioAttachment;
    std::unique_ptr<SliderAttachment> attackAttachment;
    std::unique_ptr<SliderAttachment> releaseAttachment;
    std::unique_ptr<SliderAttachment> transientLevelAttachment;
    std::unique_ptr<SliderAttachment> transientIntensityAttachment;
    std::unique_ptr<SliderAttachment> outputAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TEALTK84AudioProcessorEditor)
};

#endif // TEAL_TK84_PLUGIN_EDITOR_H
