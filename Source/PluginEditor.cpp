#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BCCompressorAudioProcessorEditor::BCCompressorAudioProcessorEditor (BCCompressorAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Configure sliders
    auto setupSlider = [this] (juce::Slider& slider, juce::Label& label, const juce::String& labelText)
    {
        addAndMakeVisible (slider);
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 20);

        addAndMakeVisible (label);
        label.setText (labelText, juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        label.attachToComponent (&slider, false);
    };

    setupSlider (thresholdSlider, thresholdLabel, "Threshold");
    setupSlider (ratioSlider, ratioLabel, "Ratio");
    setupSlider (attackSlider, attackLabel, "Attack");
    setupSlider (releaseSlider, releaseLabel, "Release");
    setupSlider (kneeSlider, kneeLabel, "Knee");
    setupSlider (makeupSlider, makeupLabel, "Makeup");
    setupSlider (mixSlider, mixLabel, "Mix");

    // Configure sidechain button
    addAndMakeVisible (sidechainButton);
    sidechainButton.setButtonText ("Sidechain");

    // Create attachments (link sliders to APVTS parameters)
    thresholdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.apvts, "threshold", thresholdSlider);

    ratioAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.apvts, "ratio", ratioSlider);

    attackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.apvts, "attack", attackSlider);

    releaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.apvts, "release", releaseSlider);

    kneeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.apvts, "knee", kneeSlider);

    makeupAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.apvts, "makeup", makeupSlider);

    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.apvts, "mix", mixSlider);

    sidechainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        audioProcessor.apvts, "sidechain", sidechainButton);

    // Start timer for metering (30 Hz refresh)
    startTimerHz (30);

    setSize (700, 400);
}

BCCompressorAudioProcessorEditor::~BCCompressorAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void BCCompressorAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Background
    g.fillAll (juce::Colour (0xff2a2a2a));

    // Header
    g.setColour (juce::Colour (0xff3a3a3a));
    g.fillRect (0, 0, getWidth(), 50);

    g.setColour (juce::Colours::white);
    g.setFont (24.0f);
    g.drawText ("BC COMPRESSOR", 10, 10, getWidth() - 20, 30, juce::Justification::centred);

    // Gain Reduction Meter
    const int meterX = 20;
    const int meterY = getHeight() - 60;
    const int meterWidth = getWidth() - 40;
    const int meterHeight = 30;

    // Meter background
    g.setColour (juce::Colour (0xff1a1a1a));
    g.fillRoundedRectangle (meterX, meterY, meterWidth, meterHeight, 5.0f);

    // Meter value
    float grNormalized = juce::jmap (currentGainReduction, -20.0f, 0.0f, 0.0f, 1.0f);
    grNormalized = juce::jlimit (0.0f, 1.0f, grNormalized);

    g.setColour (juce::Colour (0xff00ff00).darker (0.3f));
    g.fillRoundedRectangle (meterX + 2, meterY + 2,
                            (meterWidth - 4) * grNormalized, meterHeight - 4, 3.0f);

    // Meter label
    g.setColour (juce::Colours::white);
    g.setFont (12.0f);
    g.drawText ("GAIN REDUCTION: " + juce::String (currentGainReduction, 1) + " dB",
                meterX, meterY, meterWidth, meterHeight,
                juce::Justification::centred);
}

void BCCompressorAudioProcessorEditor::resized()
{
    const int sliderSize = 80;
    const int sliderSpacing = 90;
    const int topMargin = 80;
    const int leftMargin = 30;

    // Top row: Threshold, Ratio, Attack, Release
    thresholdSlider.setBounds (leftMargin, topMargin, sliderSize, sliderSize);
    ratioSlider.setBounds (leftMargin + sliderSpacing, topMargin, sliderSize, sliderSize);
    attackSlider.setBounds (leftMargin + sliderSpacing * 2, topMargin, sliderSize, sliderSize);
    releaseSlider.setBounds (leftMargin + sliderSpacing * 3, topMargin, sliderSize, sliderSize);

    // Bottom row: Knee, Makeup, Mix
    const int bottomRowY = topMargin + 130;
    kneeSlider.setBounds (leftMargin, bottomRowY, sliderSize, sliderSize);
    makeupSlider.setBounds (leftMargin + sliderSpacing, bottomRowY, sliderSize, sliderSize);
    mixSlider.setBounds (leftMargin + sliderSpacing * 2, bottomRowY, sliderSize, sliderSize);

    // Sidechain button
    sidechainButton.setBounds (leftMargin + sliderSpacing * 3, bottomRowY + 30, 80, 30);
}

void BCCompressorAudioProcessorEditor::timerCallback()
{
    // Update gain reduction meter
    currentGainReduction = audioProcessor.getCompressorModule().getGainReductionDb();
    repaint();
}
