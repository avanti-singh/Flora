#include "PluginEditor.h"
#include "BinaryData.h"

BentoBox::BentoBox(const juce::String& boxTitle) : title(boxTitle)
{
}

void BentoBox::paint(juce::Graphics& g)
{
    // Draw white background with rounded corners
    g.setColour(juce::Colours::white);
    g.fillRoundedRectangle(0.0f, 0.0f, (float)getWidth(), (float)getHeight(), 12.0f);
    
    // Draw border
    g.setColour(juce::Colours::lightgrey);
    g.drawRoundedRectangle(1.0f, 1.0f, (float)getWidth() - 2.0f, (float)getHeight() - 2.0f, 12.0f, 1.5f);
    
}

void BentoBox::resized()
{
}

void BentoBox::addAndMakeVisible(juce::Component& component)
{
    Component::addAndMakeVisible(component);
}

// BotanicalHarmonicGranularAudioProcessorEditor implementation
BotanicalHarmonicGranularAudioProcessorEditor::BotanicalHarmonicGranularAudioProcessorEditor(BotanicalHarmonicGranularAudioProcessor& p)
    : AudioProcessorEditor(&p), 
      processor(p),
      grainSection("GRAIN"),
      fxSection("FX")
{    
    // Set up bento boxes
    addAndMakeVisible(grainSection);
    addAndMakeVisible(fxSection);

    setLookAndFeel(&floralLookAndFeel);

    juce::Font labelFont {
        juce::FontOptions("VT323", "Bold", 16.0f)
    };

    juce::Font valueFont {
        juce::FontOptions("VT323", "Plain", 14.0f)
    };

    auto setLabelFontAndColour = [&](juce::Label& label, bool isValue = false)
    {
        label.setFont(isValue ? valueFont : labelFont);
        label.setColour(juce::Label::textColourId, isValue ? juce::Colour(255, 182, 193) : juce::Colours::black);
    };

    setLabelFontAndColour(grainSizeLabel);
    setLabelFontAndColour(grainSizeValueLabel, true);
    setLabelFontAndColour(densityLabel);
    setLabelFontAndColour(densityValueLabel, true);
    setLabelFontAndColour(sprayLabel);
    setLabelFontAndColour(sprayValueLabel, true);
    setLabelFontAndColour(dryWetLabel);
    setLabelFontAndColour(dryWetValueLabel, true);
    setLabelFontAndColour(directionLabel);
    setLabelFontAndColour(envelopeLabel);
    setLabelFontAndColour(delayLabel);
    setLabelFontAndColour(delayValueLabel, true);
    setLabelFontAndColour(glitchWetLabel);
    setLabelFontAndColour(glitchWetValueLabel, true);
    setLabelFontAndColour(glitchRateLabel);
    setLabelFontAndColour(glitchRateValueLabel, true);
    setLabelFontAndColour(glitchResLabel);
    setLabelFontAndColour(glitchResValueLabel, true);
    setLabelFontAndColour(reverbLabel);
    setLabelFontAndColour(reverbValueLabel, true);

    // Grain Size
    grainSizeSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    grainSizeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    grainSizeSlider.setRange(1.0f, 1000.0f, 1.0f);
    grainSizeSlider.setValue(100.0f);
    grainSection.addAndMakeVisible(grainSizeSlider);
    grainSizeLabel.setText("Size", juce::dontSendNotification);
    grainSizeLabel.setJustificationType(juce::Justification::centred);
    grainSection.addAndMakeVisible(grainSizeLabel);
    grainSizeValueLabel.setJustificationType(juce::Justification::centred);
    grainSizeValueLabel.setText("100", juce::dontSendNotification);
    grainSection.addAndMakeVisible(grainSizeValueLabel);

    // Density
    densitySlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    densitySlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    densitySlider.setRange(1.0f, 100.0f, 0.1f);
    densitySlider.setValue(50.0f);
    grainSection.addAndMakeVisible(densitySlider);
    densityLabel.setText("Density", juce::dontSendNotification);
    densityLabel.setJustificationType(juce::Justification::centred);
    grainSection.addAndMakeVisible(densityLabel);
    densityValueLabel.setJustificationType(juce::Justification::centred);
    densityValueLabel.setText("50", juce::dontSendNotification);
    grainSection.addAndMakeVisible(densityValueLabel);

    // Spray
    spraySlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    spraySlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    spraySlider.setRange(0.0f, 1.0f, 0.01f);
    spraySlider.setValue(0.0f);
    grainSection.addAndMakeVisible(spraySlider);
    sprayLabel.setText("Spray", juce::dontSendNotification);
    sprayLabel.setJustificationType(juce::Justification::centred);
    grainSection.addAndMakeVisible(sprayLabel);
    sprayValueLabel.setJustificationType(juce::Justification::centred);
    sprayValueLabel.setText("0", juce::dontSendNotification);
    grainSection.addAndMakeVisible(sprayValueLabel);

    // Dry/Wet
    dryWetSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    dryWetSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    dryWetSlider.setRange(0.0f, 1.0f, 0.01f);
    dryWetSlider.setValue(0.5f);
    grainSection.addAndMakeVisible(dryWetSlider);
    dryWetLabel.setText("Dry / Wet", juce::dontSendNotification);
    dryWetLabel.setJustificationType(juce::Justification::centred);
    grainSection.addAndMakeVisible(dryWetLabel);
    dryWetValueLabel.setJustificationType(juce::Justification::centred);
    dryWetValueLabel.setText("0.5", juce::dontSendNotification);
    grainSection.addAndMakeVisible(dryWetValueLabel);

    directionLabel.setText("Direction", juce::dontSendNotification);
    directionLabel.setJustificationType(juce::Justification::centred);
    grainSection.addAndMakeVisible(directionLabel);

    auto configureDirectionButton = [&](juce::TextButton& btn, const juce::String& name, int id, bool toggled) {
        btn.setButtonText(name);
        btn.setClickingTogglesState(true);
        btn.setRadioGroupId(id);
        btn.setToggleState(toggled, juce::dontSendNotification);
        btn.setColour(juce::TextButton::buttonOnColourId, juce::Colour(255, 182, 193));
        btn.setColour(juce::TextButton::buttonColourId, juce::Colours::white);
        btn.setColour(juce::TextButton::textColourOnId, juce::Colours::black);
        btn.setColour(juce::TextButton::textColourOffId, juce::Colours::black);
        grainSection.addAndMakeVisible(btn);
    };

    configureDirectionButton(forwardButton, "Forward", 1, true);
    configureDirectionButton(backwardButton, "Backward", 1, false);
    configureDirectionButton(bothButton, "Both", 1, false);

    grainSection.addAndMakeVisible(envelopeComboBox);
    envelopeComboBox.setJustificationType(juce::Justification::centred);
    envelopeComboBox.addItem("Hann", 1);
    envelopeComboBox.addItem("Hamming", 2);
    envelopeComboBox.addItem("Perc", 3);
    envelopeComboBox.addItem("Exp Decay", 4);
    envelopeComboBox.addItem("Rev Exp Decay", 5);
    envelopeComboBox.addItem("Cos", 6);
    envelopeLabel.setText("Envelope", juce::dontSendNotification);
    envelopeLabel.setJustificationType(juce::Justification::centred);
    grainSection.addAndMakeVisible(envelopeLabel);

    delaySlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    delaySlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    delaySlider.setRange(0.0f, 1.0f, 0.01f);
    delaySlider.setValue(0.5f);
    fxSection.addAndMakeVisible(delaySlider);
    delayLabel.setText("Delay", juce::dontSendNotification);
    delayLabel.setJustificationType(juce::Justification::centred);
    fxSection.addAndMakeVisible(delayLabel);
    delayValueLabel.setJustificationType(juce::Justification::centred);
    delayValueLabel.setText("0.5", juce::dontSendNotification);
    fxSection.addAndMakeVisible(delayValueLabel);

    reverbSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    reverbSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    reverbSlider.setRange(0.0f, 1.0f, 0.01f);
    reverbSlider.setValue(0.0f);
    fxSection.addAndMakeVisible(reverbSlider);
    reverbLabel.setText("Reverb", juce::dontSendNotification);
    reverbLabel.setJustificationType(juce::Justification::centred);
    fxSection.addAndMakeVisible(reverbLabel);
    reverbValueLabel.setJustificationType(juce::Justification::centred);
    reverbValueLabel.setText("0.0", juce::dontSendNotification);
    fxSection.addAndMakeVisible(reverbValueLabel);

    // Replace the existing freezeButton setup with:
    freezeButton.setButtonText("Freeze");
    freezeButton.setClickingTogglesState(true);
    freezeButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(255, 182, 193));
    freezeButton.setColour(juce::TextButton::buttonColourId, juce::Colours::white);
    freezeButton.setColour(juce::TextButton::textColourOnId, juce::Colours::black);
    freezeButton.setColour(juce::TextButton::textColourOffId, juce::Colours::black);
    freezeButton.setSize(80, 25); // Match direction buttons size
    fxSection.addAndMakeVisible(freezeButton);

    grainSizeSlider.setLookAndFeel(&customKnobLook);
    densitySlider.setLookAndFeel(&customKnobLook);
    spraySlider.setLookAndFeel(&customKnobLook);
    dryWetSlider.setLookAndFeel(&customKnobLook);
    delaySlider.setLookAndFeel(&customKnobLook);
    reverbSlider.setLookAndFeel(&customKnobLook);
    glitchWetSlider.setLookAndFeel(&customKnobLook);
    glitchRateSlider.setLookAndFeel(&customKnobLook);
    glitchResSlider.setLookAndFeel(&customKnobLook);

    // === Glitch Wet ===
    glitchWetSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    glitchWetSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    glitchWetSlider.setRange(0.0f, 1.0f, 0.01f);
    glitchWetSlider.setValue(0.0f);
    fxSection.addAndMakeVisible(glitchWetSlider);
    glitchWetLabel.setText("Glitch Amt", juce::dontSendNotification);
    glitchWetLabel.setJustificationType(juce::Justification::centred);
    fxSection.addAndMakeVisible(glitchWetLabel);
    glitchWetValueLabel.setJustificationType(juce::Justification::centred);
    fxSection.addAndMakeVisible(glitchWetValueLabel);

    // === Glitch Rate ===
    glitchRateSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    glitchRateSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    glitchRateSlider.setRange(0.0f, 1.0f, 0.01f);
    glitchRateSlider.setValue(0.5f); 
    fxSection.addAndMakeVisible(glitchRateSlider);
    glitchRateLabel.setText("Glitch Rate", juce::dontSendNotification);
    glitchRateLabel.setJustificationType(juce::Justification::centred);
    fxSection.addAndMakeVisible(glitchRateLabel);
    glitchRateValueLabel.setJustificationType(juce::Justification::centred);
    fxSection.addAndMakeVisible(glitchRateValueLabel);

    // === Glitch Resolution ===
    glitchResSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    glitchResSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    glitchResSlider.setRange(0.0f, 1.0f, 0.01f);
    glitchResSlider.setValue(0.5f);
    fxSection.addAndMakeVisible(glitchResSlider);
    glitchResLabel.setText("Glitch Res", juce::dontSendNotification);
    glitchResLabel.setJustificationType(juce::Justification::centred);
    fxSection.addAndMakeVisible(glitchResLabel);
    glitchResValueLabel.setJustificationType(juce::Justification::centred);
    fxSection.addAndMakeVisible(glitchResValueLabel);


    auto& tree = processor.getValueTreeState();
    grainSizeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(tree, "grain_size", grainSizeSlider);
    densityAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(tree, "grain_density", densitySlider);
    sprayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(tree, "spray", spraySlider);
    dryWetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(tree, "dry_wet", dryWetSlider);
    envelopeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(tree, "envelope_type", envelopeComboBox);
    delayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(tree, "delay_wet", delaySlider);
    freezeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(tree, "freeze", freezeButton);
    glitchWetAttachment  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(tree, "glitch_amt", glitchWetSlider);
    glitchRateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(tree, "glitch_rate", glitchRateSlider);
    glitchResAttachment  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(tree, "glitch_resolution", glitchResSlider);

    forwardButton.onClick = [this]() {
        if (forwardButton.getToggleState())
            processor.getValueTreeState().getParameterAsValue("grain_direction").setValue(0);
    };
    backwardButton.onClick = [this]() {
        if (backwardButton.getToggleState())
            processor.getValueTreeState().getParameterAsValue("grain_direction").setValue(1);
    };
    bothButton.onClick = [this]() {
        if (bothButton.getToggleState())
            processor.getValueTreeState().getParameterAsValue("grain_direction").setValue(2);
    };

    tree.addParameterListener("grain_direction", this);

    grainSizeSlider.onValueChange = [this]() {
        grainSizeValueLabel.setText(juce::String(grainSizeSlider.getValue(), 1), juce::dontSendNotification);
    };
    densitySlider.onValueChange = [this]() {
        densityValueLabel.setText(juce::String(densitySlider.getValue(), 1), juce::dontSendNotification);
    };
    spraySlider.onValueChange = [this]() {
        sprayValueLabel.setText(juce::String(spraySlider.getValue(), 2), juce::dontSendNotification);
    };
    dryWetSlider.onValueChange = [this]() {
        dryWetValueLabel.setText(juce::String(dryWetSlider.getValue(), 2), juce::dontSendNotification);
    };
    delaySlider.onValueChange = [this]() {
        delayValueLabel.setText(juce::String(delaySlider.getValue(), 2), juce::dontSendNotification);
    };
    reverbSlider.onValueChange = [this]() {
        reverbValueLabel.setText(juce::String(reverbSlider.getValue(), 2), juce::dontSendNotification);
    };
    glitchWetSlider.onValueChange = [this]() {
        glitchWetValueLabel.setText(juce::String(glitchWetSlider.getValue(), 2), juce::dontSendNotification);
    };
    glitchRateSlider.onValueChange = [this]() {
        glitchRateValueLabel.setText(juce::String(glitchRateSlider.getValue(), 2), juce::dontSendNotification);
    };
    glitchResSlider.onValueChange = [this]() {
        glitchResValueLabel.setText(juce::String(glitchResSlider.getValue(), 2), juce::dontSendNotification);
    };
    

    setSize(900, 600);
    backgroundImage = juce::ImageCache::getFromMemory(BinaryData::BG1_png, BinaryData::BG1_pngSize);

    envelopeComboBox.setSize(110, 25);

}

BotanicalHarmonicGranularAudioProcessorEditor::~BotanicalHarmonicGranularAudioProcessorEditor()
{
    auto& tree = processor.getValueTreeState();
    tree.removeParameterListener("grain_direction", this);
    setLookAndFeel(nullptr);
}

void BotanicalHarmonicGranularAudioProcessorEditor::paint(juce::Graphics& g)
{
    if (backgroundImage.isValid())
        g.drawImage(backgroundImage, getLocalBounds().toFloat());
    else
        g.fillAll(juce::Colour(240, 240, 240)); // fallback color
}

void BotanicalHarmonicGranularAudioProcessorEditor::resized()
{
    const int pluginWidth = 900;
    const int pluginHeight = 600;
    const int topMargin = 145;
    const int bentoWidth = 350;
    const int bentoHeight = 400;
    const int sideMargin = 87;
    const int spacingBetweenBentos = pluginWidth - (2 * sideMargin) - (2 * bentoWidth);

    const int knobSize = 85;            // 5px bigger than before (was 80)
    const int largeKnobSize = 105;      // FX knobs 5px bigger (was 100)
    const int labelHeight = 20;
    const int valueHeight = 20;
    const int spacing = 10;

    grainSection.setBounds(sideMargin, topMargin, bentoWidth, bentoHeight);
    fxSection.setBounds(grainSection.getRight() + spacingBetweenBentos + 5, topMargin, bentoWidth, bentoHeight);

    auto grainArea = grainSection.getLocalBounds().reduced(10, 30);
    int y = grainArea.getY();

    int midX = grainArea.getX() + grainArea.getWidth() / 2;
    int sprayX = midX + knobSize / 2 + spacing;
    int sizeX = midX - knobSize - spacing - knobSize / 2;
    int densityX = midX - knobSize / 2;

    grainSizeSlider.setBounds(sizeX, y, knobSize, knobSize);
    spraySlider.setBounds(sprayX, y, knobSize, knobSize);
    densitySlider.setBounds(densityX, y + 40, knobSize, knobSize); // 5px lower (was 35)

    grainSizeLabel.setBounds(grainSizeSlider.getX(), grainSizeSlider.getBottom(), knobSize, labelHeight);
    grainSizeValueLabel.setBounds(grainSizeSlider.getX(), grainSizeLabel.getBottom(), knobSize, valueHeight);
    sprayLabel.setBounds(spraySlider.getX(), spraySlider.getBottom(), knobSize, labelHeight);
    sprayValueLabel.setBounds(spraySlider.getX(), sprayLabel.getBottom(), knobSize, valueHeight);
    densityLabel.setBounds(densitySlider.getX(), densitySlider.getBottom(), knobSize, labelHeight);
    densityValueLabel.setBounds(densitySlider.getX(), densityLabel.getBottom(), knobSize, valueHeight);

    y = juce::jmax(sprayValueLabel.getBottom(), grainSizeValueLabel.getBottom(), densityValueLabel.getBottom()) + 5; // 5px more spacing

    int halfWidth = grainArea.getWidth() / 2;
    int leftX = grainArea.getX() + (halfWidth - 80) / 2;
    int rightX = grainArea.getX() + halfWidth + (halfWidth - 110) / 2;

    forwardButton.setBounds(leftX, y, 80, 25);
    backwardButton.setBounds(leftX, forwardButton.getBottom() + 5, 80, 25);
    bothButton.setBounds(leftX, backwardButton.getBottom() + 5, 80, 25);
    envelopeLabel.setBounds(rightX, y, 110, labelHeight);
    envelopeComboBox.setBounds(rightX, envelopeLabel.getBottom() + 5, 110, 25);

    y = juce::jmax(bothButton.getBottom(), envelopeComboBox.getBottom()) + 15; 

    // Add extra space before dry/wet section
    const int dryWetTopMargin = -35; 

    // Position dry/wet elements
    dryWetLabel.setBounds(grainArea.getX(), 
                        y + dryWetTopMargin,  
                        grainArea.getWidth(), 
                        labelHeight);

    dryWetSlider.setBounds(grainArea.getX() + (grainArea.getWidth() - knobSize) / 2, 
                        dryWetLabel.getBottom(), 
                        knobSize, 
                        knobSize);

    dryWetValueLabel.setBounds(dryWetSlider.getX(), 
                            dryWetSlider.getBottom(), 
                            knobSize, 
                            valueHeight);

    // ==== FX SECTION ====
    auto fxArea = fxSection.getLocalBounds().reduced(10, 30);
    y = fxArea.getY();

    // Glitch knobs at the top (3 in a row)
    const int glitchKnobWidth = (fxArea.getWidth() - 2 * spacing) / 3;
    const int glitchKnobSize = juce::jmin(glitchKnobWidth, largeKnobSize);

    // Glitch Amt
    glitchWetSlider.setBounds(fxArea.getX(), y, glitchKnobSize, glitchKnobSize);
    glitchWetLabel.setBounds(glitchWetSlider.getX(), glitchWetSlider.getBottom(), glitchKnobSize, labelHeight);
    glitchWetValueLabel.setBounds(glitchWetSlider.getX(), glitchWetLabel.getBottom(), glitchKnobSize, valueHeight);

    // Glitch Rate
    glitchRateSlider.setBounds(glitchWetSlider.getRight() + spacing, y, glitchKnobSize, glitchKnobSize);
    glitchRateLabel.setBounds(glitchRateSlider.getX(), glitchRateSlider.getBottom(), glitchKnobSize, labelHeight);
    glitchRateValueLabel.setBounds(glitchRateSlider.getX(), glitchRateLabel.getBottom(), glitchKnobSize, valueHeight);

    // Glitch Res
    glitchResSlider.setBounds(glitchRateSlider.getRight() + spacing, y, glitchKnobSize, glitchKnobSize);
    glitchResLabel.setBounds(glitchResSlider.getX(), glitchResSlider.getBottom(), glitchKnobSize, labelHeight);
    glitchResValueLabel.setBounds(glitchResSlider.getX(), glitchResLabel.getBottom(), glitchKnobSize, valueHeight);

    y = glitchResValueLabel.getBottom() + 20; // Space below glitch section

    // Delay and Reverb knobs side by side below
    const int effectKnobWidth = (fxArea.getWidth() - spacing) / 2;
    const int effectKnobSize = juce::jmin(effectKnobWidth, largeKnobSize);

    // Centered layout for Delay and Reverb
    int totalFXKnobWidth = (2 * effectKnobSize) + spacing;
    int fxKnobStartX = fxArea.getX() + (fxArea.getWidth() - totalFXKnobWidth) / 2;

    // Delay (centered left)
    delaySlider.setBounds(fxKnobStartX, y, effectKnobSize, effectKnobSize);
    delayLabel.setBounds(delaySlider.getX(), delaySlider.getBottom(), effectKnobSize, labelHeight);
    delayValueLabel.setBounds(delaySlider.getX(), delayLabel.getBottom(), effectKnobSize, valueHeight);

    // Reverb (centered right)
    reverbSlider.setBounds(delaySlider.getRight() + spacing, y, effectKnobSize, effectKnobSize);
    reverbLabel.setBounds(reverbSlider.getX(), reverbSlider.getBottom(), effectKnobSize, labelHeight);
    reverbValueLabel.setBounds(reverbSlider.getX(), reverbLabel.getBottom(), effectKnobSize, valueHeight);


    // Freeze button at the bottom
    freezeButton.setBounds(fxArea.getX() + (fxArea.getWidth() - 80) / 2, 
                         reverbValueLabel.getBottom() + 20, 
                         80, 25);
}

// Parameter listener to update button states
void BotanicalHarmonicGranularAudioProcessorEditor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == "grain_direction")
    {
        int direction = static_cast<int>(newValue);
        forwardButton.setToggleState(direction == 0, juce::dontSendNotification);
        backwardButton.setToggleState(direction == 1, juce::dontSendNotification);
        bothButton.setToggleState(direction == 2, juce::dontSendNotification);
    }
}