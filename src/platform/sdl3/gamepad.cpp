#include "platform/sdl3/gamepad.h"
#include "host/platform.h"
#include "host/window.h"
#include <SDL3/SDL.h>

namespace vc1 {
// Transitional window backend: the existing KD window owns the event userptr.
// SDL3 input deliberately knows no HWND, WinMM JOYINFOEX, or D3D interfaces.
namespace {
SDL_Gamepad* gamepad = 0;
unsigned previousButtons = 0;
unsigned previousHat[4] = { 0, 0, 0, 0 };

// Stable SDL semantic order, rather than platform-specific joystick numbering.
const SDL_GamepadButton buttons[] = {
    SDL_GAMEPAD_BUTTON_SOUTH, SDL_GAMEPAD_BUTTON_EAST,
    SDL_GAMEPAD_BUTTON_WEST, SDL_GAMEPAD_BUTTON_NORTH,
    SDL_GAMEPAD_BUTTON_LEFT_SHOULDER, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER,
    SDL_GAMEPAD_BUTTON_BACK, SDL_GAMEPAD_BUTTON_START,
    SDL_GAMEPAD_BUTTON_LEFT_STICK, SDL_GAMEPAD_BUTTON_RIGHT_STICK,
    SDL_GAMEPAD_BUTTON_GUIDE, SDL_GAMEPAD_BUTTON_MISC1
};
const SDL_GamepadButton hatButtons[] = {
    SDL_GAMEPAD_BUTTON_DPAD_UP, SDL_GAMEPAD_BUTTON_DPAD_LEFT,
    SDL_GAMEPAD_BUTTON_DPAD_RIGHT, SDL_GAMEPAD_BUTTON_DPAD_DOWN
};

void PostButton(unsigned key, unsigned pressed)
{
    KdEvent* event = AllocEvent();
    if (!event) return;
    event->type = 0x38;
    event->userptr = g_kdWindowMixer ? *static_cast<void**>(g_kdWindowMixer) : 0;
    event->data[0] = key;
    event->data[1] = 0;
    event->data[2] = pressed;
    event->data[3] = 0;
    QueueEvent(event);
}
}

bool InitSdlGamepad()
{
    if (!SDL_InitSubSystem(SDL_INIT_GAMEPAD)) return false;
    // KD consumes polled edges, so don't accumulate unused SDL input events.
    SDL_SetGamepadEventsEnabled(false);
    SDL_SetJoystickEventsEnabled(false);
    return true;
}

void ShutdownSdlGamepad()
{
    if (gamepad) SDL_CloseGamepad(gamepad);
    gamepad = 0;
    previousButtons = 0;
    for (unsigned i = 0; i < 4; ++i) previousHat[i] = 0;
    SDL_QuitSubSystem(SDL_INIT_GAMEPAD);
}

void PollGamepad()
{
    SDL_PumpEvents();
    SDL_UpdateGamepads();
    if (gamepad && !SDL_GamepadConnected(gamepad)) {
        SDL_CloseGamepad(gamepad);
        gamepad = 0;
    }
    if (!gamepad) {
        int count = 0;
        SDL_JoystickID* ids = SDL_GetGamepads(&count);
        for (int i = 0; ids && i < count && !gamepad; ++i)
            gamepad = SDL_OpenGamepad(ids[i]);
        SDL_free(ids);
    }

    unsigned currentButtons = 0;
    for (unsigned i = 0; i < sizeof(buttons) / sizeof(buttons[0]); ++i) {
        const unsigned pressed = gamepad && SDL_GetGamepadButton(gamepad, buttons[i]);
        if (pressed) currentButtons |= 1u << i;
        if (((previousButtons >> i) & 1u) != pressed) PostButton(0x9001 + i, pressed);
    }
    previousButtons = currentButtons;
    for (unsigned i = 0; i < 4; ++i) {
        const unsigned pressed = gamepad && SDL_GetGamepadButton(gamepad, hatButtons[i]);
        if (previousHat[i] != pressed) PostButton(0x8002 + i, pressed);
        previousHat[i] = pressed;
    }
}
}
