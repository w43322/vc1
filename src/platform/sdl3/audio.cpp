// Bridge the recovered engine's sound contract to SDL3_mixer. These are local
// compatibility symbols, not BASS imports or original-binary mappings.
#include "kd/sound/bass_api.h"
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <map>
#include <vector>
#include <algorithm>
#include <cstring>

namespace {
MIX_Mixer* mixer;
unsigned nextHandle=1;
int lastError;
struct Fade { float from,to; Uint64 begin,end; bool stop=false; };
struct Channel {
    MIX_Track* track=0;
    MIX_Audio* audio=0;
    SDL_AudioSpec format={};
    unsigned sample=0,flags=0;
    bool ownsAudio=false;
    float attrs[4]={0,44100,1,0};
    Fade fades[4]={};
};
struct Sample { MIX_Audio* audio; BassSample info; std::vector<unsigned> channels; };
std::map<unsigned,Channel> channels;
std::map<unsigned,Sample> samples;
std::vector<unsigned> resumeAfterMasterPause;
bool outputPaused;
Channel* Find(unsigned handle) { auto it=channels.find(handle);return it==channels.end() ? nullptr : &it->second; }
bool Apply(Channel& c,unsigned attr,float value)
{
    c.attrs[attr]=value;
    if(attr==BASS_ATTRIB_VOL) return MIX_SetTrackGain(c.track,value);
    if(attr==BASS_ATTRIB_FREQ) return MIX_SetTrackFrequencyRatio(c.track,value/c.format.freq);
    if(attr==BASS_ATTRIB_PAN) { MIX_StereoGains g={value>0 ? 1-value : 1,value<0 ? 1+value : 1};return MIX_SetTrackStereo(c.track,&g); }
    return false;
}
void Update(Channel& c)
{
    Uint64 now=SDL_GetTicks();
    for(unsigned a=1;a<=3;++a) {
        Fade& f=c.fades[a];
        if(!f.end) continue;
        float t=now>=f.end ? 1.f : float(now-f.begin)/float(f.end-f.begin);
        Apply(c,a,f.from+(f.to-f.from)*t);
        if(t>=1) {f.end=0;if(f.stop) MIX_StopTrack(c.track,0);}
    }
}
unsigned NewChannel(MIX_Audio* audio,unsigned sample,bool owns)
{
    MIX_Track* track=MIX_CreateTrack(mixer);
    if(!track || !MIX_SetTrackAudio(track,audio)) { if(track) MIX_DestroyTrack(track);return 0; }
    unsigned id=nextHandle++;
    Channel& c=channels[id];c.track=track;c.audio=audio;c.sample=sample;c.ownsAudio=owns;
    MIX_GetAudioFormat(audio,&c.format);c.attrs[BASS_ATTRIB_FREQ]=float(c.format.freq);
    return id;
}
void FreeChannel(unsigned id)
{
    auto it=channels.find(id);if(it==channels.end()) return;
    MIX_DestroyTrack(it->second.track);
    if(it->second.ownsAudio) MIX_DestroyAudio(it->second.audio);
    channels.erase(it);
}
struct File { BassFileProcs procs;void* user;Sint64 position; };
Sint64 SDLCALL Size(void* p) { File& f=*static_cast<File*>(p);return f.procs.length(f.user); }
Sint64 SDLCALL Seek(void* p,Sint64 offset,SDL_IOWhence whence)
{
    File& f=*static_cast<File*>(p);
    Sint64 next=(whence==SDL_IO_SEEK_CUR ? f.position : whence==SDL_IO_SEEK_END ? Size(p) : 0)+offset;
    if(next<0 || !f.procs.seek(next,f.user)) return -1;
    return f.position=next;
}
size_t SDLCALL Read(void* p,void* data,size_t size,SDL_IOStatus* status)
{
    File& f=*static_cast<File*>(p);size_t n=f.procs.read(data,static_cast<BassDword>(size),f.user);
    f.position+=n;*status=n<size ? SDL_IO_STATUS_EOF : SDL_IO_STATUS_READY;return n;
}
bool SDLCALL Close(void* p) { File* f=static_cast<File*>(p);f->procs.close(f->user);delete f;return true; }
}
namespace vc1 { void SdlUpdateAudio() { for(auto& item:channels) Update(item.second); } }
extern "C" {
int __stdcall BASS_Init(int,BassDword,BassDword,void*,const void*)
{
    if(mixer) return 1;
    if(!SDL_InitSubSystem(SDL_INIT_AUDIO) || !MIX_Init()) {lastError=-1;return 0;}
    mixer=MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,nullptr);
    outputPaused=false;lastError=mixer ? 0 : -1;return mixer!=nullptr;
}
int __stdcall BASS_Free()
{
    while(!channels.empty()) FreeChannel(channels.begin()->first);
    for(auto& s:samples) MIX_DestroyAudio(s.second.audio);
    samples.clear();if(mixer) MIX_DestroyMixer(mixer);mixer=nullptr;
    resumeAfterMasterPause.clear();outputPaused=false;
    MIX_Quit();SDL_QuitSubSystem(SDL_INIT_AUDIO);return 1;
}
BassDword __stdcall BASS_GetVersion() { return (3u<<24)|(2u<<16)|(4u<<8); }
int __stdcall BASS_ErrorGetCode() { return lastError; }
BassDword __stdcall BASS_GetDevice() { return 1; }
int __stdcall BASS_GetDeviceInfo(BassDword,BassDeviceInfo* info)
{
    info->name="SDL3 default playback device";info->driver="SDL3_mixer";info->flags=1;return 1;
}
int __stdcall BASS_Start()
{
    if(!mixer) return 0;
    for(unsigned h:resumeAfterMasterPause) {Channel* c=Find(h);if(c && MIX_TrackPaused(c->track)) MIX_ResumeTrack(c->track);}
    resumeAfterMasterPause.clear();outputPaused=false;return 1;
}
int __stdcall BASS_Stop()
{
    if(!mixer) return 0;
    resumeAfterMasterPause.clear();outputPaused=true;return MIX_StopAllTracks(mixer,0);
}
int __stdcall BASS_Pause()
{
    if(!mixer) return 0;if(outputPaused) return 1;
    resumeAfterMasterPause.clear();
    for(auto& entry:channels) if(MIX_TrackPlaying(entry.second.track)) resumeAfterMasterPause.push_back(entry.first);
    outputPaused=true;return MIX_PauseAllTracks(mixer);
}
float __stdcall BASS_GetVolume() { return mixer ? MIX_GetMixerGain(mixer) : 0; }
int __stdcall BASS_SetVolume(float v) { return mixer && MIX_SetMixerGain(mixer,v); }
BassDword __stdcall BASS_SampleLoad(int mem,const void* data,BassQword offset,BassDword length,BassDword max,BassDword flags)
{
    if(!mixer) return 0;
    MIX_Audio* audio=mem ? MIX_LoadAudio_IO(mixer,SDL_IOFromConstMem(static_cast<const char*>(data)+offset,length),true,true)
        : MIX_LoadAudio(mixer,static_cast<const char*>(data),true);
    if(!audio) { lastError=-1;return 0; }
    unsigned id=nextHandle++;Sample& s=samples[id];s.audio=audio;s.info={};
    SDL_AudioSpec fmt;MIX_GetAudioFormat(audio,&fmt);
    s.info.freq=fmt.freq;s.info.volume=1;s.info.flags=flags;s.info.chans=fmt.channels;
    s.info.origres=16;s.info.max=max ? max : 1;s.info.length=static_cast<BassDword>(MIX_GetAudioDuration(audio)*fmt.channels*2);
    return id;
}
int __stdcall BASS_SampleFree(BassDword id)
{
    auto it=samples.find(id);if(it==samples.end()) return 0;
    for(unsigned channel:it->second.channels) FreeChannel(channel);
    MIX_DestroyAudio(it->second.audio);samples.erase(it);return 1;
}
int __stdcall BASS_SampleGetInfo(BassDword id,BassSample* info)
{
    auto it=samples.find(id);if(it==samples.end()) return 0;*info=it->second.info;return 1;
}
int __stdcall BASS_SampleSetInfo(BassDword id,const BassSample* info)
{
    auto it=samples.find(id);if(it==samples.end()) return 0;it->second.info=*info;return 1;
}
BassDword __stdcall BASS_SampleGetChannel(BassDword id,int onlyNew)
{
    auto it=samples.find(id);if(it==samples.end()) return 0;Sample& s=it->second;
    for(unsigned h:s.channels) if(!MIX_TrackPlaying(Find(h)->track) && !MIX_TrackPaused(Find(h)->track)) return h;
    if(s.channels.size()>=s.info.max) return onlyNew ? 0 : s.channels.front();
    unsigned h=NewChannel(s.audio,id,false);if(!h) return 0;
    s.channels.push_back(h);Channel& c=*Find(h);c.flags=s.info.flags;
    Apply(c,BASS_ATTRIB_VOL,s.info.volume);Apply(c,BASS_ATTRIB_PAN,s.info.pan);Apply(c,BASS_ATTRIB_FREQ,float(s.info.freq));
    return h;
}
BassDword __stdcall BASS_SampleGetChannels(BassDword id,BassDword* out)
{
    auto it=samples.find(id);if(it==samples.end()) return 0;
    unsigned count=0;for(unsigned h:it->second.channels) {if(out) out[count]=h;++count;}return count;
}
BassDword __stdcall BASS_StreamCreateFileUser(BassDword,BassDword flags,const BassFileProcs* procs,void* user)
{
    File* f=new File{*procs,user,0};SDL_IOStreamInterface iface;SDL_INIT_INTERFACE(&iface);
    iface.size=Size;iface.seek=Seek;iface.read=Read;iface.close=Close;
    SDL_IOStream* io=SDL_OpenIO(&iface,f);
    if(!io) {Close(f);return 0;}
    MIX_Audio* audio=MIX_LoadAudio_IO(mixer,io,false,true);
    if(!audio) {lastError=-1;return 0;}
    unsigned h=NewChannel(audio,0,true);
    if(!h) {MIX_DestroyAudio(audio);return 0;}Find(h)->flags=flags;return h;
}
int __stdcall BASS_StreamFree(BassDword h) { if(!Find(h)) return 0;FreeChannel(h);return 1; }
int __stdcall BASS_ChannelPlay(BassDword h,int restart)
{
    Channel* c=Find(h);if(!c) return 0;
    if(outputPaused) {lastError=9;return 0;}
    if(!restart && MIX_TrackPaused(c->track)) return MIX_ResumeTrack(c->track);
    if(!restart && MIX_TrackPlaying(c->track)) return 1;
    SDL_PropertiesID props=SDL_CreateProperties();
    SDL_SetNumberProperty(props,MIX_PROP_PLAY_LOOPS_NUMBER,(c->flags&BASS_SAMPLE_LOOP) ? -1 : 0);
    if(!restart) SDL_SetNumberProperty(props,MIX_PROP_PLAY_START_FRAME_NUMBER,MIX_GetTrackPlaybackPosition(c->track));
    bool ok=MIX_PlayTrack(c->track,props);SDL_DestroyProperties(props);return ok;
}
int __stdcall BASS_ChannelStop(BassDword h) { Channel* c=Find(h);return c && MIX_StopTrack(c->track,0); }
int __stdcall BASS_ChannelPause(BassDword h)
{
    resumeAfterMasterPause.erase(std::remove(resumeAfterMasterPause.begin(),resumeAfterMasterPause.end(),h),resumeAfterMasterPause.end());
    Channel* c=Find(h);return c && MIX_PauseTrack(c->track);
}
BassDword __stdcall BASS_ChannelIsActive(BassDword h)
{
    Channel* c=Find(h);if(!c) return BASS_ACTIVE_STOPPED;
    return MIX_TrackPaused(c->track) ? BASS_ACTIVE_PAUSED : MIX_TrackPlaying(c->track) ? BASS_ACTIVE_PLAYING : BASS_ACTIVE_STOPPED;
}
BassDword __stdcall BASS_ChannelFlags(BassDword h,BassDword flags,BassDword mask)
{
    Channel* c=Find(h);if(!c) return 0xffffffff;
    c->flags=(c->flags&~mask)|(flags&mask);MIX_SetTrackLoops(c->track,(c->flags&BASS_SAMPLE_LOOP) ? -1 : 0);return c->flags;
}
int __stdcall BASS_ChannelGetAttribute(BassDword h,BassDword attr,float* value)
{
    Channel* c=Find(h);if(!c || attr<1 || attr>3) return 0;Update(*c);*value=c->attrs[attr];return 1;
}
int __stdcall BASS_ChannelSetAttribute(BassDword h,BassDword attr,float value)
{
    Channel* c=Find(h);if(!c || attr<1 || attr>3) return 0;c->fades[attr].end=0;return Apply(*c,attr,value);
}
int __stdcall BASS_ChannelSlideAttribute(BassDword h,BassDword attr,float value,BassDword ms)
{
    Channel* c=Find(h);if(!c || attr<1 || attr>3) return 0;Update(*c);
    if(!ms) return BASS_ChannelSetAttribute(h,attr,value);
    Uint64 now=SDL_GetTicks();bool stop=attr==BASS_ATTRIB_VOL && value<0;
    c->fades[attr]={c->attrs[attr],stop ? 0.f : value,now,now+ms,stop};return 1;
}
int __stdcall BASS_ChannelIsSliding(BassDword h,BassDword attr)
{
    Channel* c=Find(h);if(!c) return 0;Update(*c);
    return attr==0 ? (c->fades[1].end || c->fades[2].end || c->fades[3].end) : attr<=3 && c->fades[attr].end;
}
int __stdcall BASS_ChannelGetInfo(BassDword h,BassChannelInfo* info)
{
    Channel* c=Find(h);if(!c) return 0;
    *info={};info->freq=c->format.freq;info->chans=c->format.channels;info->flags=c->flags;
    info->origres=16;info->ctype=0x10005;info->sample=c->sample;return 1;
}
BassQword __stdcall BASS_ChannelSeconds2Bytes(BassDword h,double sec)
{
    Channel* c=Find(h);return c ? static_cast<BassQword>(sec*c->format.freq)*c->format.channels*2 : 0;
}
double __stdcall BASS_ChannelBytes2Seconds(BassDword h,BassQword pos)
{
    Channel* c=Find(h);return c ? double(pos)/(c->format.freq*c->format.channels*2) : 0;
}
int __stdcall BASS_ChannelSetPosition(BassDword h,BassQword pos,BassDword)
{
    Channel* c=Find(h);return c && MIX_SetTrackPlaybackPosition(c->track,pos/(c->format.channels*2));
}
BassQword __stdcall BASS_ChannelGetPosition(BassDword h,BassDword)
{
    Channel* c=Find(h);return c ? MIX_GetTrackPlaybackPosition(c->track)*c->format.channels*2 : 0;
}
BassQword __stdcall BASS_ChannelGetLength(BassDword h,BassDword)
{
    Channel* c=Find(h);return c ? MIX_GetAudioDuration(c->audio)*c->format.channels*2 : 0;
}
}
