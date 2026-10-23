// bass.dll (BASS 2.4) over SDL3_mixer: the BASS_* functions the sound driver,
// music and sound effects call (declared in kd/sound/bass_api.h).
//
// A BASS channel is an SDL_mixer track. Samples keep their own tracks, up to
// the sample's channel limit. Attribute slides (volume, pan, frequency) are
// advanced by the message pump (user32.cpp) and on every query.
#include "sdl3.h"
#include "kd/sound/bass_api.h"

#include <SDL3_mixer/SDL_mixer.h>

#include <algorithm>
#include <map>
#include <vector>

namespace {

struct Fade
{
    float from;
    float to;
    Uint64 begin;
    Uint64 end;     // 0 when no slide is running
    bool stop;      // a slide to a negative volume stops the channel at the end
};

struct Channel
{
    MIX_Track* track = 0;
    MIX_Audio* audio = 0;
    SDL_AudioSpec format = {};
    unsigned sample = 0;    // owning sample, 0 for a stream
    unsigned flags = 0;
    bool ownsAudio = false; // streams own their audio, sample channels share it
    float attrs[4] = {0, 44100, 1, 0}; // indexed by BASS_ATTRIB_*
    Fade fades[4] = {};
};

struct Sample
{
    MIX_Audio* audio;
    BassSample info;
    std::vector<unsigned> channels;
};

MIX_Mixer* mixer;
unsigned nextHandle = 1;
int lastError;
std::map<unsigned, Channel> channels;
std::map<unsigned, Sample> samples;
std::vector<unsigned> resumeAfterMasterPause;
bool outputPaused;

Channel* Find(unsigned handle)
{
    std::map<unsigned, Channel>::iterator found = channels.find(handle);
    return found == channels.end() ? 0 : &found->second;
}

Sample* FindSample(unsigned handle)
{
    std::map<unsigned, Sample>::iterator found = samples.find(handle);
    return found == samples.end() ? 0 : &found->second;
}

bool Apply(Channel& channel, unsigned attr, float value)
{
    channel.attrs[attr] = value;
    switch (attr) {
    case BASS_ATTRIB_VOL:
        return MIX_SetTrackGain(channel.track, value);
    case BASS_ATTRIB_FREQ:
        return MIX_SetTrackFrequencyRatio(channel.track, value / channel.format.freq);
    case BASS_ATTRIB_PAN: {
        const MIX_StereoGains gains = {value > 0 ? 1 - value : 1, value < 0 ? 1 + value : 1};
        return MIX_SetTrackStereo(channel.track, &gains);
    }
    default:
        return false;
    }
}

void Update(Channel& channel)
{
    const Uint64 now = SDL_GetTicks();
    for (unsigned attr = 1; attr <= 3; ++attr) {
        Fade& fade = channel.fades[attr];
        if (!fade.end) {
            continue;
        }
        const float t = now >= fade.end ? 1.0f : float(now - fade.begin) / float(fade.end - fade.begin);
        Apply(channel, attr, fade.from + (fade.to - fade.from) * t);
        if (t >= 1) {
            fade.end = 0;
            if (fade.stop) {
                MIX_StopTrack(channel.track, 0);
            }
        }
    }
}

unsigned NewChannel(MIX_Audio* audio, unsigned sample, bool owns)
{
    MIX_Track* track = MIX_CreateTrack(mixer);
    if (!track || !MIX_SetTrackAudio(track, audio)) {
        if (track) {
            MIX_DestroyTrack(track);
        }
        return 0;
    }
    const unsigned handle = nextHandle++;
    Channel& channel = channels[handle];
    channel.track = track;
    channel.audio = audio;
    channel.sample = sample;
    channel.ownsAudio = owns;
    MIX_GetAudioFormat(audio, &channel.format);
    channel.attrs[BASS_ATTRIB_FREQ] = float(channel.format.freq);
    return handle;
}

void FreeChannel(unsigned handle)
{
    std::map<unsigned, Channel>::iterator found = channels.find(handle);
    if (found == channels.end()) {
        return;
    }
    MIX_DestroyTrack(found->second.track);
    if (found->second.ownsAudio) {
        MIX_DestroyAudio(found->second.audio);
    }
    channels.erase(found);
}

// A BASS user file (the game's KD file) as an SDL stream.
struct UserFile
{
    BassFileProcs procs;
    void* user;
    Sint64 position;
};

Sint64 SDLCALL FileSize(void* data)
{
    UserFile* file = static_cast<UserFile*>(data);
    return file->procs.length(file->user);
}

Sint64 SDLCALL FileSeek(void* data, Sint64 offset, SDL_IOWhence whence)
{
    UserFile* file = static_cast<UserFile*>(data);
    Sint64 base = 0;
    if (whence == SDL_IO_SEEK_CUR) {
        base = file->position;
    }
    else if (whence == SDL_IO_SEEK_END) {
        base = FileSize(data);
    }
    const Sint64 next = base + offset;
    if (next < 0 || !file->procs.seek(next, file->user)) {
        return -1;
    }
    file->position = next;
    return next;
}

size_t SDLCALL FileRead(void* data, void* buffer, size_t size, SDL_IOStatus* status)
{
    UserFile* file = static_cast<UserFile*>(data);
    const size_t read = file->procs.read(buffer, static_cast<BassDword>(size), file->user);
    file->position += read;
    *status = read < size ? SDL_IO_STATUS_EOF : SDL_IO_STATUS_READY;
    return read;
}

bool SDLCALL FileClose(void* data)
{
    UserFile* file = static_cast<UserFile*>(data);
    file->procs.close(file->user);
    delete file;
    return true;
}

// 16-bit PCM bytes, the unit of the BASS position functions.
BassQword FrameBytes(const Channel& channel)
{
    return BassQword(channel.format.channels) * 2;
}

}

namespace sdl3 {

void UpdateAudio()
{
    for (std::map<unsigned, Channel>::iterator i = channels.begin(); i != channels.end(); ++i) {
        Update(i->second);
    }
}

}

extern "C" {

int __stdcall BASS_Init(int device, BassDword freq, BassDword flags, void* window, const void* guid)
{
    (void)device;
    (void)freq;
    (void)flags;
    (void)window;
    (void)guid;
    if (mixer) {
        return 1;
    }
    if (!sdl3::Init(SDL_INIT_AUDIO) || !MIX_Init()) {
        lastError = -1;
        return 0;
    }
    mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, 0);
    outputPaused = false;
    lastError = mixer ? 0 : -1;
    return mixer != 0;
}

int __stdcall BASS_Free()
{
    while (!channels.empty()) {
        FreeChannel(channels.begin()->first);
    }
    for (std::map<unsigned, Sample>::iterator i = samples.begin(); i != samples.end(); ++i) {
        MIX_DestroyAudio(i->second.audio);
    }
    samples.clear();
    if (mixer) {
        MIX_DestroyMixer(mixer);
    }
    mixer = 0;
    resumeAfterMasterPause.clear();
    outputPaused = false;
    MIX_Quit();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    return 1;
}

// SDL3_mixer 3.2.4, in BASS's version layout.
BassDword __stdcall BASS_GetVersion()
{
    return (3u << 24) | (2u << 16) | (4u << 8);
}

int __stdcall BASS_ErrorGetCode()
{
    return lastError;
}

BassDword __stdcall BASS_GetDevice()
{
    return 1;
}

int __stdcall BASS_GetDeviceInfo(BassDword device, BassDeviceInfo* info)
{
    (void)device;
    info->name = "SDL3 default playback device";
    info->driver = "SDL3_mixer";
    info->flags = 1;
    return 1;
}

// Resumes the channels BASS_Pause paused.
int __stdcall BASS_Start()
{
    if (!mixer) {
        return 0;
    }
    for (size_t i = 0; i < resumeAfterMasterPause.size(); ++i) {
        Channel* channel = Find(resumeAfterMasterPause[i]);
        if (channel && MIX_TrackPaused(channel->track)) {
            MIX_ResumeTrack(channel->track);
        }
    }
    resumeAfterMasterPause.clear();
    outputPaused = false;
    return 1;
}

int __stdcall BASS_Stop()
{
    if (!mixer) {
        return 0;
    }
    resumeAfterMasterPause.clear();
    outputPaused = true;
    return MIX_StopAllTracks(mixer, 0);
}

int __stdcall BASS_Pause()
{
    if (!mixer) {
        return 0;
    }
    if (outputPaused) {
        return 1;
    }
    resumeAfterMasterPause.clear();
    for (std::map<unsigned, Channel>::iterator i = channels.begin(); i != channels.end(); ++i) {
        if (MIX_TrackPlaying(i->second.track)) {
            resumeAfterMasterPause.push_back(i->first);
        }
    }
    outputPaused = true;
    return MIX_PauseAllTracks(mixer);
}

float __stdcall BASS_GetVolume()
{
    return mixer ? MIX_GetMixerGain(mixer) : 0;
}

int __stdcall BASS_SetVolume(float volume)
{
    return mixer && MIX_SetMixerGain(mixer, volume);
}

BassDword __stdcall BASS_SampleLoad(int mem, const void* data, BassQword offset, BassDword length,
    BassDword max, BassDword flags)
{
    if (!mixer) {
        return 0;
    }
    MIX_Audio* audio = 0;
    if (mem) {
        SDL_IOStream* io = SDL_IOFromConstMem(static_cast<const char*>(data) + offset, length);
        audio = MIX_LoadAudio_IO(mixer, io, true, true);
    }
    else {
        audio = MIX_LoadAudio(mixer, static_cast<const char*>(data), true);
    }
    if (!audio) {
        lastError = -1;
        return 0;
    }
    const unsigned handle = nextHandle++;
    Sample& sample = samples[handle];
    sample.audio = audio;
    sample.info = BassSample();
    SDL_AudioSpec format;
    MIX_GetAudioFormat(audio, &format);
    sample.info.freq = format.freq;
    sample.info.volume = 1;
    sample.info.flags = flags;
    sample.info.chans = format.channels;
    sample.info.origres = 16;
    sample.info.max = max ? max : 1;
    sample.info.length = static_cast<BassDword>(MIX_GetAudioDuration(audio) * format.channels * 2);
    return handle;
}

int __stdcall BASS_SampleFree(BassDword handle)
{
    Sample* sample = FindSample(handle);
    if (!sample) {
        return 0;
    }
    for (size_t i = 0; i < sample->channels.size(); ++i) {
        FreeChannel(sample->channels[i]);
    }
    MIX_DestroyAudio(sample->audio);
    samples.erase(handle);
    return 1;
}

int __stdcall BASS_SampleGetInfo(BassDword handle, BassSample* info)
{
    Sample* sample = FindSample(handle);
    if (!sample) {
        return 0;
    }
    *info = sample->info;
    return 1;
}

int __stdcall BASS_SampleSetInfo(BassDword handle, const BassSample* info)
{
    Sample* sample = FindSample(handle);
    if (!sample) {
        return 0;
    }
    sample->info = *info;
    return 1;
}

// An idle channel of the sample, a new one below the limit, or (unless only a
// new one will do) the oldest.
BassDword __stdcall BASS_SampleGetChannel(BassDword handle, int onlyNew)
{
    Sample* sample = FindSample(handle);
    if (!sample) {
        return 0;
    }
    for (size_t i = 0; i < sample->channels.size(); ++i) {
        MIX_Track* track = Find(sample->channels[i])->track;
        if (!MIX_TrackPlaying(track) && !MIX_TrackPaused(track)) {
            return sample->channels[i];
        }
    }
    if (sample->channels.size() >= sample->info.max) {
        return onlyNew ? 0 : sample->channels.front();
    }
    const unsigned channelHandle = NewChannel(sample->audio, handle, false);
    if (!channelHandle) {
        return 0;
    }
    sample->channels.push_back(channelHandle);
    Channel& channel = *Find(channelHandle);
    channel.flags = sample->info.flags;
    Apply(channel, BASS_ATTRIB_VOL, sample->info.volume);
    Apply(channel, BASS_ATTRIB_PAN, sample->info.pan);
    Apply(channel, BASS_ATTRIB_FREQ, float(sample->info.freq));
    return channelHandle;
}

BassDword __stdcall BASS_SampleGetChannels(BassDword handle, BassDword* out)
{
    Sample* sample = FindSample(handle);
    if (!sample) {
        return 0;
    }
    if (out) {
        for (size_t i = 0; i < sample->channels.size(); ++i) {
            out[i] = sample->channels[i];
        }
    }
    return static_cast<BassDword>(sample->channels.size());
}

BassDword __stdcall BASS_StreamCreateFileUser(BassDword system, BassDword flags,
    const BassFileProcs* procs, void* user)
{
    (void)system;
    UserFile* file = new UserFile();
    file->procs = *procs;
    file->user = user;
    file->position = 0;
    SDL_IOStreamInterface stream;
    SDL_INIT_INTERFACE(&stream);
    stream.size = FileSize;
    stream.seek = FileSeek;
    stream.read = FileRead;
    stream.close = FileClose;
    SDL_IOStream* io = SDL_OpenIO(&stream, file);
    if (!io) {
        FileClose(file);
        return 0;
    }
    MIX_Audio* audio = MIX_LoadAudio_IO(mixer, io, false, true);
    if (!audio) {
        lastError = -1;
        return 0;
    }
    const unsigned handle = NewChannel(audio, 0, true);
    if (!handle) {
        MIX_DestroyAudio(audio);
        return 0;
    }
    Find(handle)->flags = flags;
    return handle;
}

int __stdcall BASS_StreamFree(BassDword handle)
{
    if (!Find(handle)) {
        return 0;
    }
    FreeChannel(handle);
    return 1;
}

int __stdcall BASS_ChannelPlay(BassDword handle, int restart)
{
    Channel* channel = Find(handle);
    if (!channel) {
        return 0;
    }
    if (outputPaused) {
        lastError = 9; // BASS_ERROR_START
        return 0;
    }
    if (!restart && MIX_TrackPaused(channel->track)) {
        return MIX_ResumeTrack(channel->track);
    }
    if (!restart && MIX_TrackPlaying(channel->track)) {
        return 1;
    }
    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER,
        (channel->flags & BASS_SAMPLE_LOOP) ? -1 : 0);
    if (!restart) {
        SDL_SetNumberProperty(props, MIX_PROP_PLAY_START_FRAME_NUMBER,
            MIX_GetTrackPlaybackPosition(channel->track));
    }
    const bool ok = MIX_PlayTrack(channel->track, props);
    SDL_DestroyProperties(props);
    return ok;
}

int __stdcall BASS_ChannelStop(BassDword handle)
{
    Channel* channel = Find(handle);
    return channel && MIX_StopTrack(channel->track, 0);
}

int __stdcall BASS_ChannelPause(BassDword handle)
{
    resumeAfterMasterPause.erase(
        std::remove(resumeAfterMasterPause.begin(), resumeAfterMasterPause.end(), handle),
        resumeAfterMasterPause.end());
    Channel* channel = Find(handle);
    return channel && MIX_PauseTrack(channel->track);
}

BassDword __stdcall BASS_ChannelIsActive(BassDword handle)
{
    Channel* channel = Find(handle);
    if (!channel) {
        return BASS_ACTIVE_STOPPED;
    }
    if (MIX_TrackPaused(channel->track)) {
        return BASS_ACTIVE_PAUSED;
    }
    return MIX_TrackPlaying(channel->track) ? BASS_ACTIVE_PLAYING : BASS_ACTIVE_STOPPED;
}

BassDword __stdcall BASS_ChannelFlags(BassDword handle, BassDword flags, BassDword mask)
{
    Channel* channel = Find(handle);
    if (!channel) {
        return 0xffffffff;
    }
    channel->flags = (channel->flags & ~mask) | (flags & mask);
    MIX_SetTrackLoops(channel->track, (channel->flags & BASS_SAMPLE_LOOP) ? -1 : 0);
    return channel->flags;
}

int __stdcall BASS_ChannelGetAttribute(BassDword handle, BassDword attr, float* value)
{
    Channel* channel = Find(handle);
    if (!channel || attr < 1 || attr > 3) {
        return 0;
    }
    Update(*channel);
    *value = channel->attrs[attr];
    return 1;
}

int __stdcall BASS_ChannelSetAttribute(BassDword handle, BassDword attr, float value)
{
    Channel* channel = Find(handle);
    if (!channel || attr < 1 || attr > 3) {
        return 0;
    }
    channel->fades[attr].end = 0;
    return Apply(*channel, attr, value);
}

int __stdcall BASS_ChannelSlideAttribute(BassDword handle, BassDword attr, float value,
    BassDword ms)
{
    Channel* channel = Find(handle);
    if (!channel || attr < 1 || attr > 3) {
        return 0;
    }
    Update(*channel);
    if (!ms) {
        return BASS_ChannelSetAttribute(handle, attr, value);
    }
    const Uint64 now = SDL_GetTicks();
    const bool stop = attr == BASS_ATTRIB_VOL && value < 0;
    Fade& fade = channel->fades[attr];
    fade.from = channel->attrs[attr];
    fade.to = stop ? 0.0f : value;
    fade.begin = now;
    fade.end = now + ms;
    fade.stop = stop;
    return 1;
}

int __stdcall BASS_ChannelIsSliding(BassDword handle, BassDword attr)
{
    Channel* channel = Find(handle);
    if (!channel) {
        return 0;
    }
    Update(*channel);
    if (attr == 0) {
        return channel->fades[1].end || channel->fades[2].end || channel->fades[3].end;
    }
    return attr <= 3 && channel->fades[attr].end;
}

int __stdcall BASS_ChannelGetInfo(BassDword handle, BassChannelInfo* info)
{
    Channel* channel = Find(handle);
    if (!channel) {
        return 0;
    }
    *info = BassChannelInfo();
    info->freq = channel->format.freq;
    info->chans = channel->format.channels;
    info->flags = channel->flags;
    info->origres = 16;
    info->ctype = 0x10005; // a stream type
    info->sample = channel->sample;
    return 1;
}

BassQword __stdcall BASS_ChannelSeconds2Bytes(BassDword handle, double seconds)
{
    Channel* channel = Find(handle);
    return channel ? static_cast<BassQword>(seconds * channel->format.freq) * FrameBytes(*channel) : 0;
}

double __stdcall BASS_ChannelBytes2Seconds(BassDword handle, BassQword position)
{
    Channel* channel = Find(handle);
    return channel ? double(position) / (channel->format.freq * FrameBytes(*channel)) : 0;
}

int __stdcall BASS_ChannelSetPosition(BassDword handle, BassQword position, BassDword mode)
{
    (void)mode;
    Channel* channel = Find(handle);
    return channel && MIX_SetTrackPlaybackPosition(channel->track, position / FrameBytes(*channel));
}

BassQword __stdcall BASS_ChannelGetPosition(BassDword handle, BassDword mode)
{
    (void)mode;
    Channel* channel = Find(handle);
    return channel ? MIX_GetTrackPlaybackPosition(channel->track) * FrameBytes(*channel) : 0;
}

BassQword __stdcall BASS_ChannelGetLength(BassDword handle, BassDword mode)
{
    (void)mode;
    Channel* channel = Find(handle);
    return channel ? MIX_GetAudioDuration(channel->audio) * FrameBytes(*channel) : 0;
}

}
