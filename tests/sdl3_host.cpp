// SDL3 integration test: use a virtual controller, without game data or hardware.
#include "host/platform.h"
#include "platform/sdl3/gamepad.h"
#include <SDL3/SDL.h>
#include <cstdio>
#include <cstdlib>
#include <vector>

namespace {
std::vector<vc1::KdEvent> events;
void Require(bool condition, const char* message)
{
    if (!condition) {
        std::fprintf(stderr, "%s: %s\n", message, SDL_GetError());
        std::exit(1);
    }
}
void ExpectKey(unsigned key, unsigned pressed)
{
    Require(events.size() == 1, "expected one KD input edge");
    const vc1::KdEvent& e = events[0];
    Require(e.type == 0x38 && e.data[0] == key && e.data[2] == pressed && e.data[3] == 0,
        "unexpected KD key or press state");
    Require(e.userptr == reinterpret_cast<void*>(0x1234), "window userptr lost");
    events.clear();
}
}

namespace vc1 {
double g_nsPerTick;
void* g_kdWindowMixer;
KdEvent* AllocEvent() { return static_cast<KdEvent*>(std::calloc(1, sizeof(KdEvent))); }
int QueueEvent(KdEvent* event)
{
    if (!event) return -1;
    events.push_back(*event);
    std::free(event);
    return 0;
}
}

int main()
{
    SDL_SetHint(SDL_HINT_GAMECONTROLLER_IGNORE_DEVICES_EXCEPT, "0x1209/0x0001");
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
    Require(vc1::InitSdlGamepad(), "SDL3 gamepad initialization failed");
    vc1::g_nsPerTick = 1.0e9 / static_cast<double>(SDL_GetPerformanceFrequency());
    const Uint64 before = SDL_GetPerformanceCounter();
    SDL_Delay(2);
    const Uint64 after = SDL_GetPerformanceCounter();
    Require(after > before && vc1::g_nsPerTick > 0, "non-monotonic SDL clock");

    SDL_VirtualJoystickDesc desc;
    SDL_INIT_INTERFACE(&desc);
    desc.type = SDL_JOYSTICK_TYPE_GAMEPAD;
    desc.vendor_id = 0x1209;
    desc.product_id = 1;
    desc.naxes = SDL_GAMEPAD_AXIS_COUNT;
    desc.nbuttons = SDL_GAMEPAD_BUTTON_COUNT;
    desc.axis_mask = (1u << SDL_GAMEPAD_AXIS_COUNT) - 1;
    desc.button_mask = (1u << SDL_GAMEPAD_BUTTON_COUNT) - 1;
    desc.name = "VC1 virtual test controller";
    SDL_JoystickID id = SDL_AttachVirtualJoystick(&desc);
    Require(id != 0, "virtual controller attach failed");
    SDL_Joystick* joystick = SDL_OpenJoystick(id);
    Require(joystick != 0, "virtual controller open failed");
    void* userptr = reinterpret_cast<void*>(0x1234);
    vc1::g_kdWindowMixer = &userptr;
    vc1::PollGamepad();
    Require(events.empty(), "idle controller posted input");

    Require(SDL_SetJoystickVirtualButton(joystick, SDL_GAMEPAD_BUTTON_SOUTH, true), "press failed");
    vc1::PollGamepad();
    ExpectKey(0x9001, 1);
    vc1::PollGamepad();
    Require(events.empty(), "held button repeated an edge");
    Require(SDL_SetJoystickVirtualButton(joystick, SDL_GAMEPAD_BUTTON_SOUTH, false), "release failed");
    vc1::PollGamepad();
    ExpectKey(0x9001, 0);
    Require(SDL_SetJoystickVirtualButton(joystick, SDL_GAMEPAD_BUTTON_DPAD_UP, true), "dpad press failed");
    vc1::PollGamepad();
    ExpectKey(0x8002, 1);
    Require(SDL_DetachVirtualJoystick(id), "virtual disconnect failed");
    vc1::PollGamepad();
    ExpectKey(0x8002, 0);
    vc1::PollGamepad();
    Require(events.empty(), "disconnected controller repeated input");
    SDL_CloseJoystick(joystick);
    vc1::ShutdownSdlGamepad();
    SDL_Quit();
    std::puts("SDL3 clock, button edges, dpad, disconnect releases and KD userptr passed");
    return 0;
}
