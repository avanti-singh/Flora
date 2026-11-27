#include "RotaryKnob.h"

RotaryKnob::CustomLookAndFeel::CustomLookAndFeel()
{
    // Set up colors
    knobColor = juce::Colour(250, 250, 250);     // White
    accentColor = juce::Colour(255, 105, 180);   // Pink
    pointerColor = juce::Colour(80, 80, 80);     // Dark grey for pointer
}

void RotaryKnob::CustomLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                                    float sliderPos, const float rotaryStartAngle,
                                                    const float rotaryEndAngle, juce::Slider& slider)
{
    // Basic dimensions
    const float radius = juce::jmin(width, height) * 0.4f;
    const float centerX = x + width * 0.5f;
    const float centerY = y + height * 0.5f;
    const float rx = centerX - radius;
    const float ry = centerY - radius;
    const float rw = radius * 2.0f;
    
    // Fill the background with a subtle gradient
    g.setGradientFill(juce::ColourGradient(knobColor.darker(0.05f), centerX, centerY,
                                          knobColor, centerX + radius, centerY + radius, true));
    g.fillEllipse(rx, ry, rw, rw);
    
    // Draw a subtle shadow
    g.setColour(juce::Colours::black.withAlpha(0.1f));
    g.drawEllipse(rx, ry, rw, rw, 1.0f);
    
    // Draw the outer ring
    g.setColour(knobColor.darker(0.1f));
    g.drawEllipse(rx, ry, rw, rw, 1.5f);
    
    // Calculate angles for the progress arc
    const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    const float arcRadius = radius * 0.85f;
    
    // Draw the progress arc (pink accent)
    const float arcThickness = radius * 0.1f;
    juce::Path arcPath;
    arcPath.addCentredArc(centerX, centerY, arcRadius, arcRadius,
                         0.0f, rotaryStartAngle, angle, true);
    
    g.setColour(accentColor);
    g.strokePath(arcPath, juce::PathStrokeType(arcThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    
    // Draw the pointer
    const float pointerLength = radius * 0.7f;
    const float pointerThickness = radius * 0.1f;
    
    juce::Path pointerPath;
    pointerPath.startNewSubPath(centerX, centerY - pointerThickness * 0.5f);
    pointerPath.lineTo(centerX, centerY - pointerLength);
    pointerPath.addEllipse(centerX - pointerThickness * 0.5f, 
                          centerY - pointerLength - pointerThickness * 0.5f,
                          pointerThickness, pointerThickness);
    
    g.setColour(pointerColor);
    
    // Rotate the pointer to match the knob position
    juce::AffineTransform transform = juce::AffineTransform::translation(-centerX, -centerY)
                                    .rotated(angle)
                                    .translated(centerX, centerY);
    g.fillPath(pointerPath, transform);
    
    // Add a subtle highlight to give a 3D effect
    g.setColour(juce::Colours::white.withAlpha(0.4f));
    g.fillEllipse(centerX - radius * 0.2f, centerY - radius * 0.2f, radius * 0.4f, radius * 0.4f);
}

RotaryKnob::RotaryKnob()
{
    // Set up the slider
    setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    setRotaryParameters(juce::MathConstants<float>::pi * 1.2f, 
                        juce::MathConstants<float>::pi * 2.8f, 
                        true);
    setLookAndFeel(&customLookAndFeel);
    setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
}

RotaryKnob::~RotaryKnob()
{
    setLookAndFeel(nullptr);
}
