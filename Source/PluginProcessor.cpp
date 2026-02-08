#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BCCompressorAudioProcessor::BCCompressorAudioProcessor()
    : AudioProcessor (BusesProperties()
                      .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                      .withInput  ("Sidechain", juce::AudioChannelSet::stereo(), true)) // Sidechain bus
    , apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

BCCompressorAudioProcessor::~BCCompressorAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout BCCompressorAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Threshold: -60 to 0 dB
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "threshold",
        "Threshold",
        juce::NormalisableRange<float> (-60.0f, 0.0f, 0.1f),
        -20.0f,
        "dB"));

    // Ratio: 1:1 to 20:1
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "ratio",
        "Ratio",
        juce::NormalisableRange<float> (1.0f, 20.0f, 0.1f),
        4.0f));

    // Attack: 0.1 to 100 ms
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "attack",
        "Attack",
        juce::NormalisableRange<float> (0.1f, 100.0f, 0.1f, 0.3f), // Skewed for finer control at low values
        5.0f,
        "ms"));

    // Release: 10 to 1000 ms
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "release",
        "Release",
        juce::NormalisableRange<float> (10.0f, 1000.0f, 1.0f, 0.3f),
        100.0f,
        "ms"));

    // Knee: 0 to 12 dB (hard knee to soft knee)
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "knee",
        "Knee",
        juce::NormalisableRange<float> (0.0f, 12.0f, 0.1f),
        0.0f,
        "dB"));

    // Make-up Gain: -12 to +12 dB
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "makeup",
        "Makeup",
        juce::NormalisableRange<float> (-12.0f, 12.0f, 0.1f),
        0.0f,
        "dB"));

    // Mix (Parallel Compression): 0% to 100%
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "mix",
        "Mix",
        juce::NormalisableRange<float> (0.0f, 100.0f, 1.0f),
        100.0f,
        "%"));

    // Sidechain Enable
    layout.add (std::make_unique<juce::AudioParameterBool> (
        "sidechain",
        "Sidechain",
        false));

    return layout;
}

//==============================================================================
const juce::String BCCompressorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BCCompressorAudioProcessor::acceptsMidi() const
{
    return false;
}

bool BCCompressorAudioProcessor::producesMidi() const
{
    return false;
}

bool BCCompressorAudioProcessor::isMidiEffect() const
{
    return false;
}

double BCCompressorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BCCompressorAudioProcessor::getNumPrograms()
{
    return 1;
}

int BCCompressorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BCCompressorAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String BCCompressorAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void BCCompressorAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
CompressorModule::Parameters BCCompressorAudioProcessor::getCurrentParameters() const
{
    CompressorModule::Parameters params;

    params.thresholdDb = apvts.getRawParameterValue ("threshold")->load();
    params.ratio = apvts.getRawParameterValue ("ratio")->load();
    params.attackMs = apvts.getRawParameterValue ("attack")->load();
    params.releaseMs = apvts.getRawParameterValue ("release")->load();
    params.kneeDb = apvts.getRawParameterValue ("knee")->load();
    params.makeupDb = apvts.getRawParameterValue ("makeup")->load();
    params.mixPercent = apvts.getRawParameterValue ("mix")->load();
    params.useSidechain = apvts.getRawParameterValue ("sidechain")->load() > 0.5f;

    return params;
}

//==============================================================================
void BCCompressorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Prepare the modular DSP unit
    compressorModule.prepare (sampleRate, samplesPerBlock, getTotalNumInputChannels());
}

void BCCompressorAudioProcessor::releaseResources()
{
    compressorModule.reset();
}

bool BCCompressorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // Main input/output must match
    if (layouts.getMainInputChannelSet() != layouts.getMainOutputChannelSet())
        return false;

    // Support mono and stereo
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // Sidechain must be mono or stereo (if present)
    if (! layouts.getChannelSet (true, 1).isDisabled())
    {
        if (layouts.getChannelSet (true, 1) != juce::AudioChannelSet::mono()
         && layouts.getChannelSet (true, 1) != juce::AudioChannelSet::stereo())
            return false;
    }

    return true;
}

void BCCompressorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                                juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    juce::ScopedNoDenormals noDenormals;

    // Get current parameters from APVTS
    auto params = getCurrentParameters();

    // Get sidechain buffer if present
    juce::AudioBuffer<float>* sidechainBuffer = nullptr;

    if (getBusCount (true) > 1) // If sidechain bus exists
    {
        auto sidechainBus = getBusBuffer (buffer, true, 1);
        if (sidechainBus.getNumChannels() > 0)
            sidechainBuffer = &sidechainBus;
    }

    // Get main audio buffer
    auto mainBus = getBusBuffer (buffer, true, 0);

    // Process through modular compressor
    compressorModule.process (mainBus, sidechainBuffer, params);

    // Copy processed audio to output
    auto outputBus = getBusBuffer (buffer, false, 0);
    outputBus.makeCopyOf (mainBus, true);
}

//==============================================================================
bool BCCompressorAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* BCCompressorAudioProcessor::createEditor()
{
    return new BCCompressorAudioProcessorEditor (*this);
}

//==============================================================================
void BCCompressorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Save APVTS state
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void BCCompressorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Restore APVTS state
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BCCompressorAudioProcessor();
}
