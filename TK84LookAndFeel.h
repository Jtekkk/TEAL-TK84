#pragma once
#include <JuceHeader.h>

/**
 * TK84LookAndFeel
 * Custom dark charcoal + teal hardware-inspired LookAndFeel for the TEAL TK84.
 */
class TK84LookAndFeel : public juce::LookAndFeel_V4
{
public:
    TK84LookAndFeel();

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override;

private:
    juce::Colour tealAccent   { 0xff00c8b4 };
    juce::Colour darkBg       { 0xff1a1a1e };
    juce::Colour knobDark     { 0xff2a2a2e };
    juce::Colour knobHighlight{ 0xff3a3a3e };
};