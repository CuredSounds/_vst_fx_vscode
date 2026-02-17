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

    //==============================================================================
    // Meter Panel (bottom section)
    const int meterY = getHeight() - 90;
    const int meterHeight = 80;
    
    // Panel background
    g.setColour (juce::Colour (0xff1a1a1a));
    g.fillRoundedRectangle (0, meterY, getWidth(), meterHeight, 8.0f);
    
    // Border
    g.setColour (juce::Colour (0xff444444));
    g.drawRoundedRectangle (0, meterY, getWidth(), meterHeight, 8.0f, 2.0f);

    //==============================================================================
    // Input Level Meter
    const int meterX = 20;
    const int meterW = 120;
    const int col2X = meterX + meterW + 40;
    const int col3X = col2X + meterW + 40;

    // Label
    g.setColour (juce::Colours::white);
    g.setFont (11.0f);
    g.drawText ("INPUT", meterX, meterY + 8, meterW, 15, juce::Justification::centred);

    // Meter background
    g.setColour (juce::Colour (0xff0a0a0a));
    g.fillRoundedRectangle (meterX, meterY + 25, meterW, 35, 3.0f);

    // Input level bar (dB range: -60 to 0)
    float inputNorm = juce::jmap (inputLevel, -60.0f, 0.0f, 0.0f, 1.0f);
    inputNorm = juce::jlimit (0.0f, 1.0f, inputNorm);
    juce::Colour inputColour = juce::Colour (0xff00aa00); // Green
    if (inputLevel > -12.0f) inputColour = juce::Colour (0xffff8800); // Orange warning
    if (inputLevel > -3.0f) inputColour = juce::Colour (0xffff0000);  // Red clip

    g.setColour (inputColour);
    g.fillRoundedRectangle (meterX + 2, meterY + 27, (meterW - 4) * inputNorm, 31, 2.0f);

    // Value display
    g.setColour (juce::Colours::white);
    g.setFont (10.0f);
    g.drawText (juce::String (inputLevel, 1) + " dB", meterX, meterY + 40, meterW, 12, 
                juce::Justification::centred);

    //==============================================================================
    // Gain Reduction Meter
    g.setColour (juce::Colours::white);
    g.setFont (11.0f);
    g.drawText ("GAIN REDUCTION", col2X, meterY + 8, meterW, 15, juce::Justification::centred);

    g.setColour (juce::Colour (0xff0a0a0a));
    g.fillRoundedRectangle (col2X, meterY + 25, meterW, 35, 3.0f);

    // GR bar (inverted: more negative = more reduction)
    float grNormalized = juce::jmap (currentGainReduction, -20.0f, 0.0f, 0.0f, 1.0f);
    grNormalized = juce::jlimit (0.0f, 1.0f, grNormalized);

    g.setColour (juce::Colour (0xff00ff00).darker (0.3f));
    g.fillRoundedRectangle (col2X + 2, meterY + 27, (meterW - 4) * grNormalized, 31, 2.0f);

    g.setColour (juce::Colours::white);
    g.setFont (10.0f);
    g.drawText (juce::String (currentGainReduction, 1) + " dB", col2X, meterY + 40, meterW, 12, 
                juce::Justification::centred);

    //==============================================================================
    // Output Level Meter
    g.setColour (juce::Colours::white);
    g.setFont (11.0f);
    g.drawText ("OUTPUT", col3X, meterY + 8, meterW, 15, juce::Justification::centred);

    g.setColour (juce::Colour (0xff0a0a0a));
    g.fillRoundedRectangle (col3X, meterY + 25, meterW, 35, 3.0f);

    // Output level bar
    float outputNorm = juce::jmap (outputLevel, -60.0f, 0.0f, 0.0f, 1.0f);
    outputNorm = juce::jlimit (0.0f, 1.0f, outputNorm);
    juce::Colour outputColour = juce::Colour (0xff00aa00); // Green
    if (outputLevel > -12.0f) outputColour = juce::Colour (0xffff8800); // Orange warning
    if (outputLevel > -3.0f) outputColour = juce::Colour (0xffff0000);  // Red clip

    g.setColour (outputColour);
    g.fillRoundedRectangle (col3X + 2, meterY + 27, (meterW - 4) * outputNorm, 31, 2.0f);

    // Value display
    g.setColour (juce::Colours::white);
    g.setFont (10.0f);
    g.drawText (juce::String (outputLevel, 1) + " dB", col3X, meterY + 40, meterW, 12, 
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
    // Update metering data from the compressor module
    const auto meterData = audioProcessor.getCompressorModule().getMeterData();
    currentGainReduction = meterData.gainReduction;
    inputLevel = meterData.inputLevel;
    outputLevel = meterData.outputLevel;
    
    // Smooth the levels with a simple decay for visual stability
    const float smoothing = 0.7f; // exponential smoothing factor
    static float smoothedInputLevel = -80.0f;
    static float smoothedOutputLevel = -80.0f;
    
    smoothedInputLevel = smoothing * smoothedInputLevel + (1.0f - smoothing) * inputLevel;
    smoothedOutputLevel = smoothing * smoothedOutputLevel + (1.0f - smoothing) * outputLevel;
    
    inputLevel = smoothedInputLevel;
    outputLevel = smoothedOutputLevel;
    
    repaint();
}
