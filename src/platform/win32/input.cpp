#include "platform/win32/window.h"

namespace vc1 {

// Last pointer position and buttons, mirrored from every pointer event.
// GLOBAL: VC1 0x004e90bc
int g_pointerX;
// GLOBAL: VC1 0x004e90c0
int g_pointerY;
// GLOBAL: VC1 0x004e90c4
unsigned g_pointerButtons;

// Keys that also post a G5 game key (0x1001-0x1009): {game key, KD key}.
// GLOBAL: VC1 0x004e9138
unsigned g_kdGameKeys[9][2] = {
    { 0x1001, 0x40000026 }, // up
    { 0x1002, 0x40000025 }, // left
    { 0x1003, 0x40000027 }, // right
    { 0x1004, 0x40000028 }, // down
    { 0x1005, 0x40000020 }, // space
    { 0x1006, 0x4000005a }, // Z
    { 0x1007, 0x40000058 }, // X
    { 0x1008, 0x40000043 }, // C
    { 0x1009, 0x40000056 }, // V
};

// EAX=input, EDI=event, cdecl window. Fills the window procedure's stack
// event: keys are KD_EVENT_INPUT (0x38) with the virtual key + 0x40000000,
// the press flag and the raw lparam; chars add 0x40000100; mouse messages
// are pointer events (0x39) with the client position. A remapped key posts a
// second event with the game key. Inferred name.
// FUNCTION: VC1 0x004badd0
void TranslateInput(const KdWindow* window, const KdWinInput* input, KdEvent* event)
{
    const int kind = input->kind;

    switch (kind) {
    case 0:
    case 1: {
        const unsigned key = static_cast<unsigned>(input->wparam) + 0x40000000;
        unsigned i;
        event->type = 0x38;
        event->data[0] = key;
        *reinterpret_cast<__int64*>(&event->data[2]) =
            (static_cast<__int64>(input->lparam) << 32) | static_cast<__int64>(kind == 0);
        for (i = 0; i < 9; ++i) {
            if (key == g_kdGameKeys[i][1]) {
                const unsigned game = g_kdGameKeys[i][0];
                KdEvent* const posted = AllocEvent();
                posted->type = 0x38;
                posted->userptr = window->userptr;
                posted->data[0] = game;
                posted->data[2] = event->data[2];
                posted->data[3] = event->data[3];
                QueueEvent(posted);
                return;
            }
        }
        break;
    }
    case 2: {
        CHAR ch = static_cast<CHAR>(input->wparam);
        WCHAR wide;
        MultiByteToWideChar(CP_ACP, 0, &ch, 1, &wide, 1);
        event->data[0] = static_cast<unsigned>(wide) + 0x40000100;
        event->data[2] = 2;
        event->type = 0x38;
        event->data[3] = static_cast<unsigned>(input->lparam);
        break;
    }
    case 3:
    case 4:
    case 5:
    case 6:
        event->type = 0x39;
        switch (kind) {
        case 3:
        case 4:
            g_pointerButtons = static_cast<unsigned>(input->wparam);
            event->data[1] = g_pointerButtons;
            event->data[0] = 0x4003;
            break;
        case 5:
            event->data[0] = 0x4001;
            event->data[1] = static_cast<unsigned>(input->wparam);
            break;
        case 6:
            event->data[0] = 0x4004;
            event->data[1] = static_cast<unsigned>(input->wparam);
            break;
        }
        g_pointerX = static_cast<short>(input->lparam);
        event->data[2] = static_cast<unsigned>(g_pointerX);
        g_pointerY = static_cast<short>(input->lparam >> 16);
        event->data[3] = static_cast<unsigned>(g_pointerY);
        break;
    }
}

}
