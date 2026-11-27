#pragma once

#include <JuceHeader.h>

class RotaryKnob : public juce::Slider
{
public:
    RotaryKnob();
    ~RotaryKnob() override;

    class CustomLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        CustomLookAndFeel();
        void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, 
                             float sliderPos, const float rotaryStartAngle, 
                             const float rotaryEndAngle, juce::Slider& slider) override;
    private:
        juce::Colour knobColor;
        juce::Colour accentColor;
        juce::Colour pointerColor;
    };

private:
    CustomLookAndFeel customLookAndFeel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RotaryKnob)
};
