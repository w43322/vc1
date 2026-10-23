// winmm.dll over SDL3: joystick 0 is the first SDL gamepad.
#include "sdl3.h"

namespace {

const MMRESULT kJoyUnplugged = 167;
SDL_Gamepad* gamepad;

// Buttons 1..12 in SDL's positional order.
const SDL_GamepadButton kButtons[] = {
    SDL_GAMEPAD_BUTTON_SOUTH, SDL_GAMEPAD_BUTTON_EAST,
    SDL_GAMEPAD_BUTTON_WEST, SDL_GAMEPAD_BUTTON_NORTH,
    SDL_GAMEPAD_BUTTON_LEFT_SHOULDER, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER,
    SDL_GAMEPAD_BUTTON_BACK, SDL_GAMEPAD_BUTTON_START,
    SDL_GAMEPAD_BUTTON_LEFT_STICK, SDL_GAMEPAD_BUTTON_RIGHT_STICK,
    SDL_GAMEPAD_BUTTON_GUIDE, SDL_GAMEPAD_BUTTON_MISC1
};

bool Open()
{
    if (!sdl3::Init(SDL_INIT_GAMEPAD)) {
        return false;
    }
    // The game polls; don't queue gamepad events nobody reads.
    SDL_SetGamepadEventsEnabled(false);
    SDL_SetJoystickEventsEnabled(false);
    SDL_UpdateGamepads();
    if (gamepad && !SDL_GamepadConnected(gamepad)) {
        SDL_CloseGamepad(gamepad);
        gamepad = 0;
    }
    if (!gamepad) {
        int count = 0;
        SDL_JoystickID* ids = SDL_GetGamepads(&count);
        for (int i = 0; ids && i < count && !gamepad; ++i) {
            gamepad = SDL_OpenGamepad(ids[i]);
        }
        SDL_free(ids);
    }
    return gamepad != 0;
}

// The D-pad as a point-of-view hat in hundredths of a degree.
DWORD Pov()
{
    const bool up = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_UP);
    const bool down = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_DOWN);
    const bool left = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_LEFT);
    const bool right = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT);
    if (up) {
        return right ? 4500 : left ? 31500 : 0;
    }
    if (down) {
        return right ? 13500 : left ? 22500 : 18000;
    }
    if (right) {
        return 9000;
    }
    if (left) {
        return 27000;
    }
    return 0xffff;
}

}

// PollGamepad never sets dwSize/dwFlags, which WinMM requires; they are not
// checked here, so SDL gamepads work.
MMRESULT WINAPI joyGetPosEx(UINT joystick, JOYINFOEX* info)
{
    if (joystick != 0 || !Open()) {
        return kJoyUnplugged;
    }
    info->dwButtons = 0;
    for (unsigned i = 0; i < sizeof(kButtons) / sizeof(kButtons[0]); ++i) {
        if (SDL_GetGamepadButton(gamepad, kButtons[i])) {
            info->dwButtons |= 1u << i;
        }
    }
    info->dwPOV = Pov();
    return JOYERR_NOERROR;
}
