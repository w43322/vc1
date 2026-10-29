#ifndef VC1_KD_DISPLAY_D3D8_API_H
#define VC1_KD_DISPLAY_D3D8_API_H

// The parts of the Direct3D 8 API (d3d8.dll) the original calls, after the
// DirectX 8 SDK's d3d8.h, which VC9 does not ship. The interfaces are COM:
// pure virtual __stdcall methods in the SDK's vtable order, so a call is the
// same "push args, push this, call [vtbl+slot*4]" the original makes; every
// method up to the last one the game calls is declared to keep the slots.
// Parameters the game reads field by field have their SDK structures; the
// blobs it only passes through (caps, adapter identifier, present
// parameters) stay raw. The SDL3 build implements them in
// src/imports/sdl3/d3d8.cpp.

// HWND, RECT and POINT by their SDK tags, so this header needs no <windows.h>
// (whose macros, such as DrawText, would rename game methods).
struct HWND__;
struct tagRECT;
struct tagPOINT;

namespace vc1 {

typedef long D3dResult; // HRESULT

// D3DLOCKED_RECT
struct D3dLockedRect
{
    int pitch;  // 0x00
    void* bits; // 0x04
};

// D3DSURFACE_DESC
struct D3dSurfaceDesc
{
    unsigned format;      // 0x00
    unsigned type;        // 0x04
    unsigned usage;       // 0x08
    unsigned pool;        // 0x0c
    unsigned size;        // 0x10
    unsigned multiSample; // 0x14
    unsigned width;       // 0x18
    unsigned height;      // 0x1c
};

// D3DDISPLAYMODE
struct D3dDisplayMode
{
    unsigned width;   // 0x00
    unsigned height;  // 0x04
    unsigned refresh; // 0x08
    unsigned format;  // 0x0c
};

// D3DVIEWPORT8
struct D3dViewport
{
    unsigned x;      // 0x00
    unsigned y;      // 0x04
    unsigned width;  // 0x08
    unsigned height; // 0x0c
    float minZ;      // 0x10
    float maxZ;      // 0x14
};

struct IDirect3DDevice8;

// The COM IUnknown of the D3D objects (not vc1::IUnknown, which is
// __thiscall).
struct D3dUnknown
{
    virtual D3dResult __stdcall QueryInterface(const void* iid, void** out) = 0; // 0x00
    virtual unsigned long __stdcall AddRef() = 0;                               // 0x04
    virtual unsigned long __stdcall Release() = 0;                              // 0x08
};

// IDirect3DResource8
struct IDirect3DResource8 : D3dUnknown
{
    virtual D3dResult __stdcall GetDevice(IDirect3DDevice8** device) = 0;          // 0x0c
    virtual D3dResult __stdcall SetPrivateData(const void* guid, const void* data,
        unsigned long size, unsigned long flags) = 0;                               // 0x10
    virtual D3dResult __stdcall GetPrivateData(const void* guid, void* data,
        unsigned long* size) = 0;                                                   // 0x14
    virtual D3dResult __stdcall FreePrivateData(const void* guid) = 0;              // 0x18
    virtual unsigned long __stdcall SetPriority(unsigned long priority) = 0;        // 0x1c
    virtual unsigned long __stdcall GetPriority() = 0;                              // 0x20
    virtual void __stdcall PreLoad() = 0;                                           // 0x24
    virtual unsigned __stdcall GetType() = 0;                                       // 0x28
};

// IDirect3DBaseTexture8
struct IDirect3DBaseTexture8 : IDirect3DResource8
{
    virtual unsigned long __stdcall SetLOD(unsigned long lod) = 0; // 0x2c
    virtual unsigned long __stdcall GetLOD() = 0;                  // 0x30
    virtual unsigned long __stdcall GetLevelCount() = 0;           // 0x34
};

struct IDirect3DSurface8;

// IDirect3DTexture8
struct IDirect3DTexture8 : IDirect3DBaseTexture8
{
    virtual D3dResult __stdcall GetLevelDesc(unsigned level, D3dSurfaceDesc* desc) = 0;    // 0x38
    virtual D3dResult __stdcall GetSurfaceLevel(unsigned level,
        IDirect3DSurface8** surface) = 0;                                                   // 0x3c
    virtual D3dResult __stdcall LockRect(unsigned level, D3dLockedRect* locked,
        const tagRECT* rect, unsigned long flags) = 0;                                         // 0x40
    virtual D3dResult __stdcall UnlockRect(unsigned level) = 0;                             // 0x44
};

// IDirect3DSurface8
struct IDirect3DSurface8 : D3dUnknown
{
    virtual D3dResult __stdcall GetDevice(IDirect3DDevice8** device) = 0;            // 0x0c
    virtual D3dResult __stdcall SetPrivateData(const void* guid, const void* data,
        unsigned long size, unsigned long flags) = 0;                                 // 0x10
    virtual D3dResult __stdcall GetPrivateData(const void* guid, void* data,
        unsigned long* size) = 0;                                                     // 0x14
    virtual D3dResult __stdcall FreePrivateData(const void* guid) = 0;                // 0x18
    virtual D3dResult __stdcall GetContainer(const void* iid, void** container) = 0;  // 0x1c
    virtual D3dResult __stdcall GetDesc(D3dSurfaceDesc* desc) = 0;                    // 0x20
    virtual D3dResult __stdcall LockRect(D3dLockedRect* locked, const tagRECT* rect,
        unsigned long flags) = 0;                                                     // 0x24
    virtual D3dResult __stdcall UnlockRect() = 0;                                     // 0x28
};

// IDirect3DVertexBuffer8
struct IDirect3DVertexBuffer8 : IDirect3DResource8
{
    virtual D3dResult __stdcall Lock(unsigned offset, unsigned size, unsigned char** data,
        unsigned long flags) = 0;                                       // 0x2c
    virtual D3dResult __stdcall Unlock() = 0;                           // 0x30
};

// IDirect3DIndexBuffer8
struct IDirect3DIndexBuffer8 : IDirect3DResource8
{
    virtual D3dResult __stdcall Lock(unsigned offset, unsigned size, unsigned char** data,
        unsigned long flags) = 0;                                       // 0x2c
    virtual D3dResult __stdcall Unlock() = 0;                           // 0x30
};

// IDirect3DDevice8, up to SetIndices.
struct IDirect3DDevice8 : D3dUnknown
{
    virtual D3dResult __stdcall TestCooperativeLevel() = 0;                          // 0x0c
    virtual unsigned __stdcall GetAvailableTextureMem() = 0;                         // 0x10
    virtual D3dResult __stdcall ResourceManagerDiscardBytes(unsigned long bytes) = 0; // 0x14
    virtual D3dResult __stdcall GetDirect3D(void** d3d) = 0;                         // 0x18
    // D3DCAPS8, 0xd4 bytes.
    virtual D3dResult __stdcall GetDeviceCaps(unsigned char* caps) = 0;              // 0x1c
    virtual D3dResult __stdcall GetDisplayMode(D3dDisplayMode* mode) = 0;            // 0x20
    virtual D3dResult __stdcall GetCreationParameters(void* parameters) = 0;         // 0x24
    virtual D3dResult __stdcall SetCursorProperties(unsigned x, unsigned y,
        IDirect3DSurface8* bitmap) = 0;                                              // 0x28
    virtual void __stdcall SetCursorPosition(int x, int y, unsigned long flags) = 0; // 0x2c
    virtual int __stdcall ShowCursor(int show) = 0;                                  // 0x30
    virtual D3dResult __stdcall CreateAdditionalSwapChain(void* parameters,
        void** swapChain) = 0;                                                       // 0x34
    // D3DPRESENT_PARAMETERS.
    virtual D3dResult __stdcall Reset(void* parameters) = 0;                         // 0x38
    virtual D3dResult __stdcall Present(const tagRECT* source, const tagRECT* destination,
        HWND__* window, const void* dirtyRegion) = 0;                                   // 0x3c
    virtual D3dResult __stdcall GetBackBuffer(unsigned index, unsigned type,
        IDirect3DSurface8** buffer) = 0;                                             // 0x40
    virtual D3dResult __stdcall GetRasterStatus(void* status) = 0;                   // 0x44
    virtual void __stdcall SetGammaRamp(unsigned long flags, const void* ramp) = 0;  // 0x48
    virtual void __stdcall GetGammaRamp(void* ramp) = 0;                             // 0x4c
    virtual D3dResult __stdcall CreateTexture(unsigned width, unsigned height, unsigned levels,
        unsigned long usage, unsigned format, unsigned pool,
        IDirect3DTexture8** texture) = 0;                                            // 0x50
    virtual D3dResult __stdcall CreateVolumeTexture(unsigned width, unsigned height,
        unsigned depth, unsigned levels, unsigned long usage, unsigned format,
        unsigned pool, void** texture) = 0;                                          // 0x54
    virtual D3dResult __stdcall CreateCubeTexture(unsigned edge, unsigned levels,
        unsigned long usage, unsigned format, unsigned pool, void** texture) = 0;    // 0x58
    virtual D3dResult __stdcall CreateVertexBuffer(unsigned length, unsigned long usage,
        unsigned long fvf, unsigned pool, IDirect3DVertexBuffer8** buffer) = 0;      // 0x5c
    virtual D3dResult __stdcall CreateIndexBuffer(unsigned length, unsigned long usage,
        unsigned format, unsigned pool, IDirect3DIndexBuffer8** buffer) = 0;         // 0x60
    virtual D3dResult __stdcall CreateRenderTarget(unsigned width, unsigned height,
        unsigned format, unsigned multiSample, int lockable,
        IDirect3DSurface8** surface) = 0;                                            // 0x64
    virtual D3dResult __stdcall CreateDepthStencilSurface(unsigned width, unsigned height,
        unsigned format, unsigned multiSample, IDirect3DSurface8** surface) = 0;     // 0x68
    virtual D3dResult __stdcall CreateImageSurface(unsigned width, unsigned height,
        unsigned format, IDirect3DSurface8** surface) = 0;                           // 0x6c
    virtual D3dResult __stdcall CopyRects(IDirect3DSurface8* source, const tagRECT* rects,
        unsigned count, IDirect3DSurface8* destination, const tagPOINT* points) = 0;    // 0x70
    virtual D3dResult __stdcall UpdateTexture(IDirect3DBaseTexture8* source,
        IDirect3DBaseTexture8* destination) = 0;                                     // 0x74
    virtual D3dResult __stdcall GetFrontBuffer(IDirect3DSurface8* destination) = 0;  // 0x78
    virtual D3dResult __stdcall SetRenderTarget(IDirect3DSurface8* target,
        IDirect3DSurface8* depthStencil) = 0;                                        // 0x7c
    virtual D3dResult __stdcall GetRenderTarget(IDirect3DSurface8** target) = 0;     // 0x80
    virtual D3dResult __stdcall GetDepthStencilSurface(IDirect3DSurface8** surface) = 0; // 0x84
    virtual D3dResult __stdcall BeginScene() = 0;                                    // 0x88
    virtual D3dResult __stdcall EndScene() = 0;                                      // 0x8c
    virtual D3dResult __stdcall Clear(unsigned long count, const void* rects,
        unsigned long flags, unsigned long color, float z, unsigned long stencil) = 0; // 0x90
    // D3DMATRIX, 16 floats.
    virtual D3dResult __stdcall SetTransform(unsigned state, const float* matrix) = 0; // 0x94
    virtual D3dResult __stdcall GetTransform(unsigned state, float* matrix) = 0;     // 0x98
    virtual D3dResult __stdcall MultiplyTransform(unsigned state, const float* matrix) = 0; // 0x9c
    virtual D3dResult __stdcall SetViewport(const D3dViewport* viewport) = 0;        // 0xa0
    virtual D3dResult __stdcall GetViewport(D3dViewport* viewport) = 0;              // 0xa4
    virtual D3dResult __stdcall SetMaterial(const void* material) = 0;               // 0xa8
    virtual D3dResult __stdcall GetMaterial(void* material) = 0;                     // 0xac
    virtual D3dResult __stdcall SetLight(unsigned long index, const void* light) = 0; // 0xb0
    virtual D3dResult __stdcall GetLight(unsigned long index, void* light) = 0;      // 0xb4
    virtual D3dResult __stdcall LightEnable(unsigned long index, int enable) = 0;    // 0xb8
    virtual D3dResult __stdcall GetLightEnable(unsigned long index, int* enable) = 0; // 0xbc
    virtual D3dResult __stdcall SetClipPlane(unsigned long index, const float* plane) = 0; // 0xc0
    virtual D3dResult __stdcall GetClipPlane(unsigned long index, float* plane) = 0; // 0xc4
    virtual D3dResult __stdcall SetRenderState(unsigned state, unsigned long value) = 0; // 0xc8
    virtual D3dResult __stdcall GetRenderState(unsigned state, unsigned long* value) = 0; // 0xcc
    virtual D3dResult __stdcall BeginStateBlock() = 0;                               // 0xd0
    virtual D3dResult __stdcall EndStateBlock(unsigned long* token) = 0;             // 0xd4
    virtual D3dResult __stdcall ApplyStateBlock(unsigned long token) = 0;            // 0xd8
    virtual D3dResult __stdcall CaptureStateBlock(unsigned long token) = 0;          // 0xdc
    virtual D3dResult __stdcall DeleteStateBlock(unsigned long token) = 0;           // 0xe0
    virtual D3dResult __stdcall CreateStateBlock(unsigned type, unsigned long* token) = 0; // 0xe4
    virtual D3dResult __stdcall SetClipStatus(const void* status) = 0;               // 0xe8
    virtual D3dResult __stdcall GetClipStatus(void* status) = 0;                     // 0xec
    virtual D3dResult __stdcall GetTexture(unsigned long stage,
        IDirect3DBaseTexture8** texture) = 0;                                        // 0xf0
    virtual D3dResult __stdcall SetTexture(unsigned long stage,
        IDirect3DBaseTexture8* texture) = 0;                                         // 0xf4
    virtual D3dResult __stdcall GetTextureStageState(unsigned long stage, unsigned type,
        unsigned long* value) = 0;                                                   // 0xf8
    virtual D3dResult __stdcall SetTextureStageState(unsigned long stage, unsigned type,
        unsigned long value) = 0;                                                    // 0xfc
    virtual D3dResult __stdcall ValidateDevice(unsigned long* passes) = 0;           // 0x100
    virtual D3dResult __stdcall GetInfo(unsigned long id, void* info,
        unsigned long size) = 0;                                                     // 0x104
    virtual D3dResult __stdcall SetPaletteEntries(unsigned palette, const void* entries) = 0; // 0x108
    virtual D3dResult __stdcall GetPaletteEntries(unsigned palette, void* entries) = 0; // 0x10c
    virtual D3dResult __stdcall SetCurrentTexturePalette(unsigned palette) = 0;      // 0x110
    virtual D3dResult __stdcall GetCurrentTexturePalette(unsigned* palette) = 0;     // 0x114
    virtual D3dResult __stdcall DrawPrimitive(unsigned type, unsigned start,
        unsigned count) = 0;                                                         // 0x118
    virtual D3dResult __stdcall DrawIndexedPrimitive(unsigned type, unsigned minIndex,
        unsigned vertexCount, unsigned startIndex, unsigned primitiveCount) = 0;     // 0x11c
    virtual D3dResult __stdcall DrawPrimitiveUP(unsigned type, unsigned count,
        const void* vertices, unsigned stride) = 0;                                  // 0x120
    virtual D3dResult __stdcall DrawIndexedPrimitiveUP(unsigned type, unsigned minIndex,
        unsigned vertexCount, unsigned primitiveCount, const void* indices,
        unsigned indexFormat, const void* vertices, unsigned stride) = 0;            // 0x124
    virtual D3dResult __stdcall ProcessVertices(unsigned start, unsigned destination,
        unsigned count, IDirect3DVertexBuffer8* buffer, unsigned long flags) = 0;    // 0x128
    virtual D3dResult __stdcall CreateVertexShader(const unsigned long* declaration,
        const unsigned long* function, unsigned long* handle,
        unsigned long usage) = 0;                                                    // 0x12c
    virtual D3dResult __stdcall SetVertexShader(unsigned long handle) = 0;           // 0x130
    virtual D3dResult __stdcall GetVertexShader(unsigned long* handle) = 0;          // 0x134
    virtual D3dResult __stdcall DeleteVertexShader(unsigned long handle) = 0;        // 0x138
    virtual D3dResult __stdcall SetVertexShaderConstant(unsigned long reg,
        const void* data, unsigned long count) = 0;                                  // 0x13c
    virtual D3dResult __stdcall GetVertexShaderConstant(unsigned long reg, void* data,
        unsigned long count) = 0;                                                    // 0x140
    virtual D3dResult __stdcall GetVertexShaderDeclaration(unsigned long handle,
        void* data, unsigned long* size) = 0;                                        // 0x144
    virtual D3dResult __stdcall GetVertexShaderFunction(unsigned long handle,
        void* data, unsigned long* size) = 0;                                        // 0x148
    virtual D3dResult __stdcall SetStreamSource(unsigned stream,
        IDirect3DVertexBuffer8* data, unsigned stride) = 0;                          // 0x14c
    virtual D3dResult __stdcall GetStreamSource(unsigned stream,
        IDirect3DVertexBuffer8** data, unsigned* stride) = 0;                        // 0x150
    virtual D3dResult __stdcall SetIndices(IDirect3DIndexBuffer8* indices,
        unsigned baseVertex) = 0;                                                    // 0x154
};

// IDirect3D8
struct IDirect3D8 : D3dUnknown
{
    virtual D3dResult __stdcall RegisterSoftwareDevice(void* initialize) = 0;          // 0x0c
    virtual unsigned __stdcall GetAdapterCount() = 0;                                  // 0x10
    // D3DADAPTER_IDENTIFIER8, 0x42c bytes.
    virtual D3dResult __stdcall GetAdapterIdentifier(unsigned adapter, unsigned long flags,
        void* identifier) = 0;                                                         // 0x14
    virtual unsigned __stdcall GetAdapterModeCount(unsigned adapter) = 0;              // 0x18
    virtual D3dResult __stdcall EnumAdapterModes(unsigned adapter, unsigned mode,
        D3dDisplayMode* displayMode) = 0;                                              // 0x1c
    virtual D3dResult __stdcall GetAdapterDisplayMode(unsigned adapter,
        D3dDisplayMode* displayMode) = 0;                                              // 0x20
    virtual D3dResult __stdcall CheckDeviceType(unsigned adapter, unsigned type,
        unsigned displayFormat, unsigned backBufferFormat, int windowed) = 0;          // 0x24
    virtual D3dResult __stdcall CheckDeviceFormat(unsigned adapter, unsigned type,
        unsigned adapterFormat, unsigned long usage, unsigned resourceType,
        unsigned checkFormat) = 0;                                                     // 0x28
    virtual D3dResult __stdcall CheckDeviceMultiSampleType(unsigned adapter, unsigned type,
        unsigned surfaceFormat, int windowed, unsigned multiSample) = 0;               // 0x2c
    virtual D3dResult __stdcall CheckDepthStencilMatch(unsigned adapter, unsigned type,
        unsigned adapterFormat, unsigned targetFormat, unsigned depthFormat) = 0;      // 0x30
    virtual D3dResult __stdcall GetDeviceCaps(unsigned adapter, unsigned type,
        unsigned char* caps) = 0;                                                      // 0x34
    virtual void* __stdcall GetAdapterMonitor(unsigned adapter) = 0;                   // 0x38 HMONITOR
    virtual D3dResult __stdcall CreateDevice(unsigned adapter, unsigned type, HWND__* focus,
        unsigned long behavior, void* parameters, IDirect3DDevice8** device) = 0;      // 0x3c
};

}

extern "C" vc1::IDirect3D8* __stdcall Direct3DCreate8(unsigned sdkVersion);

#endif
