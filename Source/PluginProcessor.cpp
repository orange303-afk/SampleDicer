#include "PluginProcessor.h"
#include "PluginEditor.h"

SampleDicerAudioProcessor::SampleDicerAudioProcessor():AudioProcessor(BusesProperties().withOutput("Output",juce::AudioChannelSet::stereo(),true)),state(*this,nullptr,"STATE",layout()){formats.registerBasicFormats();juce::PropertiesFile::Options options;options.applicationName="Sample Dicer";options.filenameSuffix="settings";options.folderName="Ilya Orange/Sample Dicer";options.osxLibrarySubFolder="Application Support";settings=std::make_unique<juce::PropertiesFile>(options);}
SampleDicerAudioProcessor::~SampleDicerAudioProcessor(){if(settings!=nullptr)settings->saveIfNeeded();}
juce::AudioProcessorValueTreeState::ParameterLayout SampleDicerAudioProcessor::layout(){
  std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
  for(int s=1;s<=4;++s){auto x="slot"+juce::String(s)+".";
    p.push_back(std::make_unique<juce::AudioParameterFloat>(x+"gain","Volume",-60.f,6.f,0.f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>(x+"pitch","Pitch",-24.f,24.f,0.f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>(x+"start","Start",0.f,.99f,0.f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>(x+"shift","Shift",0.f,250.f,0.f));}
  for(auto n:{"volume","pitch","start","shift"})p.push_back(std::make_unique<juce::AudioParameterFloat>("random."+juce::String(n),"Random "+juce::String(n),0.f,1.f,.15f));
  p.push_back(std::make_unique<juce::AudioParameterInt>("global.voices","Maximum Voices",1,16,8));
  p.push_back(std::make_unique<juce::AudioParameterBool>("global.burst","Burst",false));
  p.push_back(std::make_unique<juce::AudioParameterFloat>("global.burstRate","Burst Rate",juce::NormalisableRange<float>(1.f,30.f,.1f,0.45f),8.f));
  return{p.begin(),p.end()};}
void SampleDicerAudioProcessor::prepareToPlay(double r,int){hostRate=r;heldNotes=0;burstCountdown=0;}
bool SampleDicerAudioProcessor::isBusesLayoutSupported(const BusesLayout& l)const{return l.getMainOutputChannelSet()==juce::AudioChannelSet::mono()||l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo();}
juce::String SampleDicerAudioProcessor::id(int s,const juce::String& n)const{return"slot"+juce::String(s+1)+"."+n;}
bool SampleDicerAudioProcessor::loadSample(int s,const juce::File& f){
  if(!juce::isPositiveAndBelow(s,4)||!f.existsAsFile())return false; std::unique_ptr<juce::AudioFormatReader> r(formats.createReaderFor(f)); if(!r)return false;
  juce::AudioBuffer<float> next(juce::jmin(2,(int)r->numChannels),(int)r->lengthInSamples); r->read(&next,0,next.getNumSamples(),0,true,true);
  const juce::ScopedLock guard(lock); slots[s].audio=std::move(next);slots[s].sourceRate=r->sampleRate;slots[s].file=f;for(auto&v:slots[s].voices)v.playing=false;state.state.setProperty("file"+juce::String(s+1),f.getFullPathName(),nullptr);state.state.setProperty("lastFolder",f.getParentDirectory().getFullPathName(),nullptr);if(settings!=nullptr){settings->setValue("lastFolder",f.getParentDirectory().getFullPathName());settings->saveIfNeeded();}return true;}
bool SampleDicerAudioProcessor::browse(int s,int direction){if(!juce::isPositiveAndBelow(s,4)||!slots[s].file.existsAsFile())return false;juce::Array<juce::File> fs;
  for(auto e:juce::RangedDirectoryIterator(slots[s].file.getParentDirectory(),false,"*.wav;*.aif;*.aiff;*.flac;*.mp3",juce::File::findFiles))fs.add(e.getFile());
  fs.sort();if(fs.isEmpty())return false;auto i=fs.indexOf(slots[s].file);i=(juce::jmax(0,i)+(direction<0?-1:1)+fs.size())%fs.size();return loadSample(s,fs[i]);}
juce::String SampleDicerAudioProcessor::sampleName(int s)const{return juce::isPositiveAndBelow(s,4)&&slots[s].file.existsAsFile()?slots[s].file.getFileName():"Drop a sample";}
juce::File SampleDicerAudioProcessor::sampleFile(int s)const{return juce::isPositiveAndBelow(s,4)?slots[s].file:juce::File{};}
void SampleDicerAudioProcessor::triggerAll(float velocity){const auto maximum=juce::roundToInt(state.getRawParameterValue("global.voices")->load());for(auto&s:slots){if(s.audio.getNumSamples()==0)continue;auto chosen=-1;for(int i=0;i<maximum;++i)if(!s.voices[(size_t)i].playing){chosen=i;break;}if(chosen<0){chosen=s.nextVoice%maximum;s.nextVoice=(s.nextVoice+1)%maximum;}auto&v=s.voices[(size_t)chosen];v.position=-1;v.delay=0;v.velocity=velocity;v.playing=true;}}
void SampleDicerAudioProcessor::renderVoices(juce::AudioBuffer<float>&out){for(int k=0;k<4;++k){auto&s=slots[(size_t)k];auto start=state.getRawParameterValue(id(k,"start"))->load();auto step=s.sourceRate/hostRate*std::pow(2.,state.getRawParameterValue(id(k,"pitch"))->load()/12.);auto gain=juce::Decibels::decibelsToGain(state.getRawParameterValue(id(k,"gain"))->load());for(auto&v:s.voices){if(!v.playing)continue;if(v.position<0){v.position=start*s.audio.getNumSamples();v.delay=juce::roundToInt(state.getRawParameterValue(id(k,"shift"))->load()*.001*hostRate);}for(int i=0;i<out.getNumSamples();++i){if(v.delay>0){--v.delay;continue;}auto pos=(int)v.position;if(pos>=s.audio.getNumSamples()-1){v.playing=false;break;}auto frac=(float)(v.position-pos);for(int c=0;c<out.getNumChannels();++c){auto*d=s.audio.getReadPointer(juce::jmin(c,s.audio.getNumChannels()-1));out.addSample(c,i,juce::jmap(frac,d[pos],d[pos+1])*gain*v.velocity);}v.position+=step;}}}}
void SampleDicerAudioProcessor::processBlock(juce::AudioBuffer<float>& out,juce::MidiBuffer& midi){juce::ScopedNoDenormals n;out.clear();const juce::ScopedTryLock guard(lock);if(!guard.isLocked())return;bool started=false;
  for(auto m:midi){auto message=m.getMessage();if(message.isNoteOn()){++heldNotes;triggerAll(message.getFloatVelocity());started=true;}else if(message.isNoteOff())heldNotes=juce::jmax(0,heldNotes-1);else if(message.isAllNotesOff()||message.isAllSoundOff())heldNotes=0;}
  const auto burst=state.getRawParameterValue("global.burst")->load()>.5f;const auto interval=hostRate/state.getRawParameterValue("global.burstRate")->load();if(!burst||heldNotes==0)burstCountdown=0;else{if(started||burstCountdown<=0)burstCountdown=interval;burstCountdown-=out.getNumSamples();while(burstCountdown<=0){triggerAll(1.f);burstCountdown+=interval;}}
  renderVoices(out);}
void SampleDicerAudioProcessor::dice(bool files,bool values){for(int s=0;s<4;++s){if(files){if(slots[(size_t)s].file.existsAsFile()){for(int n=1+random.nextInt(12);n>0;--n)browse(s,1);}else{auto path=settings!=nullptr?settings->getValue("lastFolder"):juce::String{};if(path.isEmpty())path=state.state.getProperty("lastFolder").toString();juce::Array<juce::File> candidates;auto folder=juce::File(path);if(folder.isDirectory())for(auto e:juce::RangedDirectoryIterator(folder,false,"*.wav;*.aif;*.aiff;*.flac;*.mp3",juce::File::findFiles))candidates.add(e.getFile());if(!candidates.isEmpty())loadSample(s,candidates[random.nextInt(candidates.size())]);}}if(values)for(auto name:{"gain","pitch","start","shift"}){auto* p=state.getParameter(id(s,name));auto amount=state.getRawParameterValue("random."+juce::String(juce::String(name)=="gain"?"volume":name))->load();p->setValueNotifyingHost(juce::jlimit(0.f,1.f,p->getValue()+(random.nextFloat()*2-1)*amount));}}}
void SampleDicerAudioProcessor::getStateInformation(juce::MemoryBlock& b){if(auto x=state.copyState().createXml())copyXmlToBinary(*x,b);}
void SampleDicerAudioProcessor::setStateInformation(const void*d,int z){if(auto x=getXmlFromBinary(d,z)){state.replaceState(juce::ValueTree::fromXml(*x));for(int i=0;i<4;++i){auto p=state.state.getProperty("file"+juce::String(i+1)).toString();if(p.isNotEmpty())loadSample(i,juce::File(p));}}}
juce::AudioProcessorEditor* SampleDicerAudioProcessor::createEditor(){return new SampleDicerAudioProcessorEditor(*this);}juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new SampleDicerAudioProcessor();}
