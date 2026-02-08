#pragma once

#include <JuceHeader.h>
#istnclude "DSP/CompressorModule.h"

//==============================================================================
/**
 * BC Compressor - Audio Processor (VST3 Wrapper)
 *
 * This wraps the modular CompressorModule in a VST3/AU plugin format.
 * The DSP logic lives in CompressorModule, making it reusable in:
 * - Standalone compressor plugin (this project)
 * - Multi-FX host plugin (future)
 * - Other plugin chains
 *
 * This class only handles:
 * - APVTS parameter management
 * - DAW integration (state, buses, editor)
 * - Parameter -> DSP module bridging
 */
class BCCompressorAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    BCCompressorAudioProcessor();
    ~BCCompressorAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    // Parameter Value Tree State - manages all plugin parameters
    juce::AudioProcessorValueTreeState apvts;

    // Access to DSP module (for metering, etc.)
    CompressorModule& getCompressorModule() { return compressorModule; }

private:
    //==============================================================================
    // Creates the parameter layout for APVTS
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Reads current parameters from APVTS and returns struct
    CompressorModule::Parameters getCurrentParameters() const;

    //==============================================================================
    // The actual DSP module (decoupled from VST3 wrapper)
    CompressorModule compressorModule;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BCCompressorAudioProcessor)
};
