#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
 * BC Compressor - Editor/GUI
 *
 * Full-featured compressor interface with:
 * - All parameter controls (sliders + labels)
 * - Gain reduction meter
 * - Visual feedback
 * - Modular design (can be adapted for multi-FX host)
 */
class BCCompressorAudioProcessorEditor : public juce::AudioProcessorEditor,
                                         private juce::Timer
{
public:
    BCCompressorAudioProcessorEditor (BCCompressorAudioProcessor&);
    ~BCCompressorAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    //==============================================================================
    void timerCallback() override;

    //==============================================================================
    BCCompressorAudioProcessor& audioProcessor;

    // Sliders
    juce::Slider thresholdSlider;
    juce::Slider ratioSlider;
    juce::Slider attackSlider;
    juce::Slider releaseSlider;
    juce::Slider kneeSlider;
    juce::Slider makeupSlider;
    juce::Slider mixSlider;

    // Labels
    juce::Label thresholdLabel;
    juce::Label ratioLabel;
    juce::Label attackLabel;
    juce::Label releaseLabel;
    juce::Label kneeLabel;
    juce::Label makeupLabel;
    juce::Label mixLabel;

    // Buttons
    juce::ToggleButton sidechainButton;

    // Attachments (connect sliders to APVTS)
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ratioAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> kneeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> makeupAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> sidechainAttachment;

    // Metering
    float currentGainReduction = 0.0f;
    float inputLevel = -80.0f;
    float outputLevel = -80.0f;
    float peakInputLevel = -80.0f;
    float peakOutputLevel = -80.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BCCompressorAudioProcessorEditor)
};
