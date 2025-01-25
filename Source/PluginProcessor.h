/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

    Audio plugins depend on parameters to control the various parts of DSP.
    Juce coordinates the syncing of the parameters from GUI to DSP.

    This is the file where we want to set up variables and helper functions
    that act as the sort of backend for the project.


  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class EQMasterAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    EQMasterAudioProcessor();
    ~EQMasterAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

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

    // function to create apvts parameter layout
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // apvts: audio processor value tree state: data structure which stores and manages all parameters
    // such as volume, filter, cutoff etc.
    // acts as a 'central repository' for the current state of the project
    // instantiation takes in: audio processor, undo manager (null), value tree identifier, and parameter layout
    juce::AudioProcessorValueTreeState apvts { *this, nullptr, "Parameters", createParameterLayout() };

private:

    // central concept of dsp in juce: define a chain and then pass in processing context
    // which will run through each element of the chain automatically

    // creating type aliases to eliminate namespace and template definitions


    using Filter = juce::dsp::IIR::Filter<float>;
    // could do this for highpass, lowpass, peak, shelf, etc.
    using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;
    
    // creating two instances to do stereo processing
    using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter>;
    MonoChain leftChain, rightChain;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EQMasterAudioProcessor)
};
