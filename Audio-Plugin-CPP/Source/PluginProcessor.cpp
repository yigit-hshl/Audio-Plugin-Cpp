/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginCPPAudioProcessor::AudioPluginCPPAudioProcessor()
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

AudioPluginCPPAudioProcessor::~AudioPluginCPPAudioProcessor()
{
}

//==============================================================================
const juce::String AudioPluginCPPAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioPluginCPPAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginCPPAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginCPPAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AudioPluginCPPAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioPluginCPPAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AudioPluginCPPAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPluginCPPAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String AudioPluginCPPAudioProcessor::getProgramName (int index)
{
    return {};
}

void AudioPluginCPPAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void AudioPluginCPPAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void AudioPluginCPPAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AudioPluginCPPAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginCPPAudioProcessor::createParameterLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    return layout;
}

void AudioPluginCPPAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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

    //[DONE]: add APVTS
    //TODO: create audio parameters for all dsp choices
    //TODO: update DSP here from audio parameters
    //TODO: save/load settings
    //TODO: save/load DSP order
    //TODO: Drag-To-Reorder GUI
    //TODO: GUI design for each DSP instance
    //TODO: metering
    //TODO: prepare all DSP
    //TODO: wet/dry knob
    //TODO: mono & stereo versions
    //TODO: modulators
    //TODO: thread-safe filter
    //TODO: pre/post filtering
    //TODO: delay module

    auto newDSPOrder = DSP_Order();

    // Try to pull
    while (dspOrderFifo.pull(newDSPOrder)) {

    }

    // If pulled; replace dspOrder
    if (newDSPOrder != DSP_Order())
        dspOrder = newDSPOrder;

    // Convert dspOrder into an array of pointers
    DSP_Pointers dspPointers;

    for (size_t i = 0; i < dspPointers.size(); ++i) {
        switch (dspOrder[i]) {
            case DSP_Option::Phase:
                dspPointers[i] = &phaser;
                break;
            case DSP_Option::Chorus;
                dspPointers[i] = &chorus;
                break;
            case DSP_Option::Overdrive:
                dspPointers[i] = &overdrive;
                break;
            case DSP_Option::LadderFilter;
                dspPointers[i] = &ladderFilter;
                break;
            case DSP_Option::END_OF_LIST;
                jassertfalse;
                break;
        }
    }

    // Process
    auto block = juce::dsp::AudioBlock<float>(buffer);
    auto context = juce::dsp::ProcessContextReplacing<float>(block);

    for (size_t i = 0; i < dspPointers.size(); ++i) {
        if (dspPointers[i] != nullptr) {
            dspPointers[i]->process(context);
        }
    }

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
    }
}

//==============================================================================
bool AudioPluginCPPAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioPluginCPPAudioProcessor::createEditor()
{
    return new AudioPluginCPPAudioProcessorEditor (*this);
}

//==============================================================================
void AudioPluginCPPAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void AudioPluginCPPAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginCPPAudioProcessor();
}
