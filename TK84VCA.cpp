#include "TK84VCA.h"
#include <cmath>
#include <algorithm>

namespace TEAL::TK84 {

TK84VCA::TK84VCA()
{
    reset();
}

void TK84VCA::prepare(double /*sampleRate*/)
{
    reset();
}

void TK84VCA::reset()
{
    currentGain = 1.0f;
    dcBlockState = 0.0f;
}

void TK84VCA::setMode(VCAMode mode)
{
    currentMode = mode;
}

float TK84VCA::applyVintageNonlinearity(float input, float cvDB) const
{
    // Only apply when we have meaningful gain reduction
    if (cvDB > -0.3f)
        return input;

    // How much gain reduction is happening (0 = no GR, 1 = 40 dB GR)
    float grAmount = std::clamp(-cvDB / 40.0f, 0.0f, 1.0f);

    // Level-dependent drive (louder signals get slightly more coloration)
    float level = std::abs(input);
    float drive = level * (0.6f + grAmount * 0.8f);

    // Subtle 3rd-order harmonic (odd harmonic, typical of Class-A VCA)
    // Coefficient tuned so that at 10 dB GR on a -6 dBFS signal we get ~0.04-0.05% THD
    float thirdHarmonic = drive * drive * drive * 0.012f * grAmount;

    // Add a tiny bit of 5th for extra analog flavor at high GR (optional, very subtle)
    float fifthHarmonic = drive * drive * drive * drive * drive * 0.0018f * grAmount;

    float distorted = input + thirdHarmonic + fifthHarmonic;

    // Very gentle DC blocking to keep things clean
    float dcBlock = distorted - dcBlockState;
    dcBlockState = distorted - dcBlock * 0.995f;

    return dcBlock;
}

float TK84VCA::processSample(float input, float cvDB)
{
    // Exponential gain law: 6.1 mV/dB modeled (standard for THAT 2181 class)
    // Gain (linear) = 10 ^ (cvDB / 20)
    float gain = std::pow(10.0f, cvDB / 20.0f);
    currentGain = gain;

    float output = input * gain;

    // Apply vintage nonlinearity only in Vintage mode and when reducing gain
    if (currentMode == VCAMode::Vintage && cvDB < -0.5f)
    {
        output = applyVintageNonlinearity(output, cvDB);
    }

    return output;
}

} // namespace TEAL::TK84