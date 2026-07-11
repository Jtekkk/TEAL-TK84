#ifndef TEAL_TK84_ILLUMINATED_VU_METER_H
#define TEAL_TK84_ILLUMINATED_VU_METER_H

#include <JuceHeader.h>

/**
 * IlluminatedVUMeter
 * Analog-style VU meter with backlight, matching the TK84 spec.
 * - True VU ballistics (~300 ms integration)
 * - Illuminated teal face
 * - Switchable reference level
 * - True peak LED
 */
class IlluminatedVUMeter : public juce::Component,
                           private juce::Timer
{
public:
    IlluminatedVUMeter();
    ~IlluminatedVUMeter() override = default;

    void setLevel(float levelDB);           // Current level in dB (post output gain)
    void setReference(float refDBFS);       // e.g. -18.0f for 0 VU = -18 dBFS
    void setBacklight(bool on);
    void setPeakLED(bool on);

    float getPeakHoldLevel() const { return peakHoldLevel; }

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;

    float currentLevel = -100.0f;
    float displayedLevel = -100.0f;   // Smoothed for needle movement
    float referenceLevel = -18.0f;
    bool backlightOn = true;
    bool peakLEDOn = false;

    float peakHoldLevel = -100.0f;
    float peakHoldDecay = 0.0f;

    juce::Path needlePath;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IlluminatedVUMeter)
};

#endif // TEAL_TK84_ILLUMINATED_VU_METER_H
