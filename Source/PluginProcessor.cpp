/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
EQMasterAudioProcessor::EQMasterAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

EQMasterAudioProcessor::~EQMasterAudioProcessor()
{
}

//==============================================================================
const juce::String EQMasterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool EQMasterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool EQMasterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool EQMasterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double EQMasterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int EQMasterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int EQMasterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void EQMasterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String EQMasterAudioProcessor::getProgramName (int index)
{
    return {};
}

void EQMasterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void EQMasterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // pre playback init
    // pass process spec object to chains which pass it to each link in the chain

    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    spec.sampleRate = sampleRate;
    // pass to each chain
    leftChain.prepare(spec);
    rightChain.prepare(spec);

}

void EQMasterAudioProcessor::releaseResources()
{
    // free up resources post playback
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool EQMasterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void EQMasterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // setting up dsp
    juce::dsp::AudioBlock<float> block(buffer);
    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);

    // create processing context which wraps them
    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);

    // pass contexts to monochains
    leftChain.process(leftContext);
    rightChain.process(rightContext);
}

//==============================================================================
bool EQMasterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* EQMasterAudioProcessor::createEditor()
{
    // return new EQMasterAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void EQMasterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void EQMasterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

}

// writing function that we call from pluginprocessor.h
juce::AudioProcessorValueTreeState::ParameterLayout EQMasterAudioProcessor::createParameterLayout() {
    
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // add audio parameter floats; names, normalizable range and default value
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "LowCut Freq", 
        "LowCut Freq", 
        juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 1.f), 
        20.f)
    );

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "HighCut Freq",
        "HighCut Freq",
        juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 1.f),
        20000.f)
    );

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Peak Freq",
        "Peak Freq",
        juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 1.f),
        750.f)
    );

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Peak Gain",
        "Peak Gain",
        juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f),
        0.0f)
    );

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Peak Quality",
        "Peak Quality",
        juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f),
        1.f)
    );

    juce::StringArray stringArr;
    for (int i = 0; i < 4; ++i) {
        juce::String str;
        // create string which is multiple of twelve and describe it
        str << (12 + i * 12);
        str << " db/oct";
        // then add to string array
        stringArr.add(str);
    }

    // using choice object for slope instead of sliders
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "LowCut Slope",
        "LowCut Slope",
        stringArr,
        0)
    );

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "HighCut Slope",
        "HighCut Slope",
        stringArr,
        0)
    );

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EQMasterAudioProcessor();
}
