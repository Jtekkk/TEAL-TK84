#ifndef TEAL_TK84_GR_LED_METER_H
#define TEAL_TK84_GR_LED_METER_H

#include <JuceHeader.h>

/**
 * GRLEDMeter
 * 12-segment LED gain reduction meter with peak hold.
 * Matches the TK84 spec (amber LEDs, 0.5–20 dB scale).
 */
class GRLEDMeter : public juce::Component,
                   private juce::Timer
{
public:
    GRLEDMeter();
    ~GRLEDMeter() override = default;

    void setGRValue(float grDB);           // Current GR in dB (negative)
    void setPeakHold(bool shouldHold);
    void setScaleFactor(float scale);      // 1.0 = full, 0.5 = high-res mode

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;

    float currentGR = 0.0f;
    float peakGR = 0.0f;
    bool peakHoldEnabled = true;
    float scale = 1.0f;

    static constexpr int numSegments = 12;
    static constexpr float segmentValues[numSegments] = {
        0.5f, 1.0f, 1.5f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 8.0f, 10.0f, 15.0f, 20.0f
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GRLEDMeter)
};

#endif // TEAL_TK84_GR_LED_METER_H
