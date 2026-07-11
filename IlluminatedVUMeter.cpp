#include "IlluminatedVUMeter.h"

IlluminatedVUMeter::IlluminatedVUMeter()
{
    startTimerHz(60); // Smooth needle movement
}

void IlluminatedVUMeter::setLevel(float levelDB)
{
    currentLevel = levelDB;
}

void IlluminatedVUMeter::setReference(float refDBFS)
{
    referenceLevel = refDBFS;
}

void IlluminatedVUMeter::setBacklight(bool on)
{
    backlightOn = on;
    repaint();
}

void IlluminatedVUMeter::setPeakLED(bool on)
{
    peakLEDOn = on;
    repaint();
}

void IlluminatedVUMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(4.0f);

    // Background / face
    if (backlightOn)
    {
        g.setGradientFill(juce::ColourGradient(
            juce::Colour(0xff0a3a35),
            bounds.getCentreX(), bounds.getY(),
            juce::Colour(0xff001a17),
            bounds.getCentreX(), bounds.getBottom(),
            false));
        g.fillRoundedRectangle(bounds, 8.0f);

        // Subtle glow
        g.setColour(juce::Colour(0xff00c8b4).withAlpha(0.08f));
        g.fillRoundedRectangle(bounds.reduced(2), 6.0f);
    }
    else
    {
        g.setColour(juce::Colour(0xff1f1f23));
        g.fillRoundedRectangle(bounds, 8.0f);
    }

    // Border
    g.setColour(juce::Colour(0xff3a3a3e));
    g.drawRoundedRectangle(bounds, 8.0f, 2.0f);

    // VU Scale
    g.setColour(backlightOn ? juce::Colour(0xffe0e0e0) : juce::Colour(0xffaaaaaa));
    g.setFont(10.0f);

    // Simple scale labels
    auto scaleBounds = bounds.reduced(10, 15);
    g.drawText("-20", scaleBounds.getX(), scaleBounds.getBottom() - 12, 30, 12, juce::Justification::left);
    g.drawText("0", scaleBounds.getCentreX() - 15, scaleBounds.getY() + 5, 30, 12, juce::Justification::centred);
    g.drawText("+3", scaleBounds.getRight() - 25, scaleBounds.getBottom() - 12, 25, 12, juce::Justification::right);

    // Needle
    float normalized = juce::jlimit(0.0f, 1.0f, (currentLevel - referenceLevel + 20.0f) / 23.0f);

    float angle = juce::degreesToRadians(-45.0f + normalized * 90.0f);
    auto centre = bounds.getCentre();
    float needleLength = bounds.getHeight() * 0.55f;

    juce::Point<float> needleEnd(
        centre.x + std::sin(angle) * needleLength,
        centre.y - std::cos(angle) * needleLength
    );

    g.setColour(juce::Colour(0xffe8e8e8));
    g.drawLine(centre.x, centre.y, needleEnd.x, needleEnd.y, 1.8f);

    // Needle pivot
    g.setColour(juce::Colour(0xffcccccc));
    g.fillEllipse(centre.x - 3, centre.y - 3, 6, 6);

    // Peak LED
    if (peakLEDOn)
    {
        g.setColour(juce::Colour(0xffff3333));
        g.fillEllipse(bounds.getRight() - 18, bounds.getY() + 8, 10, 10);
        g.setColour(juce::Colour(0xffffaaaa));
        g.drawEllipse(bounds.getRight() - 18, bounds.getY() + 8, 10, 10, 1.0f);
    }
}

void IlluminatedVUMeter::resized()
{
}

void IlluminatedVUMeter::timerCallback()
{
    // Improved VU-style ballistics (approximation of ANSI standard)
    float target = currentLevel;

    // Attack is relatively fast, release is slower (classic VU behavior)
    if (target > displayedLevel)
        displayedLevel = displayedLevel * 0.82f + target * 0.18f;   // Attack
    else
        displayedLevel = displayedLevel * 0.97f + target * 0.03f;   // Release (~300ms feel)

    // Peak hold with slow decay
    if (currentLevel > peakHoldLevel)
    {
        peakHoldLevel = currentLevel;
        peakHoldDecay = 0.0f;
    }
    else
    {
        peakHoldDecay += 0.008f; // slow decay rate
        if (peakHoldDecay > 1.0f)
            peakHoldLevel = peakHoldLevel * 0.985f + currentLevel * 0.015f;
    }

    // Update peak LED (true peak near 0 dBFS)
    peakLEDOn = (currentLevel > -0.5f);

    repaint();
}
