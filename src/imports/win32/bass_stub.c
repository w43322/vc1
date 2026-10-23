/* Placeholder bodies with the stdcall argument sizes of the BASS 2.4 API.
   Linked into a throwaway DLL only to get an import library for the real
   BASS.dll (see cmake/bass.cmake); never shipped or called. */
#define STUB0(name) int __stdcall name(void) { return 0; }
#define STUB1(name) int __stdcall name(int a) { return a; }
#define STUB2(name) int __stdcall name(int a, int b) { return a + b; }
#define STUB3(name) int __stdcall name(int a, int b, int c) { return a + b + c; }
#define STUB4(name) int __stdcall name(int a, int b, int c, int d) { return a + b + c + d; }
#define STUB5(name) int __stdcall name(int a, int b, int c, int d, int e) { return a + b + c + d + e; }
#define STUB7(name) int __stdcall name(int a, int b, int c, int d, int e, int f, int g) \
    { return a + b + c + d + e + f + g; }

STUB0(BASS_Free)
STUB0(BASS_GetVersion)
STUB5(BASS_Init)
STUB0(BASS_ErrorGetCode)
STUB2(BASS_GetDeviceInfo)
STUB0(BASS_GetDevice)
STUB0(BASS_Start)
STUB0(BASS_Stop)
STUB0(BASS_Pause)
STUB0(BASS_GetVolume)
STUB1(BASS_SetVolume)
STUB1(BASS_SampleFree)
STUB7(BASS_SampleLoad)
STUB2(BASS_SampleGetInfo)
STUB2(BASS_SampleGetChannel)
STUB2(BASS_ChannelPlay)
STUB2(BASS_SampleGetChannels)
STUB1(BASS_ChannelStop)
STUB1(BASS_ChannelPause)
STUB3(BASS_ChannelGetAttribute)
STUB4(BASS_ChannelSlideAttribute)
STUB3(BASS_ChannelSetAttribute)
STUB2(BASS_SampleSetInfo)
STUB2(BASS_ChannelGetInfo)
STUB3(BASS_ChannelFlags)
STUB1(BASS_ChannelIsActive)
STUB2(BASS_ChannelIsSliding)
STUB1(BASS_StreamFree)
STUB4(BASS_StreamCreateFileUser)
STUB3(BASS_ChannelSeconds2Bytes)
STUB4(BASS_ChannelSetPosition)
STUB2(BASS_ChannelGetPosition)
STUB3(BASS_ChannelBytes2Seconds)
STUB2(BASS_ChannelGetLength)
