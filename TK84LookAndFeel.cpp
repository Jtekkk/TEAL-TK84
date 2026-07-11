#include "TK84LookAndFeel.h"

TK84LookAndFeel::TK84LookAndFeel()
{
    // Set overall dark theme colors
    setColour(juce::ResizableWindow::backgroundColourId, darkBg);
    setColour(juce::Slider::rotarySliderFillColourId, tealAccent);
    setColour(juce::Slider::rotarySliderOutlineColourId, knobHighlight);
}

void TK84LookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                       float sliderPosProportional, float rotaryStartAngle,
                                       float rotaryEndAngle, juce::Slider&)
{
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(4.0f);
    auto centre = bounds.getCentre();
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;

    // Background circle (knob body)
    g.setColour(knobDark);
    g.fillEllipse(bounds);

    // Subtle highlight ring
    g.setColour(knobHighlight);
    g.drawEllipse(bounds, 1.5f);

    // Teal outer ring (hardware feel)
    g.setColour(tealAccent.withAlpha(0.25f));
    g.drawEllipse(bounds.reduced(1.5f), 1.0f);

    // Calculate pointer angle
    float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    // Pointer line
    float pointerLength = radius * 0.72f;
    float pointerThickness = 2.2f;

    juce::Point<float> pointerEnd(
        centre.x + std::sin(angle) * pointerLength,
        centre.y - std::cos(angle) * pointerLength
    );

    g.setColour(tealAccent);
    g.drawLine(centre.x, centre.y, pointerEnd.x, pointerEnd.y, pointerThickness);

    // Small center dot
    g.setColour(tealAccent);
    g.fillEllipse(centre.x - 2.5f, centre.y - 2.5f, 5.0f, 5.0f);

    // Outer metallic rim
    g.setColour(juce::Colour(0xff4a4a4e));
    g.drawEllipse(bounds.reduced(0.5f), 1.0f);
}