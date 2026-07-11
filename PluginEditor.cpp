#include "PluginEditor.h"

TEALTK84AudioProcessorEditor::TEALTK84AudioProcessorEditor(TEALTK84AudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&lookAndFeel);
    setSize(720, 480);

    // Configure stepped sliders
    auto setupSlider = [this](SteppedRotarySlider& slider, int steps, const juce::String& name) {
        slider.setNumberOfSteps(steps);
        addAndMakeVisible(slider);
    };

    setupSlider(thresholdSlider,        26, "Threshold");
    setupSlider(ratioSlider,             8, "Ratio");
    setupSlider(attackSlider,            8, "Attack");
    setupSlider(releaseSlider,           8, "Release");
    setupSlider(transientLevelSlider,   25, "Transient Level");
    setupSlider(transientIntensitySlider, 21, "Transient Intensity");
    setupSlider(outputSlider,           41, "Output");

    addAndMakeVisible(grMeter);
    addAndMakeVisible(vuMeter);

    addAndMakeVisible(titleLabel);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xff00c8b4));
    titleLabel.setJustificationType(juce::Justification::centred);

    // Section labels styling
    auto styleSectionLabel = [](juce::Label& label) {
        label.setFont(juce::Font(11.0f, juce::Font::bold));
        label.setColour(juce::Label::textColourId, juce::Colour(0xff00c8b4));
        label.setJustificationType(juce::Justification::centred);
    };

    addAndMakeVisible(compressorLabel);   styleSectionLabel(compressorLabel);
    addAndMakeVisible(transientLabel);    styleSectionLabel(transientLabel);
    addAndMakeVisible(metersLabel);       styleSectionLabel(metersLabel);

    // Control labels
    auto setupLabel = [](juce::Label& label, const juce::String& text) {
        label.setText(text, juce::dontSendNotification);
        label.setFont(juce::Font(9.0f));
        label.setColour(juce::Label::textColourId, juce::Colour(0xffaaaaaa));
        label.setJustificationType(juce::Justification::centred);
    };

    setupLabel(thresholdLabel, "THRESHOLD");
    setupLabel(ratioLabel, "RATIO");
    setupLabel(attackLabel, "ATTACK");
    setupLabel(releaseLabel, "RELEASE");
    setupLabel(transientLevelLabel, "LEVEL");
    setupLabel(transientIntensityLabel, "INTENSITY");
    setupLabel(outputLabel, "OUTPUT");

    addAndMakeVisible(thresholdLabel);
    addAndMakeVisible(ratioLabel);
    addAndMakeVisible(attackLabel);
    addAndMakeVisible(releaseLabel);
    addAndMakeVisible(transientLevelLabel);
    addAndMakeVisible(transientIntensityLabel);
    addAndMakeVisible(outputLabel);

    // Wire sliders to APVTS parameters
    auto& apvts = audioProcessor.apvts;

    thresholdAttachment        = std::make_unique<SliderAttachment>(apvts, "threshold",        thresholdSlider);
    ratioAttachment            = std::make_unique<SliderAttachment>(apvts, "ratio",            ratioSlider);
    attackAttachment           = std::make_unique<SliderAttachment>(apvts, "attack",           attackSlider);
    releaseAttachment          = std::make_unique<SliderAttachment>(apvts, "release",          releaseSlider);
    transientLevelAttachment   = std::make_unique<SliderAttachment>(apvts, "transientLevel",   transientLevelSlider);
    transientIntensityAttachment = std::make_unique<SliderAttachment>(apvts, "transientIntensity", transientIntensitySlider);
    outputAttachment           = std::make_unique<SliderAttachment>(apvts, "outputGain",       outputSlider);

    // Start real-time metering updates (~30 Hz)
    startTimerHz(30);
}

void TEALTK84AudioProcessorEditor::paint(juce::Graphics& g)
{
    // Dark charcoal background
    g.fillAll(juce::Colour(0xff1a1a1e));

    // Top header gradient
    g.setGradientFill(juce::ColourGradient(
        juce::Colour(0xff25252a), 0, 0,
        juce::Colour(0xff1a1a1e), 0, 70, false));
    g.fillRect(0, 0, getWidth(), 70);

    // Subtle section dividers
    g.setColour(juce::Colour(0xff2a2a2e));
    g.drawHorizontalLine(115, 20, getWidth() - 20);
    g.drawHorizontalLine(240, 20, getWidth() - 130);
}

void TEALTK84AudioProcessorEditor::timerCallback()
{
    // Read current metering settings from APVTS
    int grRangeIndex = static_cast<int>(audioProcessor.apvts.getRawParameterValue("grMeterRange")->load());
    int vuRefIndex   = static_cast<int>(audioProcessor.apvts.getRawParameterValue("vuRef")->load());

    float grScale = (grRangeIndex == 1) ? 0.5f : 1.0f; // 0 = Full, 1 = High-Res
    float vuRef   = TEAL::TK84::kVURefValues[vuRefIndex];

    // Update GR Meter
    float grDB = audioProcessor.getCurrentGR();
    grMeter.setGRValue(grDB);
    grMeter.setScaleFactor(grScale);

    // Update VU Meter
    float vuLevel = audioProcessor.getCurrentVU();
    vuMeter.setLevel(vuLevel);
    vuMeter.setReference(vuRef);
}

void TEALTK84AudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(16);

    // Title bar
    titleLabel.setBounds(bounds.removeFromTop(42));

    // === COMPRESSOR SECTION ===
    compressorLabel.setBounds(bounds.removeFromTop(18));

    auto compArea = bounds.removeFromTop(115);

    int knobW = 68;
    int gap = 10;

    // Row 1: Threshold + Ratio
    auto row1 = compArea.removeFromTop(58);
    thresholdSlider.setBounds(row1.removeFromLeft(knobW));
    thresholdLabel.setBounds(row1.removeFromLeft(knobW).removeFromTop(14));
    row1.removeFromLeft(gap);
    ratioSlider.setBounds(row1.removeFromLeft(knobW));
    ratioLabel.setBounds(row1.removeFromLeft(knobW).removeFromTop(14));

    // Row 2: Attack + Release
    auto row2 = compArea.removeFromTop(57);
    attackSlider.setBounds(row2.removeFromLeft(knobW));
    attackLabel.setBounds(row2.removeFromLeft(knobW).removeFromTop(14));
    row2.removeFromLeft(gap);
    releaseSlider.setBounds(row2.removeFromLeft(knobW));
    releaseLabel.setBounds(row2.removeFromLeft(knobW).removeFromTop(14));

    bounds.removeFromTop(8);

    // === TRANSIENT SECTION ===
    transientLabel.setBounds(bounds.removeFromTop(18));

    auto transArea = bounds.removeFromTop(115);

    // Transient Level + Intensity
    auto tRow = transArea.removeFromTop(58);
    transientLevelSlider.setBounds(tRow.removeFromLeft(knobW));
    transientLevelLabel.setBounds(tRow.removeFromLeft(knobW).removeFromTop(14));
    tRow.removeFromLeft(gap);
    transientIntensitySlider.setBounds(tRow.removeFromLeft(knobW));
    transientIntensityLabel.setBounds(tRow.removeFromLeft(knobW).removeFromTop(14));

    // Output Gain
    auto tRow2 = transArea.removeFromTop(57);
    outputSlider.setBounds(tRow2.removeFromLeft(knobW));
    outputLabel.setBounds(tRow2.removeFromLeft(knobW).removeFromTop(14));

    // === METERS (Right side) ===
    auto meterArea = bounds.removeFromRight(105);

    metersLabel.setBounds(meterArea.removeFromTop(18));

    grMeter.setBounds(meterArea.removeFromTop(165).reduced(6));
    vuMeter.setBounds(meterArea.removeFromTop(195).reduced(8));
}
