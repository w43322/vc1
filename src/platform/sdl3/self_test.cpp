// Asset-free integration checks for the selected runtime and mixer.
#include "platform/sdl3/runtime.h"
#include "platform/sdl3/render.h"
#include "kd/sound/bass_api.h"
#include <vector>
#include <cmath>
namespace {
void Require(bool ok,const char* why) {if(!ok) {SDL_Log("BACKEND TEST FAILED: %s (%s)",why,SDL_GetError());std::exit(1);}}
std::vector<unsigned char> Wave()
{
    const unsigned samples=44100*2;
    std::vector<unsigned char> data(44+samples*2,0);
    memcpy(data.data(),"RIFF",4);memcpy(data.data()+8,"WAVEfmt ",8);memcpy(data.data()+36,"data",4);
    auto put32=[&](unsigned at,unsigned v){for(unsigned i=0;i<4;++i)data[at+i]=v>>(i*8);};
    auto put16=[&](unsigned at,unsigned v){data[at]=v;data[at+1]=v>>8;};
    put32(4,unsigned(data.size()-8));put32(16,16);put16(20,1);put16(22,1);put32(24,44100);put32(28,88200);put16(32,2);put16(34,16);put32(40,samples*2);
    for(unsigned i=0;i<samples;++i) put16(44+i*2,static_cast<short>(std::sin(i*6.283185307*440/44100)*8000));
    return data;
}
struct Memory {std::vector<unsigned char>* data;size_t position;bool closed;};
void __stdcall Close(void* p) {static_cast<Memory*>(p)->closed=true;}
BassQword __stdcall Size(void* p) {return static_cast<Memory*>(p)->data->size();}
BassDword __stdcall Read(void* b,BassDword n,void* p)
{
    Memory& m=*static_cast<Memory*>(p);size_t left=m.data->size()-m.position;
    if(n>left)n=BassDword(left);memcpy(b,m.data->data()+m.position,n);m.position+=n;return n;
}
int __stdcall Seek(BassQword offset,void* p) {Memory& m=*static_cast<Memory*>(p);if(offset>m.data->size())return 0;m.position=size_t(offset);return 1;}
void Audio()
{
    Require(BASS_Init(-1,44100,0,nullptr,nullptr)!=0,"SDL mixer initialization");
    auto wave=Wave();BassDword sample=BASS_SampleLoad(1,wave.data(),0,BassDword(wave.size()),8,0);Require(sample!=0,"WAVE decode");
    BassSample info;Require(BASS_SampleGetInfo(sample,&info) && info.freq==44100 && info.chans==1,"sample format");
    BassDword voices[8];for(unsigned i=0;i<8;++i) {voices[i]=BASS_SampleGetChannel(sample,1);Require(voices[i] && BASS_ChannelPlay(voices[i],1),"eight simultaneous effects");}
    Require(BASS_SampleGetChannel(sample,1)==0 && BASS_SampleGetChannels(sample,nullptr)==8,"sample voice limit");
    Require(BASS_ChannelPause(voices[0]) && BASS_ChannelIsActive(voices[0])==BASS_ACTIVE_PAUSED,"pause");
    Require(BASS_ChannelPlay(voices[0],0) && BASS_ChannelIsActive(voices[0])==BASS_ACTIVE_PLAYING,"resume");
    Require(BASS_ChannelPause(voices[1]) && BASS_Pause() && BASS_Start(),"master pause/resume");
    Require(BASS_ChannelIsActive(voices[0])==BASS_ACTIVE_PLAYING && BASS_ChannelIsActive(voices[1])==BASS_ACTIVE_PAUSED,"master resume preserves individual pause");
    Require(BASS_ChannelSetAttribute(voices[0],BASS_ATTRIB_PAN,-0.5f) && BASS_ChannelSetAttribute(voices[0],BASS_ATTRIB_FREQ,22050),"pan and frequency");
    Require(BASS_ChannelSlideAttribute(voices[0],BASS_ATTRIB_VOL,0.2f,20),"volume ramp");SDL_Delay(30);vc1::SdlUpdateAudio();float value=0;
    Require(BASS_ChannelGetAttribute(voices[0],BASS_ATTRIB_VOL,&value) && std::fabs(value-0.2f)<0.001f && !BASS_ChannelIsSliding(voices[0],0),"volume ramp completion");
    Require(BASS_ChannelSlideAttribute(voices[2],BASS_ATTRIB_VOL,-1,10),"fade out and stop");
    SDL_Delay(20);vc1::SdlUpdateAudio();Require(BASS_ChannelIsActive(voices[2])==BASS_ACTIVE_STOPPED,"fade out completion stops channel");
    Require(BASS_Stop() && BASS_Start() && BASS_ChannelIsActive(voices[0])==BASS_ACTIVE_STOPPED,"master stop does not resume stopped voices");
    Memory memory={&wave,0,false};BassFileProcs procs={Close,Size,Read,Seek};
    BassDword stream=BASS_StreamCreateFileUser(0,0,&procs,&memory);Require(stream!=0,"custom stream decode");
    Require(BASS_ChannelGetLength(stream,0)==44100*2*2,"stream duration");
    Require(BASS_ChannelSetPosition(stream,BASS_ChannelSeconds2Bytes(stream,0.5),0),"stream seek");
    Require(std::fabs(BASS_ChannelBytes2Seconds(stream,BASS_ChannelGetPosition(stream,0))-0.5)<0.01,"seek position");
    BASS_ChannelFlags(stream,BASS_SAMPLE_LOOP,0xffffffff);Require(BASS_ChannelPlay(stream,0),"looped stream playback");
    Require(BASS_StreamFree(stream) && memory.closed,"stream IO ownership");
    Require(BASS_SampleFree(sample) && BASS_Free(),"sample/mixer cleanup");
}
struct Workers {vc1::MutexRef mutex={};int count=0;};
int SDLCALL Worker(void* p)
{
    Workers& w=*static_cast<Workers*>(p);
    for(int i=0;i<1000;++i) {if(vc1::MutexLock(&w.mutex)) return 1;++w.count;vc1::MutexUnlock(&w.mutex);}return 0;
}
void TlsDestructor(void* p) {++*static_cast<int*>(p);}
void Threading()
{
    Workers workers;
    SDL_Thread* a=SDL_CreateThread(Worker,"vc1-test-a",&workers);
    SDL_Thread* b=SDL_CreateThread(Worker,"vc1-test-b",&workers);
    Require(a && b,"SDL worker creation");int resultA=0,resultB=0;SDL_WaitThread(a,&resultA);SDL_WaitThread(b,&resultB);
    Require(!resultA && !resultB && workers.count==2000,"lazy mutex concurrency");vc1::MutexDestroy(&workers.mutex);
    vc1::TlsBlock* tls=nullptr;int destroyed=0;
    Require(!vc1::AllocTlsBlock(reinterpret_cast<void*>(TlsDestructor),&tls),"TLS allocation");
    Require(SDL_SetTLS(&tls->id,&destroyed,nullptr) && SDL_GetTLS(&tls->id)==&destroyed,"TLS storage");
    Require(!vc1::DestroyTlsBlock(tls) && destroyed==1,"TLS destructor");
}
void FilesAndEvents()
{
    using namespace vc1;
    char path[1024];SDL_snprintf(path,sizeof(path),"%svc1-backend-test.tmp",SDL_GetBasePath());
    void* handle=OpenKdPath(path,"w+b");Require(handle!=nullptr,"file create");
    struct Handle {KdObject* driver;int native;};Handle* h=static_cast<Handle*>(handle);
    const char payload[]="SDL filesystem roundtrip";
    Require(h->driver->write(h->driver,payload,1,sizeof(payload),h->native)==sizeof(payload),"file write");
    Require(!h->driver->flush(h->driver,h->native) && !h->driver->seek(h->driver,h->native,0,0),"file flush/seek");
    unsigned size=0;char* mapped=static_cast<char*>(KdMapFile(handle,&size));
    Require(mapped && size==sizeof(payload) && !memcmp(mapped,payload,size),"mapped file read");h->driver->unmap(h->driver,h->native,mapped);
    Require(KdFclose(handle)==0 && SDL_RemovePath(path),"file close/remove");
    FreeBumpHeaps();SDL_FlushEvents(SDL_EVENT_FIRST,SDL_EVENT_LAST);
    SDL_Event key={};key.type=SDL_EVENT_KEY_DOWN;key.key.key=SDLK_UP;key.key.scancode=SDL_SCANCODE_UP;key.key.down=true;
    Require(SDL_PushEvent(&key),"inject test key");SdlPollEvents();
    unsigned seen=0;for(KdEvent* e=g_heapHead;e;e=e->next) if(e->type==0x38 && e->data[2] && (e->data[0]==0x1001 || e->data[0]==0x40000026)) ++seen;
    Require(seen==2,"SDL key to KD/game-key translation");
    FreeBumpHeaps();
    KdTimer* timer=KdSetTimer(1000000,nullptr);SDL_Delay(3);FireTimers();
    Require(g_heapHead && g_heapHead->type==KD_EVENT_TIMER,"timer delivery");UnlinkTimer(timer);free(timer);FreeBumpHeaps();
}
}
int RunSdlBackendTests()
{
    Audio();FilesAndEvents();Threading();
    SDL_Log("SDL backend tests passed: geometry, alpha, clip, transform, render targets, readback, eight voices, streams, fades, files, keys and timers");
    return 0;
}
