#include "PluginEditor.h"

RiserLabEditor::RiserLabEditor (RiserLabProcessor& p)
    : AudioProcessorEditor (&p), proc (p)
{
    setSize (420, 540);

    const auto purple = juce::Colour (0xff8b6bff);

    riseKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    riseKnob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 22);
    riseKnob.setColour (juce::Slider::rotarySliderFillColourId, purple);
    riseKnob.setColour (juce::Slider::thumbColourId, juce::Colours::white);
    addAndMakeVisible (riseKnob);

    mixSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    mixSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 60, 22);
    mixSlider.setColour (juce::Slider::trackColourId, juce::Colour (0xffff8a2b));
    addAndMakeVisible (mixSlider);

    lengthBox.addItemList (juce::StringArray { "1/16", "1/8", "1/4", "1/2", "1 bar", "2 bars" }, 1);
    addAndMakeVisible (lengthBox);
    addAndMakeVisible (tremButton);

    for (auto* l : { &riseLabel, &mixLabel, &lengthLabel })
    {
        l->setJustificationType (juce::Justification::centredLeft);
        l->setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.8f));
        addAndMakeVisible (l);
    }
    riseLabel.setText ("RISE", juce::dontSendNotification);
    riseLabel.setJustificationType (juce::Justification::centred);
    mixLabel.setText ("MIX", juce::dontSendNotification);
    lengthLabel.setText ("REVERSE LENGTH", juce::dontSendNotification);

    riseAtt   = std::make_unique<SA> (proc.apvts, "rise", riseKnob);
    mixAtt    = std::make_unique<SA> (proc.apvts, "mix", mixSlider);
    lengthAtt = std::make_unique<CA> (proc.apvts, "length", lengthBox);
    tremAtt   = std::make_unique<BA> (proc.apvts, "trem", tremButton);
}

void RiserLabEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff121014));
    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (22.0f, juce::Font::bold));
    g.drawText ("RISER LAB", 20, 14, getWidth() - 40, 30, juce::Justification::centred);
}

void RiserLabEditor::resized()
{
    auto r = getLocalBounds().reduced (20);
    r.removeFromTop (40);

    riseLabel.setBounds (r.removeFromTop (20));
    riseKnob.setBounds (r.removeFromTop (280));
    r.removeFromTop (10);

    lengthLabel.setBounds (r.removeFromTop (20));
    auto row = r.removeFromTop (30);
    lengthBox.setBounds (row.removeFromLeft (150));
    tremButton.setBounds (row.removeFromRight (120));
    r.removeFromTop (14);

    mixLabel.setBounds (r.removeFromTop (20));
    mixSlider.setBounds (r.removeFromTop (30));
}
