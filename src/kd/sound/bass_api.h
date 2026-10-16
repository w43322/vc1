#ifndef VC1_KD_SOUND_BASS_API_H
#define VC1_KD_SOUND_BASS_API_H

// The parts of the BASS 2.4 API (un4seen, BASS.dll beside the game) the
// original calls. Declared without dllimport: the original calls the import
// library's jmp thunks (0x0044467a..0x00444740, config/verified.csv).

typedef unsigned long BassDword;
typedef unsigned __int64 BassQword;

// BASS_DEVICEINFO
struct BassDeviceInfo
{
    const char* name;
    const char* driver;
    BassDword flags;
};

// BASS_SAMPLE
struct BassSample
{
    BassDword freq;
    float volume;
    float pan;
    BassDword flags;
    BassDword length;
    BassDword max;
    BassDword origres;
    BassDword chans;
    BassDword mingap;
    BassDword mode3d;
    float mindist;
    float maxdist;
    BassDword iangle;
    BassDword oangle;
    float outvol;
    BassDword vam;
    BassDword priority;
};

// BASS_CHANNELINFO
struct BassChannelInfo
{
    BassDword freq;
    BassDword chans;
    BassDword flags;
    BassDword ctype;
    BassDword origres;
    BassDword plugin;
    BassDword sample;
    const char* filename;
};

// BASS_FILEPROCS
struct BassFileProcs
{
    void (__stdcall* close)(void* user);
    BassQword (__stdcall* length)(void* user);
    BassDword (__stdcall* read)(void* buffer, BassDword length, void* user);
    int (__stdcall* seek)(BassQword offset, void* user);
};

enum
{
    BASS_ACTIVE_STOPPED = 0,
    BASS_ACTIVE_PLAYING = 1,
    BASS_ACTIVE_STALLED = 2,
    BASS_ACTIVE_PAUSED = 3,

    BASS_ATTRIB_FREQ = 1,
    BASS_ATTRIB_VOL = 2,
    BASS_ATTRIB_PAN = 3,

    BASS_POS_BYTE = 0,
    BASS_SAMPLE_LOOP = 4
};

extern "C" {
int __stdcall BASS_Free();
BassDword __stdcall BASS_GetVersion();
int __stdcall BASS_Init(int device, BassDword freq, BassDword flags, void* win, const void* clsid);
int __stdcall BASS_ErrorGetCode();
int __stdcall BASS_GetDeviceInfo(BassDword device, BassDeviceInfo* info);
BassDword __stdcall BASS_GetDevice();
int __stdcall BASS_Start();
int __stdcall BASS_Stop();
int __stdcall BASS_Pause();
float __stdcall BASS_GetVolume();
int __stdcall BASS_SetVolume(float volume);
int __stdcall BASS_SampleFree(BassDword handle);
BassDword __stdcall BASS_SampleLoad(int mem, const void* file, BassQword offset, BassDword length,
    BassDword max, BassDword flags);
int __stdcall BASS_SampleGetInfo(BassDword handle, BassSample* info);
BassDword __stdcall BASS_SampleGetChannel(BassDword handle, int onlyNew);
int __stdcall BASS_ChannelPlay(BassDword handle, int restart);
BassDword __stdcall BASS_SampleGetChannels(BassDword handle, BassDword* channels);
int __stdcall BASS_ChannelStop(BassDword handle);
int __stdcall BASS_ChannelPause(BassDword handle);
int __stdcall BASS_ChannelGetAttribute(BassDword handle, BassDword attrib, float* value);
int __stdcall BASS_ChannelSlideAttribute(BassDword handle, BassDword attrib, float value, BassDword time);
int __stdcall BASS_ChannelSetAttribute(BassDword handle, BassDword attrib, float value);
int __stdcall BASS_SampleSetInfo(BassDword handle, const BassSample* info);
int __stdcall BASS_ChannelGetInfo(BassDword handle, BassChannelInfo* info);
BassDword __stdcall BASS_ChannelFlags(BassDword handle, BassDword flags, BassDword mask);
BassDword __stdcall BASS_ChannelIsActive(BassDword handle);
int __stdcall BASS_ChannelIsSliding(BassDword handle, BassDword attrib);
int __stdcall BASS_StreamFree(BassDword handle);
BassDword __stdcall BASS_StreamCreateFileUser(BassDword system, BassDword flags,
    const BassFileProcs* procs, void* user);
BassQword __stdcall BASS_ChannelSeconds2Bytes(BassDword handle, double pos);
int __stdcall BASS_ChannelSetPosition(BassDword handle, BassQword pos, BassDword mode);
BassQword __stdcall BASS_ChannelGetPosition(BassDword handle, BassDword mode);
double __stdcall BASS_ChannelBytes2Seconds(BassDword handle, BassQword pos);
BassQword __stdcall BASS_ChannelGetLength(BassDword handle, BassDword mode);
}

#endif
