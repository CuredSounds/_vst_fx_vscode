#include "PluginProcessor.h"
#include "PluginEditor.h"

BCDelayAudioProcessor::BCDelayAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#else
     :
#endif
       apvts (*this, nullptr, "Parameters", createParameterLayout())
{
}

BCDelayAudioProcessor::~BCDelayAudioProcessor()
{
}

const juce::String BCDelayAudioProcessor::getName() const
{
    return "_BC_delay";
}

bool BCDelayAudioProcessor::acceptsMidi() const
{
   return false;
}

bool BCDelayAudioProcessor::producesMidi() const
{
   return false;
}

bool BCDelayAudioProcessor::isMidiEffect() const
{
   return false;
}

double BCDelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BCDelayAudioProcessor::getNumPrograms()
{
    return 1;
}

int BCDelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BCDelayAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String BCDelayAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void BCDelayAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

void BCDelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    lastSampleRate = sampleRate;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 1;

    for (int i = 0; i < 2; ++i)
    {
        delayLine[i].prepare (spec);
        delayLine[i].setMaximumDelayInSamples (static_cast<int>(2.0 * sampleRate) + 2); // 2s max + small buffer for interpolation
        
        lfo[i].prepare (spec);
        lfo[i].initialise ([](float x) { return std::sin (x); });

        lpFilter[i].prepare (spec);
        lpFilter[i].setType (juce::dsp::StateVariableTPTFilterType::lowpass);
        
        hpFilter[i].prepare (spec);
        hpFilter[i].setType (juce::dsp::StateVariableTPTFilterType::highpass);
    }

    timeSmoother.reset (sampleRate, 0.05);
}

void BCDelayAudioProcessor::releaseResources()
{
}

bool BCDelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void BCDelayAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    const int bufferLength = buffer.getNumSamples();

    auto time = apvts.getRawParameterValue ("TIME")->load();
    auto feedback = apvts.getRawParameterValue ("FEEDBACK")->load();
    auto mix = apvts.getRawParameterValue ("MIX")->load();
    auto lpFreq = apvts.getRawParameterValue ("LOWPASS")->load();
    auto hpFreq = apvts.getRawParameterValue ("HIGHPASS")->load();
    auto pingPong = apvts.getRawParameterValue ("PINGPONG")->load() > 0.5f;
    
    auto modRate = apvts.getRawParameterValue ("MODRATE")->load();
    auto modDepth = apvts.getRawParameterValue ("MODDEPTH")->load(); // in seconds
    auto saturation = apvts.getRawParameterValue ("SATURATION")->load();

    timeSmoother.setTargetValue (time);

    for (int i = 0; i < 2; ++i)
    {
        lpFilter[i].setCutoffFrequency (lpFreq);
        hpFilter[i].setCutoffFrequency (hpFreq);
        lfo[i].setFrequency (modRate);
    }

    for (int sample = 0; sample < bufferLength; ++sample)
    {
        float currentTime = timeSmoother.getNextValue();
        float samplesToDelay[2] = { 0.0f, 0.0f };
        
        int numChannelsToProcess = std::min (totalNumInputChannels, 2);

        for (int channel = 0; channel < numChannelsToProcess; ++channel)
        {
            float inputSample = buffer.getSample (channel, sample);
            
            // Calculate modulated delay time
            float lfoValue = lfo[channel].processSample (0.0f);
            float modulatedDelayTime = currentTime + (lfoValue * modDepth);
            
            // Ensure delay time is within bounds
            modulatedDelayTime = std::max (0.0f, std::min (2.0f, modulatedDelayTime));
            
            float delayedSample = delayLine[channel].popSample (0, modulatedDelayTime * static_cast<float>(lastSampleRate));
            
            // Apply filtering
            float processedSample = lpFilter[channel].processSample (0, hpFilter[channel].processSample (0, delayedSample));
            
            // Apply Saturation (Analog Character)
            processedSample = std::tanh (processedSample * saturation);
            
            float outSample = (inputSample * (1.0f - mix)) + (processedSample * mix);
            buffer.setSample (channel, sample, outSample);
            
            samplesToDelay[channel] = inputSample + (processedSample * feedback);
        }
        
        // Write to delay lines (with Ping-Pong logic)
        for (int channel = 0; channel < numChannelsToProcess; ++channel)
        {
            int writeChannel = (pingPong && totalNumOutputChannels > 1) ? 1 - channel : channel;
            if (writeChannel >= 2) writeChannel = channel;
            
            delayLine[writeChannel].pushSample (0, samplesToDelay[channel]);
        }
    }

    if (auto* editor = dynamic_cast<BCDelayAudioProcessorEditor*> (getActiveEditor()))
    {
        editor->getVisualiser().pushBuffer (buffer);
    }
}

bool BCDelayAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* BCDelayAudioProcessor::createEditor()
{
    return new BCDelayAudioProcessorEditor (*this);
}

void BCDelayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void BCDelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout BCDelayAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID {"TIME", 1}, "Time", 0.0f, 2.0f, 0.5f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID {"FEEDBACK", 1}, "Feedback", 0.0f, 0.95f, 0.5f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID {"MIX", 1}, "Mix", 0.0f, 1.0f, 0.5f));
    
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID {"LOWPASS", 1}, "Low Pass", 
                                                             juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f), 20000.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID {"HIGHPASS", 1}, "High Pass", 
                                                             juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f), 20.0f));
    layout.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID {"PINGPONG", 1}, "Ping Pong", false));

    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID {"MODRATE", 1}, "Mod Rate", 0.1f, 10.0f, 1.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID {"MODDEPTH", 1}, "Mod Depth", 0.0f, 0.01f, 0.002f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID {"SATURATION", 1}, "Saturation", 0.0f, 2.0f, 1.0f));

    return layout;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BCDelayAudioProcessor();
}
