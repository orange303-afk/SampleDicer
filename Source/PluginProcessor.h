#pragma once
#include <JuceHeader.h>

class SampleDicerAudioProcessor : public juce::AudioProcessor {
public:
  SampleDicerAudioProcessor();
  ~SampleDicerAudioProcessor() override;
  void prepareToPlay(double,int) override; void releaseResources() override {}
  bool isBusesLayoutSupported(const BusesLayout&) const override;
  void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
  juce::AudioProcessorEditor* createEditor() override; bool hasEditor() const override{return true;}
  const juce::String getName() const override{return JucePlugin_Name;} bool acceptsMidi() const override{return true;}
  bool producesMidi() const override{return false;} bool isMidiEffect() const override{return false;}
  double getTailLengthSeconds() const override{return 10;} int getNumPrograms() override{return 1;}
  int getCurrentProgram() override{return 0;} void setCurrentProgram(int) override{}
  const juce::String getProgramName(int) override{return{};} void changeProgramName(int,const juce::String&) override{}
  void getStateInformation(juce::MemoryBlock&) override; void setStateInformation(const void*,int) override;
  bool loadSample(int,const juce::File&); bool browse(int,int); juce::String sampleName(int) const;
  juce::File sampleFile(int) const;
  void dice(bool files=true,bool values=true); juce::AudioProcessorValueTreeState state;
private:
  struct Voice { double position=-1; int delay=0; float velocity=1; bool playing=false; };
  struct Slot { juce::AudioBuffer<float> audio; juce::File file; double sourceRate=44100; std::array<Voice,16> voices; int nextVoice=0; };
  static juce::AudioProcessorValueTreeState::ParameterLayout layout();
  juce::String id(int,const juce::String&) const; void triggerAll(float velocity); void renderVoices(juce::AudioBuffer<float>&);
  std::array<Slot,4> slots; juce::AudioFormatManager formats;
  juce::Random random; double hostRate=44100; double burstCountdown=0; int heldNotes=0; juce::CriticalSection lock;
  std::unique_ptr<juce::PropertiesFile> settings;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleDicerAudioProcessor)
};
