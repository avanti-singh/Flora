#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "RotaryKnob.h"

class BotanicalHarmonicGranularAudioProcessor;

class FloralPixelLookAndFeel : public juce::LookAndFeel_V4
{
public:
    FloralPixelLookAndFeel()
    {
        background = juce::Colour(245, 230, 240); // light floral pink
        accent = juce::Colour(255, 105, 180);     // vibrant pink
        text = juce::Colours::black;
        hover = accent.brighter(0.2f);
    }

    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
        const juce::Colour& backgroundColour,
        bool isMouseOverButton, bool isButtonDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);

        juce::Colour base = backgroundColour;

        if (isButtonDown)
        base = juce::Colours::deeppink;
        else if (isMouseOverButton)
        base = juce::Colours::lightpink;

        g.setColour (base);
        g.fillRoundedRectangle (bounds, 6.0f);

        g.setColour (juce::Colours::black);
        g.drawRoundedRectangle (bounds, 6.0f, 1.0f);
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
                        bool /*isMouseOverButton*/, bool /*isButtonDown*/) override
    {
        g.setColour(text);
        g.setFont(juce::Font("Press Start 2P", 12.0f, juce::Font::plain)); // 8-bit aesthetic
        g.drawFittedText(button.getButtonText(), button.getLocalBounds(), juce::Justification::centred, 1);
    }

    void drawComboBox (juce::Graphics& g, int width, int height, bool,
        int, int, int, int, juce::ComboBox& box) override
    {
        auto bounds = juce::Rectangle<int>(0, 0, width, height);
        g.setColour (juce::Colour(255, 182, 193));
        g.fillRoundedRectangle(bounds.toFloat(), 4.0f);
        g.setColour (juce::Colour(255, 182, 193));
        g.drawRoundedRectangle(bounds.toFloat(), 4.0f, 1.0f);
    }

    juce::Label* createComboBoxTextBox (juce::ComboBox& box) override
    {
        auto* l = LookAndFeel_V4::createComboBoxTextBox(box);
        l->setColour(juce::Label::textColourId, juce::Colours::black);
        return l;
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool isMouseOver, bool isButtonDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(2);
        auto bg = button.getToggleState() ? accent : background;
        g.setColour(bg);
        g.fillRoundedRectangle(bounds, 4.0f);

        g.setColour(text);
        g.setFont(juce::Font("Press Start 2P", 11.0f, juce::Font::plain));
        g.drawFittedText(button.getButtonText(), button.getLocalBounds(), juce::Justification::centred, 1);

        g.setColour(text);
        g.drawRoundedRectangle(bounds, 4.0f, 1.5f);
    }

private:
    juce::Colour background, accent, text, hover;
};

class ImageKnobLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ImageKnobLookAndFeel()
    {
        knobImage = juce::ImageCache::getFromMemory(BinaryData::rotaryknob_png, BinaryData::rotaryknob_pngSize);
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider& slider) override
    {
        const float radius = juce::jmin(width, height) * 0.5f;
        const float centerX = x + width * 0.5f;
        const float centerY = y + height * 0.5f;

        // Draw knob image with scaling
        const float scale = 0.65f; // Adjust to make knob larger or smaller
        const float imgWidth = knobImage.getWidth();
        const float imgHeight = knobImage.getHeight();
        const float drawWidth = imgWidth * scale;
        const float drawHeight = imgHeight * scale;

        juce::AffineTransform transform =
            juce::AffineTransform::scale(scale)
            .translated(centerX - drawWidth * 0.5f, centerY - drawHeight * 0.5f);

        g.drawImageTransformed(knobImage, transform);

        // Draw progress arc
        const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        juce::Path arcPath;
        float arcThickness = radius * 0.15f;
        arcPath.addCentredArc(centerX, centerY, radius - arcThickness, radius - arcThickness,
                              0.0f, rotaryStartAngle, angle, true);

        g.setColour(juce::Colour(255, 182, 193));  // Light pink
        g.strokePath(arcPath, juce::PathStrokeType(arcThickness, juce::PathStrokeType::curved));
    }

private:
    juce::Image knobImage;
};



//==============================================================================
// Custom component for the bento box sections
class BentoBox : public juce::Component
{
public:
    BentoBox(const juce::String& boxTitle);
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void addAndMakeVisible(juce::Component& component);
    
private:
    juce::String title;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BentoBox)
};

class BotanicalHarmonicGranularAudioProcessorEditor : public juce::AudioProcessorEditor,
                                                      public juce::AudioProcessorValueTreeState::Listener
{
public:
    BotanicalHarmonicGranularAudioProcessorEditor(BotanicalHarmonicGranularAudioProcessor&);
    ~BotanicalHarmonicGranularAudioProcessorEditor() override;

    FloralPixelLookAndFeel floralLookAndFeel;
    ImageKnobLookAndFeel customKnobLook;

    juce::Image backgroundImage;

    void paint(juce::Graphics&) override;
    void resized() override;
    
    // AudioProcessorValueTreeState::Listener implementation
    void parameterChanged(const juce::String& parameterID, float newValue) override;

private:
    BotanicalHarmonicGranularAudioProcessor& processor;
    
    // Bento boxes
    BentoBox grainSection;
    BentoBox fxSection;
    
    // Grain section controls
    RotaryKnob grainSizeSlider;
    juce::Label grainSizeLabel;
    juce::Label grainSizeValueLabel;
    
    RotaryKnob densitySlider;
    juce::Label densityLabel;
    juce::Label densityValueLabel;
    
    RotaryKnob spraySlider;
    juce::Label sprayLabel;
    juce::Label sprayValueLabel;
    
    RotaryKnob dryWetSlider;
    juce::Label dryWetLabel;
    juce::Label dryWetValueLabel;
    
    // Direction buttons
    juce::TextButton forwardButton;
    juce::TextButton backwardButton;
    juce::TextButton bothButton;
    juce::Label directionLabel;
    
    // Envelope dropdown
    juce::ComboBox envelopeComboBox;
    juce::Label envelopeLabel;
    
    // FX section controls
    RotaryKnob delaySlider;
    juce::Label delayLabel;
    juce::Label delayValueLabel;
    
    // Reverb control
    RotaryKnob reverbSlider;
    juce::Label reverbLabel;
    juce::Label reverbValueLabel;
    
    // Freeze toggle
    juce::ToggleButton freezeButton;

    // Glitch section
    RotaryKnob glitchWetSlider;
    juce::Label glitchWetLabel;
    juce::Label glitchWetValueLabel;

    RotaryKnob glitchRateSlider;
    juce::Label glitchRateLabel;
    juce::Label glitchRateValueLabel;

    RotaryKnob glitchResSlider;
    juce::Label glitchResLabel;
    juce::Label glitchResValueLabel;

    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> grainSizeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> densityAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sprayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dryWetAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> envelopeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> glitchAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> reverbAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> freezeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> glitchWetAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> glitchRateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> glitchResAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BotanicalHarmonicGranularAudioProcessorEditor)
};