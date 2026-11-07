// The web build's sound output. The original's BASS mixes on a thread of its
// own; SDL's web backend instead mixes on the page's main thread, in a
// ScriptProcessorNode callback that is late whenever a frame or a load runs
// long, and the sound stutters. Here a thread mixes the SDL_mixer mixer that
// bass.cpp plays on into a ring of samples in the module's shared memory, and
// an AudioWorklet on the browser's audio thread plays them. The mixing thread
// runs only SDL_mixer: bass.cpp hands it streams already read into memory.
#include "audio.h"

#include <SDL3/SDL.h>
#include <emscripten.h>
#include <string.h>
#include <unistd.h>

namespace {

// Interleaved stereo float frames. A power of two, so positions wrap with a
// mask; about 85 ms at 48 kHz, which is also the most the output lags.
const unsigned kRingFrames = 4096;
const unsigned kChunkFrames = 256;

struct Ring
{
    unsigned positions[2]; // frames written, frames played (the worklet's)
    float samples[kRingFrames * 2];
};

Ring g_ring;
MIX_Mixer* g_mixer;
SDL_Thread* g_thread;
SDL_AtomicInt g_running;

int SDLCALL MixThread(void*)
{
    float chunk[kChunkFrames * 2];
    while (SDL_GetAtomicInt(&g_running)) {
        const unsigned written = __atomic_load_n(&g_ring.positions[0], __ATOMIC_ACQUIRE);
        const unsigned played = __atomic_load_n(&g_ring.positions[1], __ATOMIC_ACQUIRE);
        if (kRingFrames - (written - played) < kChunkFrames) {
            usleep(2000);
            continue;
        }
        if (MIX_Generate(g_mixer, chunk, sizeof(chunk)) < 0) {
            memset(chunk, 0, sizeof(chunk));
        }
        for (unsigned i = 0; i < kChunkFrames; ++i) {
            const unsigned at = ((written + i) & (kRingFrames - 1)) * 2;
            g_ring.samples[at] = chunk[i * 2];
            g_ring.samples[at + 1] = chunk[i * 2 + 1];
        }
        __atomic_store_n(&g_ring.positions[0], written + kChunkFrames, __ATOMIC_RELEASE);
    }
    return 0;
}

// Creates the page's AudioContext and the worklet that plays the ring;
// returns the context's sample rate. The context starts suspended until the
// player first clicks or types (autoplay policy). Module.vc1Audio is there
// for the devtools: the context and the ring's written / played frames.
EM_JS(int, StartOutput, (unsigned* positions, float* samples, unsigned frames), {
    var Context = window.AudioContext || window.webkitAudioContext;
    var context = new Context({ latencyHint: 'interactive' });
    Module.vc1Audio = { context: context, positions: new Int32Array(HEAPU8.buffer, positions, 2) };
    var processor = [
        'class Vc1Output extends AudioWorkletProcessor {',
        '  constructor(options) {',
        '    super();',
        '    var o = options.processorOptions;',
        '    this.positions = new Int32Array(o.memory, o.positions, 2);',
        '    this.samples = new Float32Array(o.memory, o.samples, o.frames * 2);',
        '    this.mask = o.frames - 1;',
        '  }',
        '  process(inputs, outputs) {',
        '    var left = outputs[0][0], right = outputs[0][1] || left, n = left.length;',
        '    var played = Atomics.load(this.positions, 1);',
        '    var ready = Math.min(n, (Atomics.load(this.positions, 0) - played) | 0);',
        '    for (var i = 0; i < ready; i++) {',
        '      var at = ((played + i) & this.mask) * 2;',
        '      left[i] = this.samples[at];',
        '      right[i] = this.samples[at + 1];',
        '    }',
        '    left.fill(0, ready);',
        '    right.fill(0, ready);',
        '    Atomics.store(this.positions, 1, (played + ready) | 0);',
        '    return true;',
        '  }',
        '}',
        'registerProcessor("vc1-output", Vc1Output);'
    ].join('\n');
    var url = URL.createObjectURL(new Blob([processor], { type: 'text/javascript' }));
    context.audioWorklet.addModule(url).then(function () {
        var node = new AudioWorkletNode(context, 'vc1-output', {
            numberOfInputs: 0,
            outputChannelCount: [2],
            processorOptions: { memory: HEAPU8.buffer, positions: positions, samples: samples, frames: frames }
        });
        node.connect(context.destination);
    }, function (error) {
        console.error('Sound output', error);
    });
    var resume = function () {
        if (context.state === 'suspended' && !Module.vc1AudioHeld) {
            context.resume();
        }
    };
    window.addEventListener('pointerdown', resume, true);
    window.addEventListener('keydown', resume, true);
    return context.sampleRate;
});

}

MIX_Mixer* vc1_web_audio_open()
{
    SDL_AudioSpec spec;
    spec.format = SDL_AUDIO_F32;
    spec.channels = 2;
    spec.freq = StartOutput(g_ring.positions, g_ring.samples, kRingFrames);
    g_mixer = MIX_CreateMixer(&spec);
    if (!g_mixer) {
        return 0;
    }
    SDL_SetAtomicInt(&g_running, 1);
    g_thread = SDL_CreateThread(MixThread, "vc1 sound", 0);
    if (!g_thread) {
        MIX_DestroyMixer(g_mixer);
        g_mixer = 0;
    }
    return g_mixer;
}

void vc1_web_audio_close()
{
    if (g_thread) {
        SDL_SetAtomicInt(&g_running, 0);
        SDL_WaitThread(g_thread, 0);
        g_thread = 0;
    }
    g_mixer = 0;
}
