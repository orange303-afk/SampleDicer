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
  void clearSample(int slot);
  juce::File sampleFile(int) const;
  float playheadPosition(int) const;
  bool renderCurrentSoundToFile(const juce::File&, bool useAiff, juce::String& errorMessage);
  uint32_t glitchVisualGeneration() const noexcept { return glitchTriggerGeneration.load(std::memory_order_acquire); }
  void dice(bool files=true,bool values=true); bool diceSlot(int slot); juce::AudioProcessorValueTreeState state;
  void back();
  bool canGoBack() const noexcept { return hasPreviousGeneration; }
private:
  struct Voice { double position=-1; int delay=0; float velocity=1; float gainDb=0; float pitch=0; float start=0; float shift=0; float fade=1; float startFadeLengthMs=10; float fadeLengthMs=10; float startFadeCurve=0; float fadeCurve=0; uint32_t generation=0; bool playing=false; };
  struct Slot { juce::AudioBuffer<float> audio; juce::File file; double sourceRate=44100; std::array<Voice,16> voices; int nextVoice=0; int displayVoice=-1; std::atomic<float> displayPosition{-1.f}; std::atomic<bool> sampleReady{false}; std::atomic<uint32_t> sampleGeneration{0}; };
  struct Generation { std::array<juce::File,4> files; std::array<std::array<float,4>,4> values{}; };
  struct RtParameters { std::atomic<float>* gain{}; std::atomic<float>* pitch{}; std::atomic<float>* start{}; std::atomic<float>* shift{}; std::atomic<float>* fade{}; std::atomic<float>* startFadeLength{}; std::atomic<float>* fadeLength{}; std::atomic<float>* startFadeCurve{}; std::atomic<float>* fadeCurve{}; std::atomic<float>* locked{}; };
  static juce::AudioProcessorValueTreeState::ParameterLayout layout();
  juce::String id(int,const juce::String&) const; void triggerAll(float velocity,int midiNote); void renderVoices(juce::AudioBuffer<float>&);
  void startGlitchEvent() noexcept;
  void processGlitch(juce::AudioBuffer<float>&) noexcept;
  void captureGeneration();
  float audioRandom01() noexcept;
  std::array<Slot,4> slots; juce::AudioFormatManager formats;
  std::array<RtParameters,4> rtParameters;
  std::atomic<float>* rtVoices{}; std::atomic<float>* rtPte{}; std::atomic<float>* rtKey{}; std::atomic<float>* rtRoundRobin{}; std::atomic<float>* rtBurst{}; std::atomic<float>* rtBurstRate{}; std::atomic<float>* rtGlitchMode{};
  std::atomic<float>* rtMasterGain{};
  std::atomic<float>* rtRandomVolume{}; std::atomic<float>* rtRandomPitch{}; std::atomic<float>* rtRandomStart{}; std::atomic<float>* rtRandomShift{};
  juce::Random uiRandom; uint32_t audioRngState=0x9e3779b9u; double hostRate=44100; double burstCountdown=0; int heldNotes=0; int lastTriggeredNote=60; int lastRoundRobinSlot=-1; juce::CriticalSection lock;
  static constexpr int glitchDelayCapacity = 65536;
  static constexpr int glitchStutterCapacity = 8192;
  std::array<std::array<float, glitchDelayCapacity>, 2> glitchDelayBuffer {};
  std::array<std::array<float, glitchStutterCapacity>, 2> glitchStutterBuffer {};
  std::array<float, 2> glitchHeldSample {};
  int glitchWritePosition=0, glitchDelaySamples=1, glitchSamplesRemaining=0;
  int glitchStutterLength=1, glitchStutterPosition=0, glitchStutterRepeats=0;
  int glitchHoldLength=1, glitchHoldCounter=0;
  bool glitchCapturing=false;
  float glitchWet=0.35f, glitchFeedback=0.25f, glitchCrackleChance=0.001f;
  std::atomic<uint32_t> glitchTriggerGeneration { 0 };
  Generation previousGeneration; bool hasPreviousGeneration=false;
  std::unique_ptr<juce::PropertiesFile> settings;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleDicerAudioProcessor)
};
