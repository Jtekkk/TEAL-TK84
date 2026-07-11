#include "GRLEDMeter.h"

GRLEDMeter::GRLEDMeter()
{
    startTimerHz(30); // Smooth updates
}

void GRLEDMeter::setGRValue(float grDB)
{
    currentGR = juce::jmin(0.0f, grDB); // GR is negative or zero

    if (peakHoldEnabled && currentGR < peakGR)
        peakGR = currentGR;
}

void GRLEDMeter::setPeakHold(bool shouldHold)
{
    peakHoldEnabled = shouldHold;
    if (!shouldHold)
        peakGR = currentGR;
}

void GRLEDMeter::setScaleFactor(float newScale)
{
    scale = newScale;
}

void GRLEDMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    float segmentHeight = bounds.getHeight() / numSegments;
    float segmentWidth = bounds.getWidth();

    for (int i = 0; i < numSegments; ++i)
    {
        float segmentTop = bounds.getY() + i * segmentHeight;
        juce::Rectangle<float> segRect(bounds.getX(), segmentTop, segmentWidth, segmentHeight * 0.85f);

        float threshold = -segmentValues[i] * scale;

        bool isLit = (currentGR <= threshold);

        // Peak hold segment
        bool isPeak = peakHoldEnabled && (peakGR <= threshold) && (peakGR < currentGR - 0.1f);

        juce::Colour colour;

        if (isLit)
            colour = juce::Colour(0xffffaa00); // Amber
        else if (isPeak)
            colour = juce::Colour(0xffffaa00).withAlpha(0.4f);
        else
            colour = juce::Colour(0xff2a2a2e);

        g.setColour(colour);
        g.fillRoundedRectangle(segRect, 2.0f);

        // Subtle border
        g.setColour(juce::Colours::black.withAlpha(0.6f));
        g.drawRoundedRectangle(segRect, 2.0f, 1.0f);
    }
}

void GRLEDMeter::resized()
{
}

void GRLEDMeter::timerCallback()
{
    // Slow peak hold decay
    if (peakHoldEnabled && peakGR < currentGR)
    {
        peakGR = peakGR * 0.985f + currentGR * 0.015f;
        repaint();
    }
    else if (std::abs(peakGR - currentGR) > 0.01f)
    {
        repaint();
    }
}
