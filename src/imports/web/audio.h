#ifndef VC1_IMPORTS_WEB_AUDIO_H
#define VC1_IMPORTS_WEB_AUDIO_H

#include <SDL3_mixer/SDL_mixer.h>

// The BASS mixer of the web build (audio.cpp): mixed on a thread of its own
// into an AudioWorklet, instead of SDL's output on the page's main thread.
MIX_Mixer* vc1_web_audio_open();
// Stops the mixing thread; the caller then destroys the mixer.
void vc1_web_audio_close();

#endif
