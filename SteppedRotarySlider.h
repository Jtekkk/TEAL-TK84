#ifndef TEAL_TK84_STEPPED_ROTARY_SLIDER_H
#define TEAL_TK84_STEPPED_ROTARY_SLIDER_H

#include <JuceHeader.h>

/**
 * SteppedRotarySlider
 * A rotary slider that snaps to discrete steps, mimicking hardware stepped pots.
 * - Scroll wheel moves exactly one detent
 * - Double-click resets to default
 * - Drag moves smoothly but snaps on mouse up (or continuously if desired)
 */
class SteppedRotarySlider : public juce::Slider
{
public:
    SteppedRotarySlider();

    void setNumberOfSteps(int steps);
    void setDefaultValue(float value);

    // Override to enforce stepping
    double proportionOfLengthToValue(double proportion) override;
    double valueToProportionOfLength(double value) override;

    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;

private:
    int numSteps = 1;
    float defaultValue = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SteppedRotarySlider)
};

#endif // TEAL_TK84_STEPPED_ROTARY_SLIDER_H
