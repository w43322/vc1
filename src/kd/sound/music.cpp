#include "kd/sound/music.h"
#include "kd/sound/bass_api.h"
#include "kd/stream/file_reader.h"
#include "kd/iface/iid.h"
#include "host/runtime.h"
#include "bindings/sqplus.h"

#include <new>
#include <stdlib.h>

namespace vc1 {

extern const char kReadBinary[];      // display.cpp
extern const char kScriptStop[];      // sound_driver.cpp
extern const char kScriptPause[];     // sound_driver.cpp
extern const char kScriptSetVolume[]; // sound_driver.cpp

// STRING: VC1 0x004d4bec
static const char kMusic[] = "Music";
// GLOBAL: VC1 0x004d0654
const char* g_kdNameMusic = kMusic;

// Script names shared with KdSound.
// GLOBAL: VC1 0x004d6570
extern const char kScriptPlay[] = "Play";
// GLOBAL: VC1 0x004d6578
extern const char kScriptResume[] = "Resume";
// GLOBAL: VC1 0x004d6594
extern const char kScriptSetLoop[] = "SetLoop";
// GLOBAL: VC1 0x004d659c
extern const char kScriptIsActive[] = "IsActive";
// GLOBAL: VC1 0x004d65a8
extern const char kScriptIsPaused[] = "IsPaused";
// GLOBAL: VC1 0x004d6694
extern const char kScriptSetPosition[] = "SetPosition";
// GLOBAL: VC1 0x004d66a0
extern const char kScriptGetPosition[] = "GetPosition";

// MOV EAX, 0x004d4bec; RET. IScriptObject vtable+0x14.
// FUNCTION: VC1 0x0041c3a0
const char* KdMusic::Script14()
{
    return kMusic;
}

// FUNCTION: VC1 0x0045e490
bool KdMusic::IsStarted()
{
    return BASS_ChannelIsActive(stream) != BASS_ACTIVE_STOPPED;
}

// FUNCTION: VC1 0x0045e4a0
bool KdMusic::IsPaused()
{
    return BASS_ChannelIsActive(stream) == BASS_ACTIVE_PAUSED;
}

// FUNCTION: VC1 0x0045e4c0
bool KdMusic::IsActive()
{
    return BASS_ChannelIsActive(stream) == BASS_ACTIVE_PLAYING;
}

// RET 4. Streams of a second or less never loop; otherwise BASS_SAMPLE_LOOP
// is set or cleared with an all-ones mask.
// FUNCTION: VC1 0x0045e4d0
bool KdMusic::SetLoop(bool loop)
{
    const double seconds = BASS_ChannelBytes2Seconds(stream,
        BASS_ChannelGetLength(stream, BASS_POS_BYTE));
    if (seconds < 1.0) {
        return false;
    }
    return BASS_ChannelFlags(stream, loop ? BASS_SAMPLE_LOOP : 0, 0xffffffff) != 0;
}

// RET 8.
// FUNCTION: VC1 0x0045e520
bool KdMusic::SetPan(int value, int time)
{
    const float pan = value * 0.01f;
    if (time > 0) {
        return BASS_ChannelSlideAttribute(stream, BASS_ATTRIB_PAN, pan, time) != 0;
    }
    return BASS_ChannelSetAttribute(stream, BASS_ATTRIB_PAN, pan) != 0;
}

// FUNCTION: VC1 0x0045e580
int KdMusic::GetPan()
{
    float pan = 0.0f;
    BASS_ChannelGetAttribute(stream, BASS_ATTRIB_PAN, &pan);
    return static_cast<int>(pan * 100.0);
}

// RET 8. Script "SetVolume".
// FUNCTION: VC1 0x0045e5b0
bool KdMusic::SetVolume(int value, int time)
{
    const float volume = value * 0.01f;
    if (time > 0) {
        return BASS_ChannelSlideAttribute(stream, BASS_ATTRIB_VOL, volume, time) != 0;
    }
    return BASS_ChannelSetAttribute(stream, BASS_ATTRIB_VOL, volume) != 0;
}

// FUNCTION: VC1 0x0045e610
int KdMusic::GetVolume()
{
    float volume = 0.0f;
    BASS_ChannelGetAttribute(stream, BASS_ATTRIB_VOL, &volume);
    return static_cast<int>(volume * 100.0);
}

// RET 8.
// FUNCTION: VC1 0x0045e640
bool KdMusic::SetFrequency(int value, int time)
{
    if (time > 0) {
        return BASS_ChannelSlideAttribute(stream, BASS_ATTRIB_FREQ,
            static_cast<float>(value), time) != 0;
    }
    return BASS_ChannelSetAttribute(stream, BASS_ATTRIB_FREQ, static_cast<float>(value)) != 0;
}

// FUNCTION: VC1 0x0045e680
int KdMusic::GetFrequency()
{
    float frequency = 0.0f;
    BASS_ChannelGetAttribute(stream, BASS_ATTRIB_FREQ, &frequency);
    return static_cast<int>(frequency);
}

// Script "Resume".
// FUNCTION: VC1 0x0045e6a0
bool KdMusic::Resume()
{
    return BASS_ChannelPlay(stream, 0) != 0;
}

// Script "Pause".
// FUNCTION: VC1 0x0045e6c0
bool KdMusic::Pause()
{
    return BASS_ChannelPause(stream) != 0;
}

// Script "Stop".
// FUNCTION: VC1 0x0045e6d0
bool KdMusic::Stop()
{
    return BASS_ChannelStop(stream) != 0;
}

// Script "GetPosition": the playback position in seconds.
// FUNCTION: VC1 0x0045e6e0
float KdMusic::GetPosition()
{
    return static_cast<float>(BASS_ChannelBytes2Seconds(stream,
        BASS_ChannelGetPosition(stream, BASS_POS_BYTE)));
}

// RET 4. Script "SetPosition".
// FUNCTION: VC1 0x0045e710
bool KdMusic::SetPosition(float seconds)
{
    return BASS_ChannelSetPosition(stream,
        BASS_ChannelSeconds2Bytes(stream, seconds), BASS_POS_BYTE) > 0;
}

// RET 4. Script "Play": the stream handle, or -1 when BASS refuses.
// FUNCTION: VC1 0x0045e750
int KdMusic::Play(bool restart)
{
    if (BASS_ChannelPlay(stream, restart)) {
        return static_cast<int>(stream);
    }
    return -1;
}

// XOR AL,AL; RET 8. IScriptObject vtable+0x10.
// FUNCTION: VC1 0x0045e780
unsigned char KdMusic::Script10(void* a, void* b)
{
    (void)a;
    (void)b;
    return 0;
}

// ESI = this.
// FUNCTION: VC1 0x0045e790
KdMusic::~KdMusic()
{
    BASS_StreamFree(stream);
}

// FUNCTION: VC1 0x0045e7c0
void* KdMusic::QueryInterface(const char** iid)
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

// FUNCTION: VC1 0x0045e810
unsigned KdMusic::Release()
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

// FUNCTION: VC1 0x0045e830
unsigned KdMusic::AddRef()
{
    unsigned n = refs;
    if (n != 0) {
        n += 1;
        refs = n;
    }
    return n;
}

// Inferred name. BASS FILESEEKPROC over the KD file handle (driver +0x140,
// SEEK_SET); RET 0xc.
// FUNCTION: VC1 0x0045e840
static int __stdcall MusicFileSeek(BassQword offset, void* user)
{
    KdFileHandle* const file = static_cast<KdFileHandle*>(user);
    if (!file) {
        return 0;
    }
    return file->driver->seek(file->driver, file->native, offset, 0) == 0;
}

// Inferred name. BASS FILEREADPROC (driver +0x120); RET 0xc.
// FUNCTION: VC1 0x0045e880
static BassDword __stdcall MusicFileRead(void* buffer, BassDword length, void* user)
{
    KdFileHandle* const file = static_cast<KdFileHandle*>(user);
    if (!file) {
        return 0;
    }
    return file->driver->read(file->driver, buffer, 1, length, file->native);
}

// Inferred name. BASS FILELENPROC: tell, seek to the end, tell, seek back;
// the high dword of the result is always 0. RET 4.
// FUNCTION: VC1 0x0045e8b0
static BassQword __stdcall MusicFileLength(void* user)
{
    KdFileHandle* const file = static_cast<KdFileHandle*>(user);
    __int64 position;
    __int64 end;

    if (!file) {
        return 0;
    }
    position = file->driver->tell(file->driver, file->native);
    file->driver->seek(file->driver, file->native, 0, 2);
    end = file->driver->tell(file->driver, file->native);
    file->driver->seek(file->driver, file->native, position, 0);
    return static_cast<BassDword>(end);
}

// Inferred name. BASS FILECLOSEPROC: driver close (+0x118), then frees the
// handle (no errno, unlike KdFclose). RET 4.
// FUNCTION: VC1 0x0045e940
static void __stdcall MusicFileClose(void* user)
{
    KdFileHandle* const file = static_cast<KdFileHandle*>(user);
    if (file) {
        file->driver->close(file->driver, file->native);
        free(file);
    }
}

// The BASS_FILEPROCS every music stream is created with.
// GLOBAL: VC1 0x004e8968
BassFileProcs g_musicFileProcs = {
    MusicFileClose,
    MusicFileLength,
    MusicFileRead,
    MusicFileSeek,
};

// EAX = this.
// FUNCTION: VC1 0x0045e970
KdMusic::KdMusic() : stream(0)
{
}

// ESI = out.
// FUNCTION: VC1 0x0045e9b0
void** CreateKdMusic(void** out)
{
    KdMusic* object;
    IAbstract* face;
    IUnknown* queried;

    object = static_cast<KdMusic*>(malloc(sizeof(KdMusic)));
    if (!object) {
        queried = 0;
    }
    else {
        new (object) KdMusic();
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

// STRING: VC1 0x004d66ac
static const char kMusicLoaded[] = "Loaded %s (freq: %d origres: %d chans: %d ctype: %d flags: %x)";

// RET 4. this = ILoadable. Frees the previous stream and opens name as a
// BASS user-file stream; the stream owns the file from then on. When
// BASS_ChannelGetInfo fails the file is closed again (KdFclose) although the
// stream still holds it.
// FUNCTION: VC1 0x0045ea30
unsigned char KdMusic::Load(const char* name)
{
    KdFileHandle* file;
    BassChannelInfo info;

    if (stream) {
        BASS_StreamFree(stream);
    }
    file = static_cast<KdFileHandle*>(OpenKdFile(name, kReadBinary));
    if (!file) {
        return 0;
    }
    stream = BASS_StreamCreateFileUser(0, 0, &g_musicFileProcs, file);
    if (!stream) {
        file->driver->close(file->driver, file->native);
        free(file);
        return 0;
    }
    if (!BASS_ChannelGetInfo(stream, &info)) {
        KdFclose(file);
        return 0;
    }
    HostLog(&g_kdNameMusic, kMusicLoaded, name, info.freq, info.origres, info.chans,
        info.ctype, info.flags);
    return 1;
}

// RET 8. IScriptObject vtable+0x0c. Method table 0x004ee4d0 (guard bit
// 0x004ee4e0, atexit 0x004caf50). The ISound slots bind through vcall thunks
// (0x0041c390 Play ... 0x0041c820 IsPaused).
// FUNCTION: VC1 0x0045eed0
unsigned char KdMusic::Script0c(const char* name, SquirrelObject* out)
{
    static ScriptMethodTable methods;

    if (methods.Empty()) {
        methods[kScriptPlay] = CreateScriptMethod<KdMusic>(&KdMusic::Play);
        methods[kScriptStop] = CreateScriptMethod<KdMusic>(&KdMusic::Stop);
        methods[kScriptPause] = CreateScriptMethod<KdMusic>(&KdMusic::Pause);
        methods[kScriptResume] = CreateScriptMethod<KdMusic>(&KdMusic::Resume);
        methods[kScriptSetPosition] = CreateScriptMethod<KdMusic>(&KdMusic::SetPosition);
        methods[kScriptGetPosition] = CreateScriptMethod<KdMusic>(&KdMusic::GetPosition);
        methods[kScriptSetVolume] = CreateScriptMethod<KdMusic>(&KdMusic::SetVolume);
        methods[kScriptSetLoop] = CreateScriptMethod<KdMusic>(&KdMusic::SetLoop);
        methods[kScriptIsActive] = CreateScriptMethod<KdMusic>(&KdMusic::IsActive);
        methods[kScriptIsPaused] = CreateScriptMethod<KdMusic>(&KdMusic::IsPaused);
    }
    return methods.Find(name, out);
}

}
