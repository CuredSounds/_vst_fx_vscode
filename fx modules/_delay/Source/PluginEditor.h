#pragma once

#include "PluginProcessor.h"
#include <juce_audio_utils/juce_audio_utils.h>

class BCDelayAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    BCDelayAudioProcessorEditor (BCDelayAudioProcessor&);
    ~BCDelayAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    juce::AudioVisualiserComponent& getVisualiser() { return visualiser; }

private:
    BCDelayAudioProcessor& audioProcessor;

    juce::AudioVisualiserComponent visualiser;

    juce::Slider timeSlider;
    juce::Slider feedbackSlider;
    juce::Slider mixSlider;
    juce::Slider lpSlider;
    juce::Slider hpSlider;
    juce::Slider modRateSlider;
    juce::Slider modDepthSlider;
    juce::Slider saturationSlider;
    juce::ToggleButton pingPongButton;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> timeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> feedbackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lpAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> hpAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> modRateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> modDepthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> saturationAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> pingPongAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BCDelayAudioProcessorEditor)
};
