#include "PluginProcessor.h"
#include "PluginEditor.h"

BCDelayAudioProcessorEditor::BCDelayAudioProcessorEditor (BCDelayAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), visualiser (2)
{
    setSize (600, 550);

    visualiser.setSamplesPerBlock (256);
    visualiser.setBufferSize (1024);
    visualiser.setColours (juce::Colours::black, juce::Colours::white.withAlpha (0.5f));
    addAndMakeVisible (visualiser);

    auto setupSlider = [this](juce::Slider& slider, std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attachment, const juce::String& paramID)
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 90, 0);
        addAndMakeVisible (slider);
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.getAPVTS(), paramID, slider);
    };

    setupSlider (timeSlider, timeAttachment, "TIME");
    setupSlider (feedbackSlider, feedbackAttachment, "FEEDBACK");
    setupSlider (mixSlider, mixAttachment, "MIX");
    setupSlider (lpSlider, lpAttachment, "LOWPASS");
    setupSlider (hpSlider, hpAttachment, "HIGHPASS");
    setupSlider (modRateSlider, modRateAttachment, "MODRATE");
    setupSlider (modDepthSlider, modDepthAttachment, "MODDEPTH");
    setupSlider (saturationSlider, saturationAttachment, "SATURATION");

    pingPongButton.setButtonText ("Ping Pong");
    addAndMakeVisible (pingPongButton);
    pingPongAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (audioProcessor.getAPVTS(), "PINGPONG", pingPongButton);
}

BCDelayAudioProcessorEditor::~BCDelayAudioProcessorEditor()
{
}

void BCDelayAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    auto area = getLocalBounds();
    auto titleArea = area.removeFromTop (40);
    g.setFont (30.0f);
    g.drawText ("BC Delay", titleArea, juce::Justification::centred, true);

    area.removeFromBottom (100); // Leave space for visualiser

    g.setFont (12.0f);
    auto rowHeight = area.getHeight() / 3;
    auto row1 = area.removeFromTop (rowHeight);
    auto row2 = area.removeFromTop (rowHeight);
    auto row3 = area;

    auto drawLabel = [&](juce::Rectangle<int> r, juce::String text)
    {
        g.drawText (text, r.withY (r.getBottom() - 15).withHeight (15), juce::Justification::centred, true);
    };

    auto cellWidth = row1.getWidth() / 3;
    
    drawLabel (row1.removeFromLeft (cellWidth), "Time");
    drawLabel (row1.removeFromLeft (cellWidth), "Feedback");
    drawLabel (row1, "Mix");

    drawLabel (row2.removeFromLeft (cellWidth), "Low Pass");
    drawLabel (row2.removeFromLeft (cellWidth), "High Pass");
    drawLabel (row2, "Saturation");

    drawLabel (row3.removeFromLeft (cellWidth), "Mod Rate");
    drawLabel (row3.removeFromLeft (cellWidth), "Mod Depth");
}

void BCDelayAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    auto visualiserArea = area.removeFromBottom (100);
    visualiser.setBounds (visualiserArea.reduced (10, 5));

    area.removeFromTop (40); // Title

    auto rowHeight = area.getHeight() / 3;
    auto row1 = area.removeFromTop (rowHeight);
    auto row2 = area.removeFromTop (rowHeight);
    auto row3 = area;

    auto cellWidth = row1.getWidth() / 3;

    timeSlider.setBounds (row1.removeFromLeft (cellWidth).reduced (10, 20));
    feedbackSlider.setBounds (row1.removeFromLeft (cellWidth).reduced (10, 20));
    mixSlider.setBounds (row1.reduced (10, 20));

    lpSlider.setBounds (row2.removeFromLeft (cellWidth).reduced (10, 20));
    hpSlider.setBounds (row2.removeFromLeft (cellWidth).reduced (10, 20));
    saturationSlider.setBounds (row2.reduced (10, 20));

    modRateSlider.setBounds (row3.removeFromLeft (cellWidth).reduced (10, 20));
    modDepthSlider.setBounds (row3.removeFromLeft (cellWidth).reduced (10, 20));
    pingPongButton.setBounds (row3.reduced (20, 40));
}
