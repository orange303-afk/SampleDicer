#include "PluginProcessor.h"
#include "PluginEditor.h"

SampleDicerAudioProcessor::SampleDicerAudioProcessor():AudioProcessor(BusesProperties().withOutput("Output",juce::AudioChannelSet::stereo(),true)),state(*this,nullptr,"STATE",layout()){formats.registerBasicFormats();for(int s=0;s<4;++s){auto&rt=rtParameters[(size_t)s];rt.gain=state.getRawParameterValue(id(s,"gain"));rt.pitch=state.getRawParameterValue(id(s,"pitch"));rt.start=state.getRawParameterValue(id(s,"start"));rt.shift=state.getRawParameterValue(id(s,"shift"));rt.fade=state.getRawParameterValue(id(s,"fade"));rt.startFadeLength=state.getRawParameterValue(id(s,"startFadeLength"));rt.fadeLength=state.getRawParameterValue(id(s,"fadeLength"));rt.startFadeCurve=state.getRawParameterValue(id(s,"startFadeCurve"));rt.fadeCurve=state.getRawParameterValue(id(s,"fadeCurve"));rt.locked=state.getRawParameterValue(id(s,"lock"));}rtVoices=state.getRawParameterValue("global.voices");rtPte=state.getRawParameterValue("global.pte");rtKey=state.getRawParameterValue("global.key");rtRoundRobin=state.getRawParameterValue("global.roundRobin");rtBurst=state.getRawParameterValue("global.burst");rtBurstRate=state.getRawParameterValue("global.burstRate");rtGlitchMode=state.getRawParameterValue("global.glitchMode");rtMasterGain=state.getRawParameterValue("global.masterGain");rtRandomVolume=state.getRawParameterValue("random.volume");rtRandomPitch=state.getRawParameterValue("random.pitch");rtRandomStart=state.getRawParameterValue("random.start");rtRandomShift=state.getRawParameterValue("random.shift");for(int action=0;action<7;++action)state.addParameterListener(actionParameterId(action),this);juce::PropertiesFile::Options options;options.applicationName="Sample Dicer";options.filenameSuffix="settings";options.folderName="Ilya Orange/Sample Dicer";options.osxLibrarySubFolder="Application Support";settings=std::make_unique<juce::PropertiesFile>(options);}
SampleDicerAudioProcessor::~SampleDicerAudioProcessor(){cancelPendingUpdate();for(int action=0;action<7;++action)state.removeParameterListener(actionParameterId(action),this);if(settings!=nullptr)settings->saveIfNeeded();}
juce::AudioProcessorValueTreeState::ParameterLayout SampleDicerAudioProcessor::layout(){
  std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
  for(int s=1;s<=4;++s){auto x="slot"+juce::String(s)+".";
    p.push_back(std::make_unique<juce::AudioParameterFloat>(x+"gain","Volume",-60.f,6.f,0.f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>(x+"pitch","Pitch",-24.f,24.f,0.f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>(x+"start","Start",0.f,.99f,0.f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>(x+"shift","Shift",0.f,250.f,0.f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>(x+"fade","Fade End",.01f,1.f,1.f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>(x+"startFadeLength","Start Fade Length",1.f,250.f,10.f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>(x+"fadeLength","Fade Length",1.f,250.f,10.f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>(x+"startFadeCurve","Start Fade Curve",-1.f,1.f,0.f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>(x+"fadeCurve","Fade Curve",-1.f,1.f,0.f));
    p.push_back(std::make_unique<juce::AudioParameterBool>(x+"lock","Lock Slot",false));}
  for(auto n:{"volume","pitch","start","shift"})p.push_back(std::make_unique<juce::AudioParameterFloat>("random."+juce::String(n),"Random "+juce::String(n),0.f,1.f,.15f));
  p.push_back(std::make_unique<juce::AudioParameterInt>("global.voices","Maximum Voices",1,16,8));
  p.push_back(std::make_unique<juce::AudioParameterBool>("global.burst","Burst",false));
  p.push_back(std::make_unique<juce::AudioParameterFloat>("global.burstRate","Burst Rate",juce::NormalisableRange<float>(1.f,30.f,.1f,0.45f),8.f));
  p.push_back(std::make_unique<juce::AudioParameterBool>("global.pte","Per-trigger Randomization",false));
  p.push_back(std::make_unique<juce::AudioParameterBool>("global.key","Chromatic Keytrack",false));
  p.push_back(std::make_unique<juce::AudioParameterBool>("global.roundRobin","Random Round Robin",false));
  p.push_back(std::make_unique<juce::AudioParameterBool>("global.glitchMode","Glitch Mode",false));
  p.push_back(std::make_unique<juce::AudioParameterFloat>("global.masterGain","Master Volume",-60.f,6.f,0.f));
  p.push_back(std::make_unique<juce::AudioParameterBool>("action.dice","DICE Trigger",false));
  p.push_back(std::make_unique<juce::AudioParameterBool>("action.samples","Randomize Samples Trigger",false));
  p.push_back(std::make_unique<juce::AudioParameterBool>("action.params","Randomize Parameters Trigger",false));
  for(int slot=1;slot<=4;++slot)p.push_back(std::make_unique<juce::AudioParameterBool>("action.slot"+juce::String(slot),"Randomize Slot "+juce::String(slot)+" Sample Trigger",false));
  return{p.begin(),p.end()};}
const char* SampleDicerAudioProcessor::actionParameterId(int index) noexcept{static constexpr const char* ids[]{"action.dice","action.samples","action.params","action.slot1","action.slot2","action.slot3","action.slot4"};return juce::isPositiveAndBelow(index,7)?ids[index]:ids[0];}
void SampleDicerAudioProcessor::prepareToPlay(double r,int){hostRate=r;transportStateKnown=false;stopAllVoices();}
void SampleDicerAudioProcessor::releaseResources(){stopAllVoices();transportStateKnown=false;}
bool SampleDicerAudioProcessor::isBusesLayoutSupported(const BusesLayout& l)const{return l.getMainOutputChannelSet()==juce::AudioChannelSet::mono()||l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo();}
juce::String SampleDicerAudioProcessor::id(int s,const juce::String& n)const{return"slot"+juce::String(s+1)+"."+n;}
bool SampleDicerAudioProcessor::loadSample(int s,const juce::File& f){
  if(!juce::isPositiveAndBelow(s,4)||!f.existsAsFile())return false; std::unique_ptr<juce::AudioFormatReader> r(formats.createReaderFor(f)); if(!r)return false;
  juce::AudioBuffer<float> next(juce::jmin(2,(int)r->numChannels),(int)r->lengthInSamples); r->read(&next,0,next.getNumSamples(),0,true,true);
  slots[(size_t)s].sampleReady.store(false,std::memory_order_release);const juce::ScopedLock guard(lock);slots[(size_t)s].audio=std::move(next);slots[(size_t)s].sourceRate=r->sampleRate;slots[(size_t)s].file=f;slots[(size_t)s].sampleGeneration.fetch_add(1,std::memory_order_release);slots[(size_t)s].displayVoice=-1;slots[(size_t)s].displayPosition.store(-1.f);slots[(size_t)s].sampleReady.store(true,std::memory_order_release);state.state.setProperty("file"+juce::String(s+1),f.getFullPathName(),nullptr);state.state.setProperty("lastFolder",f.getParentDirectory().getFullPathName(),nullptr);if(settings!=nullptr){settings->setValue("lastFolder",f.getParentDirectory().getFullPathName());settings->saveIfNeeded();}return true;}
void SampleDicerAudioProcessor::clearSample(int s){if(!juce::isPositiveAndBelow(s,4))return;auto&slot=slots[(size_t)s];slot.sampleReady.store(false,std::memory_order_release);const juce::ScopedLock guard(lock);slot.audio.setSize(0,0);slot.file=juce::File{};slot.sampleGeneration.fetch_add(1,std::memory_order_release);slot.displayVoice=-1;slot.displayPosition.store(-1.f);state.state.setProperty("file"+juce::String(s+1),juce::String{},nullptr);}
bool SampleDicerAudioProcessor::browse(int s,int direction){if(!juce::isPositiveAndBelow(s,4)||!slots[s].file.existsAsFile())return false;juce::Array<juce::File> fs;
  for(auto e:juce::RangedDirectoryIterator(slots[s].file.getParentDirectory(),false,"*.wav;*.aif;*.aiff;*.flac;*.mp3",juce::File::findFiles))fs.add(e.getFile());
  fs.sort();if(fs.isEmpty())return false;auto i=fs.indexOf(slots[s].file);i=(juce::jmax(0,i)+(direction<0?-1:1)+fs.size())%fs.size();return loadSample(s,fs[i]);}
juce::String SampleDicerAudioProcessor::sampleName(int s)const{return juce::isPositiveAndBelow(s,4)&&slots[s].file.existsAsFile()?slots[s].file.getFileName():"Drop a sample";}
juce::File SampleDicerAudioProcessor::sampleFile(int s)const{return juce::isPositiveAndBelow(s,4)?slots[s].file:juce::File{};}
float SampleDicerAudioProcessor::playheadPosition(int s)const{return juce::isPositiveAndBelow(s,4)?slots[(size_t)s].displayPosition.load():-1.f;}
bool SampleDicerAudioProcessor::renderCurrentSoundToFile(const juce::File& destination, bool useAiff,
                                                          juce::String& errorMessage)
{
  struct RenderSlot
  {
    juce::AudioBuffer<float> audio;
    double sourceRate=44100.0;
    float gainDb=0.0f,pitch=0.0f,start=0.0f,shift=0.0f,fade=1.0f;
    float startFadeLengthMs=10.0f,fadeLengthMs=10.0f,startFadeCurve=0.0f,fadeCurve=0.0f;
  };
  std::array<RenderSlot,4> renderSlots;
  {
    const juce::ScopedLock guard(lock);
    for(size_t i=0;i<renderSlots.size();++i)
    {
      if(!slots[i].sampleReady.load(std::memory_order_acquire)||slots[i].audio.getNumSamples()<2)continue;
      renderSlots[i].audio=slots[i].audio;
      renderSlots[i].sourceRate=slots[i].sourceRate;
    }
  }
  bool hasAudio=false;
  int outputSamples=0;
  const auto outputRate=hostRate>0.0?hostRate:44100.0;
  for(size_t i=0;i<renderSlots.size();++i)
  {
    auto& s=renderSlots[i];
    if(s.audio.getNumSamples()<2)continue;
    hasAudio=true;
    const auto& rt=rtParameters[i];
    s.gainDb=rt.gain->load();s.pitch=rt.pitch->load();s.start=rt.start->load();s.shift=rt.shift->load();
    s.fade=rt.fade->load();s.startFadeLengthMs=rt.startFadeLength->load();s.fadeLengthMs=rt.fadeLength->load();
    s.startFadeCurve=rt.startFadeCurve->load();s.fadeCurve=rt.fadeCurve->load();
    s.start=juce::jmin(s.start,s.fade);
    const auto step=s.sourceRate/outputRate*std::pow(2.0,(double)s.pitch/12.0);
    const auto sourceLength=juce::jmax(0.0,(double)(s.fade-s.start)*s.audio.getNumSamples());
    outputSamples=juce::jmax(outputSamples,juce::roundToInt(s.shift*.001*outputRate)+
                                          (int)std::ceil(sourceLength/juce::jmax(step,0.000001)));
  }
  if(!hasAudio||outputSamples<=0){errorMessage="Load at least one sample before exporting.";return false;}
  juce::AudioBuffer<float> output(2,outputSamples);output.clear();
  for(const auto& s:renderSlots)
  {
    if(s.audio.getNumSamples()<2)continue;
    const auto step=s.sourceRate/outputRate*std::pow(2.0,(double)s.pitch/12.0);
    const auto startPosition=(double)s.start*s.audio.getNumSamples();
    const auto fadeEnd=(double)s.fade*s.audio.getNumSamples();
    const auto availableFade=juce::jmax(1.0,(double)(s.fade-s.start)*s.audio.getNumSamples());
    const auto startFadeSamples=juce::jmin(availableFade,juce::jmax(1.0,s.sourceRate*s.startFadeLengthMs*.001));
    const auto fadeSamples=juce::jmin(availableFade,juce::jmax(1.0,s.sourceRate*s.fadeLengthMs*.001));
    const auto startCurveExponent=std::pow(4.0,(double)s.startFadeCurve);
    const auto fadeCurveExponent=std::pow(4.0,(double)s.fadeCurve);
    const auto gain=juce::Decibels::decibelsToGain(s.gainDb);
    const auto delay=juce::roundToInt(s.shift*.001*outputRate);
    auto position=startPosition;
    for(int i=delay;i<outputSamples&&position<fadeEnd&&position<s.audio.getNumSamples()-1;++i,position+=step)
    {
      const auto index=(int)position;const auto fraction=(float)(position-index);
      const auto fadeInGain=std::pow(juce::jlimit(0.0,1.0,(position-startPosition)/startFadeSamples),startCurveExponent);
      const auto fadeOutGain=std::pow(juce::jlimit(0.0,1.0,(fadeEnd-position)/fadeSamples),fadeCurveExponent);
      const auto envelope=(float)(fadeInGain*fadeOutGain)*gain;
      for(int channel=0;channel<2;++channel)
      {
        const auto* source=s.audio.getReadPointer(juce::jmin(channel,s.audio.getNumChannels()-1));
        output.addSample(channel,i,juce::jmap(fraction,source[index],source[index+1])*envelope);
      }
    }
  }
  output.applyGain(juce::Decibels::decibelsToGain(rtMasterGain->load()));
  if(destination.existsAsFile()&&!destination.deleteFile()){errorMessage="The existing export file cannot be replaced.";return false;}
  std::unique_ptr<juce::OutputStream> stream=destination.createOutputStream();
  if(stream==nullptr){errorMessage="The export file cannot be created in the selected folder.";return false;}
  std::unique_ptr<juce::AudioFormat> format=useAiff
      ?std::unique_ptr<juce::AudioFormat>(new juce::AiffAudioFormat())
      :std::unique_ptr<juce::AudioFormat>(new juce::WavAudioFormat());
  const auto options=juce::AudioFormatWriterOptions{}.withSampleRate(outputRate).withNumChannels(2).withBitsPerSample(24);
  auto writer=format->createWriterFor(stream,options);
  if(writer==nullptr){errorMessage="The audio writer could not be created.";destination.deleteFile();return false;}
  if(!writer->writeFromAudioSampleBuffer(output,0,output.getNumSamples()))
  {errorMessage="Writing the audio file failed.";writer.reset();destination.deleteFile();return false;}
  return true;
}
float SampleDicerAudioProcessor::audioRandom01() noexcept{audioRngState^=audioRngState<<13;audioRngState^=audioRngState>>17;audioRngState^=audioRngState<<5;return(float)(audioRngState&0x00ffffffu)/16777215.f;}
void SampleDicerAudioProcessor::startGlitchEvent() noexcept
{
  glitchDelaySamples=juce::jlimit(1,glitchDelayCapacity-1,juce::roundToInt(hostRate*(0.012+audioRandom01()*0.17)));
  glitchWet=0.18f+audioRandom01()*0.55f;glitchFeedback=0.15f+audioRandom01()*0.63f;
  glitchCrackleChance=0.00025f+audioRandom01()*0.0025f;
  glitchStutterLength=juce::jlimit(32,glitchStutterCapacity,juce::roundToInt(hostRate*(0.008+audioRandom01()*0.055)));
  glitchStutterPosition=0;glitchStutterRepeats=1+juce::roundToInt(audioRandom01()*5.0f);glitchCapturing=true;
  glitchHoldLength=1+juce::roundToInt(audioRandom01()*11.0f);glitchHoldCounter=0;
  glitchSamplesRemaining=juce::roundToInt(hostRate*(0.45+audioRandom01()*1.35));
  glitchTriggerGeneration.fetch_add(1,std::memory_order_release);
}
void SampleDicerAudioProcessor::processGlitch(juce::AudioBuffer<float>& out) noexcept
{
  if(glitchSamplesRemaining<=0)return;const auto channels=juce::jmin(2,out.getNumChannels());
  for(int i=0;i<out.getNumSamples();++i){if(glitchSamplesRemaining--<=0)break;
    const auto readPosition=(glitchWritePosition-glitchDelaySamples+glitchDelayCapacity)%glitchDelayCapacity;
    std::array<float,2> effected{};
    for(int c=0;c<channels;++c){const auto dry=out.getSample(c,i);const auto delayed=glitchDelayBuffer[(size_t)c][(size_t)readPosition];glitchDelayBuffer[(size_t)c][(size_t)glitchWritePosition]=dry+delayed*glitchFeedback;effected[(size_t)c]=dry+(delayed-dry)*glitchWet;}
    if(glitchCapturing){for(int c=0;c<channels;++c)glitchStutterBuffer[(size_t)c][(size_t)glitchStutterPosition]=effected[(size_t)c];if(++glitchStutterPosition>=glitchStutterLength){glitchStutterPosition=0;glitchCapturing=false;}}
    else if(glitchStutterRepeats>0){for(int c=0;c<channels;++c)effected[(size_t)c]=0.18f*effected[(size_t)c]+0.82f*glitchStutterBuffer[(size_t)c][(size_t)glitchStutterPosition];if(++glitchStutterPosition>=glitchStutterLength){glitchStutterPosition=0;--glitchStutterRepeats;}}
    if(glitchHoldCounter<=0){glitchHeldSample=effected;glitchHoldCounter=glitchHoldLength;}else{effected=glitchHeldSample;--glitchHoldCounter;}
    if(audioRandom01()<glitchCrackleChance){const auto crackle=(audioRandom01()*2.0f-1.0f)*(0.12f+audioRandom01()*0.38f);for(int c=0;c<channels;++c)effected[(size_t)c]+=crackle;}
    for(int c=0;c<channels;++c)out.setSample(c,i,std::tanh(effected[(size_t)c]*1.15f));
    glitchWritePosition=(glitchWritePosition+1)%glitchDelayCapacity;
  }
}
void SampleDicerAudioProcessor::triggerAll(float velocity, int midiNote)
{
  const auto maximum=juce::roundToInt(rtVoices->load());const auto pteEnabled=rtPte->load()>.5f;const auto keyEnabled=rtKey->load()>.5f;const auto roundRobinEnabled=rtRoundRobin->load()>.5f;
  auto selectedSlot=-1;if(roundRobinEnabled){std::array<int,4> ready{};auto readyCount=0;for(size_t i=0;i<slots.size();++i)if(slots[i].sampleReady.load(std::memory_order_acquire))ready[(size_t)readyCount++]=(int)i;if(readyCount>1){auto filteredCount=0;for(int i=0;i<readyCount;++i)if(ready[(size_t)i]!=lastRoundRobinSlot)ready[(size_t)filteredCount++]=ready[(size_t)i];if(filteredCount>0)readyCount=filteredCount;}if(readyCount>0){const auto choice=juce::jmin(readyCount-1,(int)(audioRandom01()*(float)readyCount));selectedSlot=ready[(size_t)choice];lastRoundRobinSlot=selectedSlot;}}else lastRoundRobinSlot=-1;
  for(size_t slotIndex=0;slotIndex<slots.size();++slotIndex){if(roundRobinEnabled&&(int)slotIndex!=selectedSlot)continue;auto&s=slots[slotIndex];if(!s.sampleReady.load(std::memory_order_acquire))continue;auto chosen=-1;for(int i=0;i<maximum;++i)if(!s.voices[(size_t)i].playing){chosen=i;break;}if(chosen<0){chosen=s.nextVoice%maximum;s.nextVoice=(s.nextVoice+1)%maximum;}auto&v=s.voices[(size_t)chosen];const auto&rt=rtParameters[slotIndex];v.gainDb=rt.gain->load();v.pitch=rt.pitch->load();v.start=rt.start->load();v.shift=rt.shift->load();v.fade=rt.fade->load();v.startFadeLengthMs=rt.startFadeLength->load();v.fadeLengthMs=rt.fadeLength->load();v.startFadeCurve=rt.startFadeCurve->load();v.fadeCurve=rt.fadeCurve->load();if(pteEnabled&&rt.locked->load()<=.5f){auto vary=[this](float value,float minimum,float upper,float amount){auto normalized=(value-minimum)/(upper-minimum);normalized=juce::jlimit(0.f,1.f,normalized+(audioRandom01()*2.f-1.f)*amount);return minimum+normalized*(upper-minimum);};v.gainDb=vary(v.gainDb,-60.f,6.f,rtRandomVolume->load());v.pitch=vary(v.pitch,-24.f,24.f,rtRandomPitch->load());v.start=vary(v.start,0.f,.99f,rtRandomStart->load());v.shift=vary(v.shift,0.f,250.f,rtRandomShift->load());}v.start=juce::jmin(v.start,v.fade);if(keyEnabled)v.pitch+=(float)(midiNote-60);v.position=-1;v.delay=0;v.velocity=velocity;v.generation=s.sampleGeneration.load(std::memory_order_acquire);v.playing=true;s.displayVoice=chosen;s.displayPosition.store(v.start);}
}
void SampleDicerAudioProcessor::renderVoices(juce::AudioBuffer<float>&out)
{
  for(int k=0;k<4;++k){auto&s=slots[(size_t)k];const auto generation=s.sampleGeneration.load(std::memory_order_acquire);for(size_t voiceIndex=0;voiceIndex<s.voices.size();++voiceIndex){auto&v=s.voices[voiceIndex];if(!v.playing)continue;if(v.generation!=generation){v.playing=false;continue;}auto step=s.sourceRate/hostRate*std::pow(2.,v.pitch/12.);auto gain=juce::Decibels::decibelsToGain(v.gainDb);auto startPosition=v.start*s.audio.getNumSamples();auto fadeEnd=v.fade*s.audio.getNumSamples();auto availableFade=juce::jmax(1.0,(double)(v.fade-v.start)*s.audio.getNumSamples());auto startFadeSamples=juce::jmin(availableFade,juce::jmax(1.0,s.sourceRate*v.startFadeLengthMs*.001));auto fadeSamples=juce::jmin(availableFade,juce::jmax(1.0,s.sourceRate*v.fadeLengthMs*.001));auto startCurveExponent=std::pow(4.0,(double)v.startFadeCurve);auto fadeCurveExponent=std::pow(4.0,(double)v.fadeCurve);if(v.position<0){v.position=startPosition;v.delay=juce::roundToInt(v.shift*.001*hostRate);}for(int i=0;i<out.getNumSamples();++i){if(v.delay>0){--v.delay;continue;}auto pos=(int)v.position;if(pos>=s.audio.getNumSamples()-1||v.position>=fadeEnd){v.playing=false;if((int)voiceIndex==s.displayVoice)s.displayPosition.store(-1.f);break;}auto frac=(float)(v.position-pos);auto fadeInGain=std::pow(juce::jlimit(0.0,1.0,(v.position-startPosition)/startFadeSamples),startCurveExponent);auto fadeOutGain=std::pow(juce::jlimit(0.0,1.0,(fadeEnd-v.position)/fadeSamples),fadeCurveExponent);auto envelopeGain=(float)(fadeInGain*fadeOutGain);for(int c=0;c<out.getNumChannels();++c){auto*d=s.audio.getReadPointer(juce::jmin(c,s.audio.getNumChannels()-1));out.addSample(c,i,juce::jmap(frac,d[pos],d[pos+1])*gain*v.velocity*envelopeGain);}v.position+=step;}if((int)voiceIndex==s.displayVoice&&v.playing)s.displayPosition.store((float)(v.position/s.audio.getNumSamples()));}}
}
void SampleDicerAudioProcessor::stopAllVoices() noexcept
{
  heldNotes=0;burstCountdown=0;lastRoundRobinSlot=-1;
  for(auto& slot:slots){for(auto& voice:slot.voices){voice.playing=false;voice.position=-1;voice.delay=0;}slot.displayVoice=-1;slot.displayPosition.store(-1.f,std::memory_order_release);}
  glitchSamplesRemaining=0;glitchCapturing=false;glitchStutterRepeats=0;glitchStutterPosition=0;glitchHoldCounter=0;glitchWritePosition=0;glitchHeldSample.fill(0.0f);
  for(auto& channel:glitchDelayBuffer)channel.fill(0.0f);for(auto& channel:glitchStutterBuffer)channel.fill(0.0f);
}
void SampleDicerAudioProcessor::requestRandomizeAction(RandomizeAction action)
{
  const auto index=static_cast<int>(action);if(!juce::isPositiveAndBelow(index,7))return;
  if(auto* parameter=state.getParameter(actionParameterId(index))){parameter->beginChangeGesture();parameter->setValueNotifyingHost(1.0f);parameter->endChangeGesture();}
}
void SampleDicerAudioProcessor::parameterChanged(const juce::String& parameterId,float newValue)
{
  if(newValue<=0.5f)return;
  for(int action=0;action<7;++action)if(parameterId==actionParameterId(action)){pendingRandomizeActions.fetch_or(1u<<(uint32_t)action,std::memory_order_release);triggerAsyncUpdate();break;}
}
void SampleDicerAudioProcessor::handleAsyncUpdate()
{
  const auto actions=pendingRandomizeActions.exchange(0,std::memory_order_acq_rel);
  for(int action=0;action<7;++action)if((actions&(1u<<(uint32_t)action))!=0)if(auto* parameter=state.getParameter(actionParameterId(action)))parameter->setValueNotifyingHost(0.0f);
  if((actions&1u)!=0)dice(true,true);
  if((actions&2u)!=0)dice(true,false);
  if((actions&4u)!=0)dice(false,true);
  for(int slot=0;slot<4;++slot)if((actions&(1u<<(uint32_t)(slot+3)))!=0)diceSlot(slot);
}
void SampleDicerAudioProcessor::processBlock(juce::AudioBuffer<float>& out,juce::MidiBuffer& midi){juce::ScopedNoDenormals n;out.clear();
  if(auto* playHead=getPlayHead())if(auto position=playHead->getPosition()){const auto playing=position->getIsPlaying();if(transportStateKnown&&playing!=lastTransportPlaying)stopAllVoices();lastTransportPlaying=playing;transportStateKnown=true;}
  bool started=false;
  for(auto m:midi){auto message=m.getMessage();if(message.isNoteOn()){++heldNotes;lastTriggeredNote=message.getNoteNumber();if(rtGlitchMode->load()>.5f)startGlitchEvent();triggerAll(message.getFloatVelocity(),lastTriggeredNote);started=true;}else if(message.isNoteOff())heldNotes=juce::jmax(0,heldNotes-1);else if(message.isAllNotesOff()||message.isAllSoundOff())heldNotes=0;}
  const auto burst=rtBurst->load()>.5f;const auto interval=hostRate/rtBurstRate->load();if(!burst||heldNotes==0)burstCountdown=0;else{if(started||burstCountdown<=0)burstCountdown=interval;burstCountdown-=out.getNumSamples();while(burstCountdown<=0){triggerAll(1.f,lastTriggeredNote);burstCountdown+=interval;}}
  {const juce::ScopedTryLock guard(lock);if(guard.isLocked())renderVoices(out);}if(rtGlitchMode->load()>.5f)processGlitch(out);else{glitchSamplesRemaining=0;glitchCapturing=false;glitchStutterRepeats=0;}out.applyGain(juce::Decibels::decibelsToGain(rtMasterGain->load()));}
void SampleDicerAudioProcessor::captureGeneration(){for(int s=0;s<4;++s){previousGeneration.files[(size_t)s]=slots[(size_t)s].file;int index=0;for(auto name:{"gain","pitch","start","shift"})previousGeneration.values[(size_t)s][(size_t)index++]=state.getRawParameterValue(id(s,name))->load();}hasPreviousGeneration=true;}
void SampleDicerAudioProcessor::dice(bool files,bool values){captureGeneration();for(int s=0;s<4;++s){if(state.getRawParameterValue(id(s,"lock"))->load()>.5f)continue;if(files){if(slots[(size_t)s].file.existsAsFile()){for(int n=1+uiRandom.nextInt(12);n>0;--n)browse(s,1);}else{auto path=settings!=nullptr?settings->getValue("lastFolder"):juce::String{};if(path.isEmpty())path=state.state.getProperty("lastFolder").toString();juce::Array<juce::File> candidates;auto folder=juce::File(path);if(folder.isDirectory())for(auto e:juce::RangedDirectoryIterator(folder,false,"*.wav;*.aif;*.aiff;*.flac;*.mp3",juce::File::findFiles))candidates.add(e.getFile());if(!candidates.isEmpty())loadSample(s,candidates[uiRandom.nextInt(candidates.size())]);}}if(values)for(auto name:{"gain","pitch","start","shift"}){auto* p=state.getParameter(id(s,name));auto amount=state.getRawParameterValue("random."+juce::String(juce::String(name)=="gain"?"volume":name))->load();p->setValueNotifyingHost(juce::jlimit(0.f,1.f,p->getValue()+(uiRandom.nextFloat()*2-1)*amount));}}}
bool SampleDicerAudioProcessor::diceSlot(int s)
{
  if(!juce::isPositiveAndBelow(s,4))return false;const auto current=slots[(size_t)s].file;auto folder=current.existsAsFile()?current.getParentDirectory():juce::File(settings!=nullptr?settings->getValue("lastFolder"):state.state.getProperty("lastFolder").toString());if(!folder.isDirectory())return false;juce::Array<juce::File> candidates;for(auto e:juce::RangedDirectoryIterator(folder,false,"*.wav;*.aif;*.aiff;*.flac;*.mp3",juce::File::findFiles))candidates.add(e.getFile());if(candidates.isEmpty())return false;if(candidates.size()>1){const auto currentIndex=candidates.indexOf(current);if(currentIndex>=0)candidates.remove(currentIndex);}captureGeneration();return loadSample(s,candidates[uiRandom.nextInt(candidates.size())]);
}
void SampleDicerAudioProcessor::back(){if(!hasPreviousGeneration)return;auto current=previousGeneration;captureGeneration();for(int s=0;s<4;++s){if(current.files[(size_t)s].existsAsFile())loadSample(s,current.files[(size_t)s]);else clearSample(s);int index=0;for(auto name:{"gain","pitch","start","shift"})if(auto*p=state.getParameter(id(s,name))){p->setValueNotifyingHost(p->convertTo0to1(current.values[(size_t)s][(size_t)index++]));}}}
void SampleDicerAudioProcessor::getStateInformation(juce::MemoryBlock& b){if(auto x=state.copyState().createXml())copyXmlToBinary(*x,b);}
void SampleDicerAudioProcessor::setStateInformation(const void*d,int z){if(auto x=getXmlFromBinary(d,z)){state.replaceState(juce::ValueTree::fromXml(*x));for(int i=0;i<4;++i){auto p=state.state.getProperty("file"+juce::String(i+1)).toString();if(p.isNotEmpty())loadSample(i,juce::File(p));}}}
juce::AudioProcessorEditor* SampleDicerAudioProcessor::createEditor(){return new SampleDicerAudioProcessorEditor(*this);}juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new SampleDicerAudioProcessor();}
