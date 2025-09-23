/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <Fifo.h>

//==============================================================================
/**
*/
class AudioPluginCPPAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    AudioPluginCPPAudioProcessor();
    ~AudioPluginCPPAudioProcessor() override;

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

    enum class DSP_Option {
        Phase,
        Chorus,
        Overdrive,
        LadderFilter,
        END_OF_LIST
    };
    
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts{ *this, nullptr, "Settings", createParameterLayout() };

    using DSP_Order = std::array<DSP_Option, static_cast<site_t>(DSP_Option::END_OF_LIST)>;
    SimpleMBComp::Fifo<DSP_Order> dspOrderFifo;

    /*
    Phaser:
        Rate: Hz
        Depth: 0 to 1
        Center freq.: Hz
        Feedback: -1 to 1
        Mix: 0 to 1
    */
    juce::AudioParameterFloat* phaserRateHz = nullptr;
    juce::AudioParameterFloat* phaserCenterFreqHz = nullptr;
    juce::AudioParameterFloat* phaserDepthPercent = nullptr;
    juce::AudioParameterFloat* phaserFeedbackPercent = nullptr;
    juce::AudioParameterFloat* phaserMixPercent = nullptr;

    /*
    Chorus:
        Rate: Hz
        Depth: 0 to 1
        Center delay: ms (1 to 100)
        Feedback: -1 to 1
        Mix: 0 to 1
    */
    juce::AudioParameterFloat* chorusRateHz = nullptr;
    juce::AudioParameterFloat* chorusDepthPercent = nullptr;
    juce::AudioParameterFloat* chorusCenterDelayMs = nullptr;
    juce::AudioParameterFloat* chorusFeedbackPercent = nullptr;
    juce::AudioParameterFloat* chorusMixPercent = nullptr;

    // Drive: 1-100
    juce::AudioParameterFloat* overdriveSaturation = nullptr;

private:
    DSP_Order dspOrder;

    template<typename DSP>
    struct DSP_Choice : juce::dsp::ProcessorBase {
        void prepare(const juce::dsp::ProcessSpec& spec) override {
            dsp.prepare(spec);
        }
        void process(const juce::dsp::ProcessContextReplacing<float>& context) override {
            dsp.process(context);
        }
        void reset() override {
            dsp.reset();
        }

        DSP dsp;
    };

    DSP_Choice<juce::dsp::DelayLine<float>> delay;
    DSP_Choice<juce::dsp::Phaser<float>> phaser;
    DSP_Choice<juce::dsp::Chorus<float>> chorus;
    DSP_Choice<juce::dsp::LadderFilter<float>> overdrive, ladderFilter;

    using DSP_Pointers = std::array<juce::dsp::ProcessorBase*, static_cast<site_t>(DSP_Option::END_OF_LIST)>;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginCPPAudioProcessor)
};
