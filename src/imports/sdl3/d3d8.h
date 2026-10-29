#ifndef VC1_IMPORTS_SDL3_D3D8_H
#define VC1_IMPORTS_SDL3_D3D8_H

// d3d8.dll over SDL3: the D3D8 subset the recovered display code calls
// (d3d8.cpp). The objects implement the interfaces of kd/display/d3d8_api.h;
// methods the game never calls end the program with their name. Vertices
// (FVF XYZ | DIFFUSE | TEX1) are transformed by the view and projection
// matrices on the CPU and drawn with SDL geometry.
#include "sdl3.h"
#include "kd/display/d3d8_api.h"

#include <vector>

namespace vc1 {

// Logs the D3D8 method the game called but this layer does not implement and
// aborts: its caller's expectations are unknown.
[[noreturn]] void D3dUnsupported(const char* method);

// QueryInterface and the IDirect3DResource8 methods, none of which the game
// calls, for the texture and buffer objects.
template <class Interface>
class SdlResource : public Interface
{
public:
    D3dResult __stdcall QueryInterface(const void*, void**) override
    {
        D3dUnsupported("QueryInterface");
    }
    D3dResult __stdcall GetDevice(IDirect3DDevice8**) override
    {
        D3dUnsupported("IDirect3DResource8::GetDevice");
    }
    D3dResult __stdcall SetPrivateData(const void*, const void*, unsigned long,
        unsigned long) override
    {
        D3dUnsupported("IDirect3DResource8::SetPrivateData");
    }
    D3dResult __stdcall GetPrivateData(const void*, void*, unsigned long*) override
    {
        D3dUnsupported("IDirect3DResource8::GetPrivateData");
    }
    D3dResult __stdcall FreePrivateData(const void*) override
    {
        D3dUnsupported("IDirect3DResource8::FreePrivateData");
    }
    unsigned long __stdcall SetPriority(unsigned long) override
    {
        D3dUnsupported("IDirect3DResource8::SetPriority");
    }
    unsigned long __stdcall GetPriority() override
    {
        D3dUnsupported("IDirect3DResource8::GetPriority");
    }
    void __stdcall PreLoad() override
    {
        D3dUnsupported("IDirect3DResource8::PreLoad");
    }
    unsigned __stdcall GetType() override
    {
        D3dUnsupported("IDirect3DResource8::GetType");
    }
};

// IDirect3DTexture8 (one level, A8R8G8B8 or X8R8G8B8). pixels mirror the SDL
// texture for locks and the texture-plus-diffuse color operation.
class SdlTexture final : public SdlResource<IDirect3DTexture8>
{
public:
    unsigned refs = 1;
    SDL_Texture* texture = 0;
    unsigned width = 0, height = 0, format = 0;
    bool target = false;
    std::vector<unsigned> pixels;
    std::vector<unsigned> lock;
    SDL_Rect region = {0, 0, 0, 0};
    bool writable = false;
    SdlTexture* prev = 0;
    SdlTexture* next = 0;

    unsigned long __stdcall AddRef() override;
    unsigned long __stdcall Release() override;
    unsigned long __stdcall SetLOD(unsigned long) override
    {
        D3dUnsupported("IDirect3DTexture8::SetLOD");
    }
    unsigned long __stdcall GetLOD() override
    {
        D3dUnsupported("IDirect3DTexture8::GetLOD");
    }
    unsigned long __stdcall GetLevelCount() override
    {
        D3dUnsupported("IDirect3DTexture8::GetLevelCount");
    }
    D3dResult __stdcall GetLevelDesc(unsigned level, D3dSurfaceDesc* desc) override;
    D3dResult __stdcall GetSurfaceLevel(unsigned level, IDirect3DSurface8** surface) override;
    D3dResult __stdcall LockRect(unsigned level, D3dLockedRect* locked, const tagRECT* rect,
        unsigned long flags) override;
    D3dResult __stdcall UnlockRect(unsigned level) override;
};

// IDirect3DSurface8: level 0 of a texture (owner), or a system-memory image
// surface from CreateImageSurface (pixels).
class SdlSurface final : public IDirect3DSurface8
{
public:
    unsigned refs = 1;
    SdlTexture* owner = 0;
    unsigned width = 0, height = 0;
    std::vector<unsigned> pixels;

    D3dResult __stdcall QueryInterface(const void*, void**) override
    {
        D3dUnsupported("QueryInterface");
    }
    unsigned long __stdcall AddRef() override;
    unsigned long __stdcall Release() override;
    D3dResult __stdcall GetDevice(IDirect3DDevice8**) override
    {
        D3dUnsupported("IDirect3DSurface8::GetDevice");
    }
    D3dResult __stdcall SetPrivateData(const void*, const void*, unsigned long,
        unsigned long) override
    {
        D3dUnsupported("IDirect3DSurface8::SetPrivateData");
    }
    D3dResult __stdcall GetPrivateData(const void*, void*, unsigned long*) override
    {
        D3dUnsupported("IDirect3DSurface8::GetPrivateData");
    }
    D3dResult __stdcall FreePrivateData(const void*) override
    {
        D3dUnsupported("IDirect3DSurface8::FreePrivateData");
    }
    D3dResult __stdcall GetContainer(const void*, void**) override
    {
        D3dUnsupported("IDirect3DSurface8::GetContainer");
    }
    D3dResult __stdcall GetDesc(D3dSurfaceDesc*) override
    {
        D3dUnsupported("IDirect3DSurface8::GetDesc");
    }
    D3dResult __stdcall LockRect(D3dLockedRect* locked, const tagRECT* rect,
        unsigned long flags) override;
    D3dResult __stdcall UnlockRect() override;
};

}

#endif
