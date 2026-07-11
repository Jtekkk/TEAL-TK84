#include "SteppedRotarySlider.h"

SteppedRotarySlider::SteppedRotarySlider()
{
    setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    setRotaryParameters(juce::MathConstants<float>::pi * 1.2f,
                        juce::MathConstants<float>::pi * 2.8f,
                        true);
}

void SteppedRotarySlider::setNumberOfSteps(int steps)
{
    numSteps = juce::jmax(1, steps);
}

void SteppedRotarySlider::setDefaultValue(float value)
{
    defaultValue = value;
}

double SteppedRotarySlider::proportionOfLengthToValue(double proportion)
{
    // Snap to nearest step
    double stepped = std::round(proportion * (numSteps - 1)) / (numSteps - 1);
    return stepped;
}

double SteppedRotarySlider::valueToProportionOfLength(double value)
{
    return value; // Already stepped from the parameter
}

void SteppedRotarySlider::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    // Move exactly one step per wheel tick
    double currentValue = getValue();
    double stepSize = 1.0 / (numSteps - 1);

    if (wheel.deltaY > 0)
        setValue(currentValue + stepSize, juce::sendNotificationSync);
    else if (wheel.deltaY < 0)
        setValue(currentValue - stepSize, juce::sendNotificationSync);
}
