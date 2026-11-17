#pragma once
#include "PluginProcessor.h"

class RiserLabEditor : public juce::AudioProcessorEditor
{
public:
    explicit RiserLabEditor (RiserLabProcessor&);
    ~RiserLabEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    RiserLabProcessor& proc;

    juce::Slider riseKnob, mixSlider;
    juce::ComboBox lengthBox;
    juce::ToggleButton tremButton { "Tremolo" };
    juce::Label riseLabel, mixLabel, lengthLabel;

    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<SA> riseAtt, mixAtt;
    std::unique_ptr<CA> lengthAtt;
    std::unique_ptr<BA> tremAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RiserLabEditor)
};
