#ifndef VC1_KDCORE_WINDOW_H
#define VC1_KDCORE_WINDOW_H

#include "kdcore/native.h"
#include "kdcore/platform.h"

namespace vc1 {

extern const char kKdWindowClass[]; // Win32 class name for single-instance lookup.

// Inferred name. kdCreateWindow block (0x004961b0), the window's GWL_USERDATA.
// SIZE 0x18
struct KdWindow
{
    void* userptr;       // +0x00 event userptr; the block itself
    HWND hwnd;           // +0x04
    LPARAM lastPointer;  // +0x08 last WM_MOUSEMOVE/timer position
    unsigned pad0c;      // +0x0c
    int width;           // +0x10 client size for WM_GETMINMAXINFO
    int height;          // +0x14
};

// Inferred name. The window procedure's input message for TranslateInput:
// kind 0 key down, 1 key up, 2 char, 3 button down, 4 button up, 5 move,
// 6 wheel.
struct KdWinInput
{
    int kind;
    WPARAM wparam;
    LPARAM lparam;
};

void TranslateInput(const KdWindow* window, const KdWinInput* input, KdEvent* event);

WNDCLASSA* GetKdWindowClass(); // 0x00496020

}
#endif
