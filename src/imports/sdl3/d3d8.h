#ifndef VC1_IMPORTS_SDL3_D3D8_H
#define VC1_IMPORTS_SDL3_D3D8_H

// d3d8.dll over SDL3: the D3D8 subset the recovered display code calls
// (d3d8.cpp). Native objects are COM-shaped so the recovered vtable calls
// (__stdcall, this pushed) run unchanged; slots the game does not use abort
// with their index. Vertices (FVF XYZ | DIFFUSE | TEX1) are transformed by
// the view and projection matrices on the CPU and drawn with SDL geometry.
#include "sdl3.h"
#include <vector>

namespace vc1 {

struct IUnknown;

// IDirect3DTexture8 (one level, A8R8G8B8 or X8R8G8B8). pixels mirror the SDL
// texture for locks and the texture-plus-diffuse color operation.
struct SdlTexture
{
    void** vtbl;
    unsigned refs;
    SDL_Texture* texture;
    unsigned width, height, format;
    bool target;
    std::vector<unsigned> pixels;
    std::vector<unsigned> lock;
    SDL_Rect region;
    bool writable;
    SdlTexture* prev;
    SdlTexture* next;
};

// IDirect3DSurface8: level 0 of a texture (owner), or a system-memory image
// surface from CreateImageSurface (pixels).
struct SdlSurface
{
    void** vtbl;
    unsigned refs;
    SdlTexture* owner;
    unsigned width, height;
    std::vector<unsigned> pixels;
};

}

#endif
