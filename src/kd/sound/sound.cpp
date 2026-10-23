#include "kd/sound/sound.h"
#include "kd/sound/bass_api.h"
#include "kd/stream/file_reader.h"
#include "kd/iface/iid.h"
#include "kdcore/platform.h"
#include "bindings/sqplus.h"

#include <new>
#include <stdlib.h>

namespace vc1 {

extern const char kReadBinary[];      // display.cpp
extern const char kScriptStop[];      // sound_driver.cpp
extern const char kScriptPause[];     // sound_driver.cpp
extern const char kScriptSetVolume[]; // sound_driver.cpp
extern const char kScriptPlay[];      // music.cpp
extern const char kScriptResume[];    // music.cpp
extern const char kScriptSetLoop[];   // music.cpp
extern const char kScriptIsActive[];  // music.cpp
extern const char kScriptIsPaused[];  // music.cpp

// STRING: VC1 0x004d4bf4
static const char kSound[] = "Sound";
// GLOBAL: VC1 0x004d0650
const char* g_kdNameSound = kSound;

// MOV EAX, 0x004d4bf4; RET. IScriptObject vtable+0x14.
// FUNCTION: VC1 0x0041be80
const char* KdSound::GetTypeName()
{
    return kSound;
}

// EAX = this. Channels currently playing. Out of line in the original (Play
// calls it), so noinline.
// FUNCTION: VC1 0x0045f380
__declspec(noinline) int KdSound::CountPlaying()
{
    const BassDword count = BASS_SampleGetChannels(sample, channels);
    int playing = 0;
    for (BassDword i = 0; i < count; ++i) {
        if (BASS_ChannelIsActive(channels[i]) == BASS_ACTIVE_PLAYING) {
            ++playing;
        }
    }
    return playing;
}

// Script "IsSliding": any channel with an attribute slide in progress.
// FUNCTION: VC1 0x0045f3c0
bool KdSound::IsSliding()
{
    const BassDword count = BASS_SampleGetChannels(sample, channels);
    for (BassDword i = 0; i < count; ++i) {
        if (BASS_ChannelIsSliding(channels[i], 0)) {
            return true;
        }
    }
    return false;
}

// Script "IsPaused": every channel paused (true without channels).
// FUNCTION: VC1 0x0045f400
bool KdSound::IsPaused()
{
    const BassDword count = BASS_SampleGetChannels(sample, channels);
    for (BassDword i = 0; i < count; ++i) {
        if (BASS_ChannelIsActive(channels[i]) != BASS_ACTIVE_PAUSED) {
            return false;
        }
    }
    return true;
}

// Script "IsActive": any channel playing.
// FUNCTION: VC1 0x0045f440
bool KdSound::IsActive()
{
    const BassDword count = BASS_SampleGetChannels(sample, channels);
    for (BassDword i = 0; i < count; ++i) {
        if (BASS_ChannelIsActive(channels[i]) == BASS_ACTIVE_PLAYING) {
            return true;
        }
    }
    return false;
}

// RET 4. Script "SetLoop": BASS_SAMPLE_LOOP on every channel.
// FUNCTION: VC1 0x0045f480
bool KdSound::SetLoop(bool loop)
{
    const BassDword count = BASS_SampleGetChannels(sample, channels);
    for (BassDword i = 0; i < count; ++i) {
        BassChannelInfo info;
        BASS_ChannelGetInfo(channels[i], &info);
        if (loop) {
            info.flags |= BASS_SAMPLE_LOOP;
        }
        else {
            info.flags &= ~BASS_SAMPLE_LOOP;
        }
        BASS_ChannelFlags(channels[i], info.flags, 0xffffffff);
    }
    return true;
}

// RET 8.
// FUNCTION: VC1 0x0045f4f0
bool KdSound::SetPan(int value, int time)
{
    const BassDword count = BASS_SampleGetChannels(sample, channels);
    if (count) {
        const float pan = value * 0.01f;
        for (BassDword i = 0; i < count; ++i) {
            if (time > 0) {
                BASS_ChannelSlideAttribute(channels[i], BASS_ATTRIB_PAN, pan, time);
            }
            else {
                BASS_ChannelSetAttribute(channels[i], BASS_ATTRIB_PAN, pan);
            }
        }
    }
    return true;
}

// The first channel's pan, 0 without channels.
// FUNCTION: VC1 0x0045f550
int KdSound::GetPan()
{
    const BassDword count = BASS_SampleGetChannels(sample, channels);
    float pan = 0.0f;
    if (count) {
        BASS_ChannelGetAttribute(channels[0], BASS_ATTRIB_PAN, &pan);
    }
    return static_cast<int>(pan * 100.0);
}

// RET 8. Script "SetVolume": the sample's default volume and every channel.
// FUNCTION: VC1 0x0045f590
bool KdSound::SetVolume(int value, int time)
{
    BassSample info;
    BASS_SampleGetInfo(sample, &info);
    const float volume = value * 0.01f;
    info.volume = volume;
    BASS_SampleSetInfo(sample, &info);
    const BassDword count = BASS_SampleGetChannels(sample, channels);
    for (BassDword i = 0; i < count; ++i) {
        if (time > 0) {
            BASS_ChannelSlideAttribute(channels[i], BASS_ATTRIB_VOL, volume, time);
        }
        else {
            BASS_ChannelSetAttribute(channels[i], BASS_ATTRIB_VOL, volume);
        }
    }
    return true;
}

// RET 0xc. Script "SetChannelVolume": the sample's default volume and one
// channel (as returned by Play).
// FUNCTION: VC1 0x0045f620
bool KdSound::SetChannelVolume(unsigned long channel, int value, int time)
{
    BassSample info;
    BASS_SampleGetInfo(sample, &info);
    const float volume = value * 0.01f;
    info.volume = volume;
    BASS_SampleSetInfo(sample, &info);
    if (time > 0) {
        BASS_ChannelSlideAttribute(channel, BASS_ATTRIB_VOL, volume, time);
    }
    else {
        BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL, volume);
    }
    return true;
}

// FUNCTION: VC1 0x0045f6a0
int KdSound::GetVolume()
{
    const BassDword count = BASS_SampleGetChannels(sample, channels);
    float volume = 0.0f;
    if (count) {
        BASS_ChannelGetAttribute(channels[0], BASS_ATTRIB_VOL, &volume);
    }
    return static_cast<int>(volume * 100.0);
}

// RET 8.
// FUNCTION: VC1 0x0045f6e0
bool KdSound::SetFrequency(int value, int time)
{
    const BassDword count = BASS_SampleGetChannels(sample, channels);
    if (count) {
        const float frequency = static_cast<float>(value);
        for (BassDword i = 0; i < count; ++i) {
            if (time > 0) {
                BASS_ChannelSlideAttribute(channels[i], BASS_ATTRIB_FREQ, frequency, time);
            }
            else {
                BASS_ChannelSetAttribute(channels[i], BASS_ATTRIB_FREQ, frequency);
            }
        }
    }
    return true;
}

// FUNCTION: VC1 0x0045f740
int KdSound::GetFrequency()
{
    const BassDword count = BASS_SampleGetChannels(sample, channels);
    float frequency = 0.0f;
    if (count) {
        BASS_ChannelGetAttribute(channels[0], BASS_ATTRIB_FREQ, &frequency);
    }
    return static_cast<int>(frequency);
}

// Script "Resume".
// FUNCTION: VC1 0x0045f780
bool KdSound::Resume()
{
    const BassDword count = BASS_SampleGetChannels(sample, channels);
    for (BassDword i = 0; i < count; ++i) {
        BASS_ChannelPlay(channels[i], 0);
    }
    return true;
}

// Script "Pause".
// FUNCTION: VC1 0x0045f7b0
bool KdSound::Pause()
{
    const BassDword count = BASS_SampleGetChannels(sample, channels);
    for (BassDword i = 0; i < count; ++i) {
        BASS_ChannelPause(channels[i]);
    }
    return true;
}

// RET 4. Script "StopChannel".
// FUNCTION: VC1 0x0045f7e0
bool KdSound::StopChannel(unsigned long channel)
{
    BASS_ChannelStop(channel);
    return true;
}

// Script "Stop".
// FUNCTION: VC1 0x0045f7f0
bool KdSound::Stop()
{
    const BassDword count = BASS_SampleGetChannels(sample, channels);
    for (BassDword i = 0; i < count; ++i) {
        BASS_ChannelStop(channels[i]);
    }
    return true;
}

// RET 4. Script "Play": a new channel of the sample, or -1 when all 8 are
// playing or BASS refuses.
// FUNCTION: VC1 0x0045f820
int KdSound::Play(bool restart)
{
    if (CountPlaying() != 8) {
        const BassDword channel = BASS_SampleGetChannel(sample, 0);
        if (BASS_ChannelPlay(channel, restart)) {
            return static_cast<int>(channel);
        }
    }
    return -1;
}

// XOR AL,AL; RET 8. IScriptObject vtable+0x10.
// FUNCTION: VC1 0x0045f860
unsigned char KdSound::SetMember(void* a, void* b)
{
    (void)a;
    (void)b;
    return 0;
}

// ESI = this.
// FUNCTION: VC1 0x0045f870
KdSound::~KdSound()
{
    BASS_SampleFree(sample);
}

// FUNCTION: VC1 0x0045f8a0
void* KdSound::QueryInterface(const char** iid)
{
    const char* name = *iid;
    if (name == g_iidScriptObject) {
        if (!this) {
            return 0;
        }
        return static_cast<IScriptObject*>(this);
    }
    if (name == g_iidSound) {
        return this;
    }
    if (name == g_iidLoadable) {
        if (!this) {
            return 0;
        }
        return static_cast<ILoadable*>(this);
    }
    if (name == g_iidAbstract) {
        return static_cast<IAbstract*>(this);
    }
    return 0;
}

// FUNCTION: VC1 0x0045f8f0
unsigned KdSound::Release()
{
    IAbstract* iface = static_cast<IAbstract*>(this);
    unsigned n;

    iface->refs -= 1;
    n = iface->refs;
    if (n == 0) {
        delete iface;
        return 0;
    }
    return n;
}

// FUNCTION: VC1 0x0045f910
unsigned KdSound::AddRef()
{
    unsigned n = refs;
    if (n != 0) {
        n += 1;
        refs = n;
    }
    return n;
}

// EAX = this. The channel array stays uninitialized.
// FUNCTION: VC1 0x0045f920
KdSound::KdSound() : sample(0)
{
}

// ESI = out. Out of line at KdSoundDriver's call, as in the original.
// FUNCTION: VC1 0x0045f960
__declspec(noinline) void** CreateKdSound(void** out)
{
    KdSound* object;
    IAbstract* face;
    IUnknown* queried;

    object = static_cast<KdSound*>(malloc(sizeof(KdSound)));
    if (!object) {
        queried = 0;
    }
    else {
        new (object) KdSound();
        face = static_cast<IAbstract*>(object);
        if (!face) {
            queried = 0;
        }
        else {
            queried = static_cast<IUnknown*>(face->QueryInterface(&g_iidAbstract));
        }
    }
    *out = queried;
    if (queried) {
        queried->AddRef();
    }
    static_cast<IUnknown*>(*out)->Release();
    return out;
}

// STRING: VC1 0x004d65cc
static const char kSoundError[] = "BASS Error: %d";
// STRING: VC1 0x004d65dc
static const char kSoundLoaded[] = "Loaded %s (freq: %d origres: %d chans: %d flags: %x)";

// RET 4. this = ILoadable. Reads the whole file into memory and loads it as
// a sample with up to 8 channels, BASS_SAMPLE_OVER_POS (0x20000).
// FUNCTION: VC1 0x0045f9e0
unsigned char KdSound::Load(const char* name)
{
    KdFileHandle* file;
    unsigned size;
    void* data;
    BassSample info;

    if (sample) {
        BASS_SampleFree(sample);
    }
    file = static_cast<KdFileHandle*>(OpenKdFile(name, kReadBinary));
    if (!file) {
        return 0;
    }
    file->driver->seek(file->driver, file->native, 0, 2);
    size = static_cast<unsigned>(file->driver->tell(file->driver, file->native));
    data = malloc(size);
    file->driver->seek(file->driver, file->native, 0, 0);
    if (size != static_cast<unsigned>(file->driver->read(file->driver, data, 1, size, file->native))) {
        file->driver->close(file->driver, file->native);
        free(file);
        return 0;
    }
    sample = BASS_SampleLoad(1, data, 0, size, 8, 0x20000);
    free(data);
    file->driver->close(file->driver, file->native);
    free(file);
    if (!BASS_SampleGetInfo(sample, &info)) {
        HostLogWarning(&g_kdNameSound, kSoundError, BASS_ErrorGetCode());
        sample = 0;
        return 0;
    }
    HostLog(&g_kdNameSound, kSoundLoaded, name, info.freq, info.origres, info.chans, info.flags);
    return 1;
}

// STRING: VC1 0x004d6580
static const char kScriptSetChannelVolume[] = "SetChannelVolume";
// STRING: VC1 0x004d65b4
static const char kScriptIsSliding[] = "IsSliding";
// STRING: VC1 0x004d65c0
static const char kScriptStopChannel[] = "StopChannel";

// RET 8. IScriptObject vtable+0x0c. Method table 0x004ee4e4 (guard bit
// 0x004ee4f4, atexit 0x004caef0). The ISound slots bind through vcall thunks.
// FUNCTION: VC1 0x0045ff20
unsigned char KdSound::GetMember(const char* name, SquirrelObject* out)
{
    static ScriptMethodTable methods;

    if (methods.Empty()) {
        methods[kScriptPlay] = CreateScriptMethod<KdSound>(&KdSound::Play);
        methods[kScriptStop] = CreateScriptMethod<KdSound>(&KdSound::Stop);
        methods[kScriptPause] = CreateScriptMethod<KdSound>(&KdSound::Pause);
        methods[kScriptResume] = CreateScriptMethod<KdSound>(&KdSound::Resume);
        methods[kScriptSetChannelVolume] = CreateScriptMethod<KdSound>(&KdSound::SetChannelVolume);
        methods[kScriptSetVolume] = CreateScriptMethod<KdSound>(&KdSound::SetVolume);
        methods[kScriptSetLoop] = CreateScriptMethod<KdSound>(&KdSound::SetLoop);
        methods[kScriptIsActive] = CreateScriptMethod<KdSound>(&KdSound::IsActive);
        methods[kScriptIsPaused] = CreateScriptMethod<KdSound>(&KdSound::IsPaused);
        methods[kScriptIsSliding] = CreateScriptMethod<KdSound>(&KdSound::IsSliding);
        methods[kScriptStopChannel] = CreateScriptMethod<KdSound>(&KdSound::StopChannel);
    }
    return methods.Find(name, out);
}

}
