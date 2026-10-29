#include "kd/display/display.h"
#include "kd/display/effect.h"
#include "kd/display/graphics.h"
#include "kd/display/pyro.h"
#include "kd/display/render_target.h"
#include "kd/display/texture.h"
#include "kd/factory.h"

#include "kd/iface/iunknown.h"
#include "kd/iface/iid.h"
#include "kdcore/runtime.h"
#include "kdcore/native.h"
#include "kdcore/kd_stdio.h"
#include "kd/display/jpeg_stream.h"
#include "bindings/matrix4.h"
#include "bindings/matrix3.h"
#include "bindings/component.h"
#include "bindings/sqplus.h"

#include <tga.h>
#if VC1_MODS
#include "mod/mod_io.h"
#endif

#include <errno.h>
#include <new>
#include <stdlib.h>
#include <string.h>

#pragma function(memcpy)

// Direct3DCreate8, the d3d8.dll export (import thunk 0x00442e0e, library row
// in config/verified.csv), is declared in kd/display/d3d8_api.h.

namespace vc1 {

// GLOBAL: VC1 0x004ed54c
IAbstract* g_kdDisplay;
// GLOBAL: VC1 0x004ed550
HWND g_kdDisplayWindow;
// GLOBAL: VC1 0x004ed554
unsigned g_displayBusy;
// GLOBAL: VC1 0x004ed558
int* g_displayMode;
// GLOBAL: VC1 0x004ed55c
unsigned g_d3dPresentWindowed[13];
// GLOBAL: VC1 0x004ed5a4
unsigned g_d3dPresentFull[13];
// Windowed-mode frame rectangle (screen coordinates).
// GLOBAL: VC1 0x004ed590
RECT g_displayWindowRect;
// GLOBAL: VC1 0x004ed5a0
unsigned g_displayWindowStyle;
// Full-screen view rectangle (the desktop mode).
// GLOBAL: VC1 0x004ed5d8
RECT g_displayViewRect;
// GLOBAL: VC1 0x004ed5e8
unsigned g_displayViewStyle;
// GLOBAL: VC1 0x004ed5f0
IDirect3DDevice8* g_d3dDevice;
// GLOBAL: VC1 0x004ed5ec
IDirect3D8* g_d3dObject;
// Inferred name. CreateVertexBuffer(0x17700, D3DUSAGE_WRITEONLY, FVF 0x142).
// GLOBAL: VC1 0x004ed5f4
IDirect3DVertexBuffer8* g_d3dVertexBuffer;
// Inferred name. CreateIndexBuffer(0x2ee0, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16).
// GLOBAL: VC1 0x004ed5f8
IDirect3DIndexBuffer8* g_d3dIndexBuffer;
// GLOBAL: VC1 0x004ed5fc
IDirect3DSurface8* g_d3dBackBuffer;
// GLOBAL: VC1 0x004ed600
void* g_d3dChain;
// GLOBAL: VC1 0x004ed604
void* g_d3dLastMixer;
// GLOBAL: VC1 0x004ed608
TrackedTexture* g_d3dChain2;
// Inferred names for the 2D batch: locked vertex pointer, queued primitive
// count, primitive kind (2 lines, 3 triangles, 4 indexed quads), render-state
// flags and the bound D3D texture.
// GLOBAL: VC1 0x004ed60c
unsigned g_batchVertices;
// GLOBAL: VC1 0x004ed610
int g_batchCount;
// GLOBAL: VC1 0x004ed614
unsigned g_batchKind;
// GLOBAL: VC1 0x004ed618
unsigned g_batchFlags;
// GLOBAL: VC1 0x004ed61c
IDirect3DTexture8* g_batchTexture;
// GLOBAL: VC1 0x004e960c
Matrix3 g_displayMatrixInv;
// GLOBAL: VC1 0x004e98c0
float g_d3dView[16];
// GLOBAL: VC1 0x004e9900
float g_d3dProj[16];
// GLOBAL: VC1 0x004ed700
KdRef<IUnknown> g_displayScene;
// GLOBAL: VC1 0x004ed6ec
KdRef<IUnknown> g_graphicsQuery;
// GLOBAL: VC1 0x004ed628
PyroLibrary* g_displayPyro;
// GLOBAL: VC1 0x004ed65c
KdRef<IUnknown> g_kdQueryUnknown;
// GLOBAL: VC1 0x004ed6f0
KdRef<IUnknown> g_graphics;
// GLOBAL: VC1 0x004ed6f4
KdRef<IUnknown> g_graphicsHost;
// GLOBAL: VC1 0x004e74b6
bool g_displaySkipDevice;
// GLOBAL: VC1 0x004e8744
int g_displayWidth;
// GLOBAL: VC1 0x004e8998
int g_displayHeight;
// GLOBAL: VC1 0x004e899c
int g_displayPresentInterval;
// STRING: VC1 0x004d4c38
static const char kTexture[] = "Texture";
// STRING: VC1 0x004d4c4c
static const char kDisplay[] = "Display";
// GLOBAL: VC1 0x004d0630
const char* g_kdNameDisplay = kDisplay;
// GLOBAL: VC1 0x004d0638
const char* g_kdNameTexture = kTexture;
// STRING: VC1 0x004d4c28
static const char kRenderTarget[] = "RenderTarget";
// GLOBAL: VC1 0x004d063c
const char* g_kdNameRenderTarget = kRenderTarget;
// STRING: VC1 0x004d5db8
static const char kInitSize[] = "Init: %dx%d";
// STRING: VC1 0x004d5dc4
static const char kCantInit[] = "Can't init";
// STRING: VC1 0x004d0b4c
static const char kCantCreateRt[] = "Can't create render target for full screen";
// STRING: VC1 0x004d61ec
static const char kCantInitPyro[] = "Can't init Pyro";

// GLOBAL: VC1 0x004ee284
IUnknown* g_nullRenderTexture;
// GLOBAL: VC1 0x004ee288
unsigned g_nullRenderTextureGuard;

static void ReleaseNullRenderTexture();
static unsigned char DisplayQueryReady(KdRenderTargetNode* node);
static int UpdateWindowFrame();
static int FormatPresentInterval(unsigned format);
static const char* __stdcall DxGetErrorString(long hr);
static void DisplayD3dPow2Caps(unsigned* w, unsigned* h);
static void FlushBatch(char final);
static void DisplayD3DTeardown();
static void DisplayD3dSetBuffers(int right, int bottom, int left, int top);
static void DisplayD3dLogHresult(int hr, const char* message, char flag);
static unsigned char DisplayD3dRecreate();
static unsigned char DisplayD3dReset();
static unsigned char DisplayD3DInit();
static int ApplyWindowedMode(bool windowed);
static IUnknown** DisplayAssignTarget(void** out, IUnknown** slot);
float MinFloat(float a, float b);

IPyroDevice::~IPyroDevice()
{
}

// KdDisplayDevice() is inlined into KdDisplayBase() (0x004665c0).
KdDisplayDevice::KdDisplayDevice()
{
    g_displayPyro = 0;
}

// Inlined into ~KdDisplayBase (0x004664a0) after its vptr resets.
KdDisplayDevice::~KdDisplayDevice()
{
    g_kdDisplay = 0;
}

KdDisplay::~KdDisplay()
{
    if (target) {
        target->Release();
    }
    target = 0;
}

// ESI=&target, stdcall CreateRenderTarget out. QI IRenderTarget into the slot. RET 4.
// FUNCTION: VC1 0x00402290
static IUnknown** DisplayAssignTarget(void** out, IUnknown** slot)
{
    IUnknown* queried;

    if (*slot) {
        (*slot)->Release();
    }
    if (!*out) {
        *slot = 0;
        return slot;
    }
    queried = static_cast<IUnknown*>(
        static_cast<IUnknown*>(*out)->QueryInterface(&g_iidRenderTarget));
    *slot = queried;
    if (queried) {
        queried->AddRef();
    }
    return slot;
}

// cdecl two floats; returns min on the FPU stack.
// FUNCTION: VC1 0x00402410
__declspec(noinline) float MinFloat(float a, float b)
{
    if (a < b) {
        return a;
    }
    return b;
}

// EDI=this. malloc(0x64) KdGraphics, QI IAbstract (+0x48) then IGraphics into
// graphics. The Release after the AddRef is not null-checked in the original.
// FUNCTION: VC1 0x0041a460
IRenderTarget::IRenderTarget()
{
    KdGraphics* object;
    IUnknown* ident;
    IUnknown* queried;

    extra = 0;
    graphics = 0;
    object = static_cast<KdGraphics*>(malloc(sizeof(KdGraphics)));
    if (object) {
        new (object) KdGraphics();
    }
    if (!object) {
        ident = 0;
    }
    else {
        ident = static_cast<IUnknown*>(
            static_cast<IAbstract*>(object)->QueryInterface(&g_iidAbstract));
        if (ident) {
            ident->AddRef();
        }
    }
    ident->Release();
    if (graphics) {
        graphics->Release();
    }
    queried = static_cast<IUnknown*>(ident->QueryInterface(&g_iidGraphics));
    graphics = static_cast<IGraphics*>(queried);
    if (queried) {
        queried->AddRef();
    }
    ident->Release();
}

// this=IRenderTarget. RET 8. a = IGraphics** out, b = IUnknown** camera.
// SetView(ECX=b, EAX={0, 0, GetWidth(), GetHeight()}) on graphics.
// FUNCTION: VC1 0x0041a500
void* IRenderTarget::BeginFrame(void* a, void* b)
{
    IUnknown** out = static_cast<IUnknown**>(a);
    IUnknown* queried;
    Rect rect;

    if (!DisplayQueryReady(static_cast<KdRenderTargetNode*>(extra))) {
        if (!g_graphicsQuery) {
            *out = 0;
            return out;
        }
        queried = static_cast<IUnknown*>(g_graphicsQuery->QueryInterface(&g_iidGraphics));
        *out = queried;
        if (queried) {
            queried->AddRef();
        }
        return out;
    }

    rect.height = GetHeight();
    rect.x = 0;
    rect.y = 0;
    rect.width = GetWidth();
    if (static_cast<KdGraphics*>(graphics)->SetView(&rect, static_cast<IUnknown**>(b))) {
        queried = graphics;
        *out = queried;
        if (queried) {
            queried->AddRef();
        }
        return out;
    }
    if (!g_graphicsQuery) {
        *out = 0;
        return out;
    }
    queried = static_cast<IUnknown*>(g_graphicsQuery->QueryInterface(&g_iidGraphics));
    *out = queried;
    if (queried) {
        queried->AddRef();
    }
    return out;
}

// IRenderTarget vtable+0x10. Rebind the graphics scene, end and present
// (the same sequence as KdDisplayBase::EndFrame).
// FUNCTION: VC1 0x0041a600
void* IRenderTarget::EndFrame()
{
    void* result;

    static_cast<KdGraphics*>(graphics)->BindScene();
    FlushBatch(1);
    result = reinterpret_cast<void*>(g_d3dDevice->EndScene());
    if (g_d3dLastMixer == 0) {
        result = reinterpret_cast<void*>(g_d3dDevice->Present(0, 0, 0, 0));
    }
    return result;
}

// MOV EAX, 0x004d4c4c; RET. IScriptObject vtable+0x14.
// FUNCTION: VC1 0x0041a650
const char* KdDisplayBase::GetTypeName()
{
    return kDisplay;
}

// RET 0x10. this = adapter.
// FUNCTION: VC1 0x0041ac20
int PyroVertexBufferAdapter::Upload(const void* source, int offset, int size, int discard)
{
    int result;

    (void)discard;
    result = buffer->Lock();
    if (result == 0) {
        memcpy(static_cast<unsigned char*>(buffer->GetData()) + offset, source, size);
        result = buffer->Unlock();
    }
    return result;
}

// MOV EAX, 1; RET 0x14.
// FUNCTION: VC1 0x0041aca0
int IPyroDevice::CreateBuffer(PyroDeviceBuffer** out, int count, int stride, unsigned format,
    int flags)
{
    (void)out;
    (void)count;
    (void)stride;
    (void)format;
    (void)flags;
    return 1;
}

// RET 0x14. CreateBuffer into a local (the original reuses the last argument
// slot); on success *out is a PyroVertexBufferAdapter over it (null when the
// allocation fails, still returning 0).
// FUNCTION: VC1 0x0041acb0
int IPyroDevice::CreateVertexBuffer(PyroVertexBuffer** out, int count, int stride, unsigned format,
    int flags)
{
    PyroDeviceBuffer* inner;
    int result;

    result = CreateBuffer(&inner, count, stride, format, flags);
    if (result) {
        return result;
    }
    *out = new PyroVertexBufferAdapter(inner);
    return 0;
}

// MOV EAX, 1; RET 0x10.
// FUNCTION: VC1 0x0041ad10
int IPyroDevice::DrawQuadBuffer(PyroDeviceBuffer* buffer, int first, int count, int flags)
{
    (void)buffer;
    (void)first;
    (void)count;
    (void)flags;
    return 1;
}

// RET 0x10. The default draw of a CreateVertexBuffer result: the adapter's
// device buffer through DrawQuadBuffer.
// FUNCTION: VC1 0x0041ad20
int IPyroDevice::DrawQuads(PyroVertexBuffer* buffer, int first, int count, int flags)
{
    return DrawQuadBuffer(static_cast<PyroVertexBufferAdapter*>(buffer)->buffer, first, count,
        flags);
}

// RET 0x10. this = PyroVertexBuffer face (data at +4).
// FUNCTION: VC1 0x0041adc0
int KdPyroBuffer::Upload(const void* source, int offset, int size, int discard)
{
    (void)discard;
    memcpy(data + offset, source, size);
    return 0;
}

// ESI=this, RET 4. The by-value source is AddRef'd into the member, then
// released on return.
// FUNCTION: VC1 0x0041ae20
KdPyroTexture::KdPyroTexture(KdPtr<ITexture> source)
{
    ITexture* const value = source.ptr;

    if (value) {
        value->AddRef();
    }
    if (texture.ptr) {
        texture.ptr->Release();
    }
    texture = value;
}

// MOV EAX, [ECX+8]; RET.
// FUNCTION: VC1 0x0041fdc0
void* KdPyroBuffer::GetData()
{
    return data;
}

// stdcall RET 4. DirectX 8 SDK dxerr8.lib DXGetErrorString8A (no source in
// this tree): a switch over 350 HRESULTs returning each name, "Unknown"
// otherwise. The cases were read off the original's compare tree and jump
// tables (0x00442e14..0x0044432a, tables up to 0x00444680).
// FUNCTION: VC1 0x00442e14
static const char* __stdcall DxGetErrorString(long hr)
{
    switch (hr) {
    case static_cast<long>(0x00000000UL):
        // STRING: VC1 0x004cdb44
        return "S_OK";
    case static_cast<long>(0x00000001UL):
        // STRING: VC1 0x004cdb3c
        return "S_FALSE";
    case static_cast<long>(0x00000002UL):
        // STRING: VC1 0x004cdb24
        return "ERROR_FILE_NOT_FOUND";
    case static_cast<long>(0x00000003UL):
        // STRING: VC1 0x004cdb0c
        return "ERROR_PATH_NOT_FOUND";
    case static_cast<long>(0x00000004UL):
        // STRING: VC1 0x004cdaf0
        return "ERROR_TOO_MANY_OPEN_FILES";
    case static_cast<long>(0x00000005UL):
        // STRING: VC1 0x004cdadc
        return "ERROR_ACCESS_DENIED";
    case static_cast<long>(0x00000006UL):
        // STRING: VC1 0x004cdac4
        return "ERROR_INVALID_HANDLE";
    case static_cast<long>(0x00000008UL):
        // STRING: VC1 0x004cdaac
        return "ERROR_NOT_ENOUGH_MEMORY";
    case static_cast<long>(0x00000009UL):
        // STRING: VC1 0x004cda98
        return "ERROR_INVALID_BLOCK";
    case static_cast<long>(0x0000000aUL):
        // STRING: VC1 0x004cda80
        return "ERROR_BAD_ENVIRONMENT";
    case static_cast<long>(0x0000000bUL):
        // STRING: VC1 0x004cda6c
        return "ERROR_BAD_FORMAT";
    case static_cast<long>(0x0000000eUL):
        // STRING: VC1 0x004cda58
        return "ERROR_OUTOFMEMORY";
    case static_cast<long>(0x00150005UL):
        // STRING: VC1 0x004cda48
        return "DV_FULLDUPLEX";
    case static_cast<long>(0x0015000aUL):
        // STRING: VC1 0x004cda04
        return "DV_HALFDUPLEX";
    case static_cast<long>(0x00150010UL):
        // STRING: VC1 0x004cda14
        return "DV_PENDING";
    case static_cast<long>(0x0878000aUL):
        // STRING: VC1 0x004cda20
        return "DS_NO_VIRTUALIZATION";
    case static_cast<long>(0x08780014UL):
        // STRING: VC1 0x004cda38
        return "DS_INCOMPLETE";
    case static_cast<long>(0x08781091UL):
        // STRING: VC1 0x004cd9f0
        return "DMUS_S_PARTIALLOAD";
    case static_cast<long>(0x08781092UL):
        // STRING: VC1 0x004cd9bc
        return "DMUS_S_PARTIALDOWNLOAD";
    case static_cast<long>(0x08781200UL):
        // STRING: VC1 0x004cd9d4
        return "DMUS_S_REQUEUE";
    case static_cast<long>(0x08781201UL):
        // STRING: VC1 0x004cd9e4
        return "DMUS_S_FREE";
    case static_cast<long>(0x08781202UL):
        // STRING: VC1 0x004cd9b0
        return "DMUS_S_END";
    case static_cast<long>(0x08781210UL):
        // STRING: VC1 0x004cd90c
        return "DMUS_S_STRING_TRUNCATED";
    case static_cast<long>(0x08781211UL):
        // STRING: VC1 0x004cd924
        return "DMUS_S_LAST_TOOL";
    case static_cast<long>(0x08781212UL):
        // STRING: VC1 0x004cd938
        return "DMUS_S_OVER_CHORD";
    case static_cast<long>(0x08781213UL):
        // STRING: VC1 0x004cd94c
        return "DMUS_S_UP_OCTAVE";
    case static_cast<long>(0x08781214UL):
        // STRING: VC1 0x004cd960
        return "DMUS_S_DOWN_OCTAVE";
    case static_cast<long>(0x08781215UL):
        // STRING: VC1 0x004cd974
        return "DMUS_S_NOBUFFERCONTROL";
    case static_cast<long>(0x08781216UL):
        // STRING: VC1 0x004cd98c
        return "DMUS_S_GARBAGE_COLLECTED";
    case static_cast<long>(0x8000000aUL):
        // STRING: VC1 0x004cf880
        return "E_PENDING";
    case static_cast<long>(0x80004001UL):
        // STRING: VC1 0x004cf88c
        return "E_NOTIMPL";
    case static_cast<long>(0x80004002UL):
        // STRING: VC1 0x004cf898
        return "E_NOINTERFACE";
    case static_cast<long>(0x80004003UL):
        // STRING: VC1 0x004cf8a8
        return "E_POINTER";
    case static_cast<long>(0x80004004UL):
        // STRING: VC1 0x004cf8b4
        return "E_ABORT";
    case static_cast<long>(0x80004005UL):
        // STRING: VC1 0x004cf878
        return "E_FAIL";
    case static_cast<long>(0x8000ffffUL):
        // STRING: VC1 0x004cf828
        return "E_UNEXPECTED";
    case static_cast<long>(0x80040110UL):
        // STRING: VC1 0x004cf838
        return "CLASS_E_NOAGGREGATION";
    case static_cast<long>(0x80040154UL):
        // STRING: VC1 0x004cf850
        return "REGDB_E_CLASSNOTREG";
    case static_cast<long>(0x800401f0UL):
        // STRING: VC1 0x004cf864
        return "CO_E_NOTINITIALIZED";
    case static_cast<long>(0x800401f1UL):
        // STRING: VC1 0x004cf810
        return "CO_E_ALREADYINITIALIZED";
    case static_cast<long>(0x80040200UL):
        // STRING: VC1 0x004cf7f8
        return "DIERR_INSUFFICIENTPRIVS";
    case static_cast<long>(0x80040201UL):
        // STRING: VC1 0x004cf7e4
        return "DIERR_DEVICEFULL";
    case static_cast<long>(0x80040202UL):
        // STRING: VC1 0x004cf7d4
        return "DIERR_MOREDATA";
    case static_cast<long>(0x80040203UL):
        // STRING: VC1 0x004cf7c0
        return "DIERR_NOTDOWNLOADED";
    case static_cast<long>(0x80040204UL):
        // STRING: VC1 0x004cf7ac
        return "DIERR_HASEFFECTS";
    case static_cast<long>(0x80040205UL):
        // STRING: VC1 0x004cf790
        return "DIERR_NOTEXCLUSIVEACQUIRED";
    case static_cast<long>(0x80040206UL):
        // STRING: VC1 0x004cf778
        return "DIERR_INCOMPLETEEFFECT";
    case static_cast<long>(0x80040207UL):
        // STRING: VC1 0x004cf764
        return "DIERR_NOTBUFFERED";
    case static_cast<long>(0x80040208UL):
        // STRING: VC1 0x004cf750
        return "DIERR_EFFECTPLAYING";
    case static_cast<long>(0x80040209UL):
        // STRING: VC1 0x004cf740
        return "DIERR_UNPLUGGED";
    case static_cast<long>(0x8004020aUL):
        // STRING: VC1 0x004cf72c
        return "DIERR_REPORTFULL";
    case static_cast<long>(0x8004020bUL):
        // STRING: VC1 0x004cf6c8
        return "DIERR_MAPFILEFAIL";
    case static_cast<long>(0x80040300UL):
        // STRING: VC1 0x004cf6dc
        return "DIERR_DRIVERFIRST";
    case static_cast<long>(0x80040301UL):
        // STRING: VC1 0x004cf6f0
        return "DIERR_DRIVERFIRST+1";
    case static_cast<long>(0x80040302UL):
        // STRING: VC1 0x004cf704
        return "DIERR_DRIVERFIRST+2";
    case static_cast<long>(0x80040303UL):
        // STRING: VC1 0x004cf718
        return "DIERR_DRIVERFIRST+3";
    case static_cast<long>(0x80040304UL):
        // STRING: VC1 0x004cf6b4
        return "DIERR_DRIVERFIRST+4";
    case static_cast<long>(0x80040305UL):
        // STRING: VC1 0x004cf660
        return "DIERR_DRIVERFIRST+5";
    case static_cast<long>(0x800403ffUL):
        // STRING: VC1 0x004cf674
        return "DIERR_DRIVERLAST";
    case static_cast<long>(0x80040400UL):
        // STRING: VC1 0x004cf688
        return "DIERR_INVALIDCLASSINSTALLER";
    case static_cast<long>(0x80040401UL):
        // STRING: VC1 0x004cf6a4
        return "DIERR_CANCELLED";
    case static_cast<long>(0x80040402UL):
        // STRING: VC1 0x004cf650
        return "DIERR_BADINF";
    case static_cast<long>(0x80070002UL):
        // STRING: VC1 0x004cf600
        return "DIERR_NOTFOUND";
    case static_cast<long>(0x80070005UL):
        // STRING: VC1 0x004cf610
        return "E_ACCESSDENIED";
    case static_cast<long>(0x80070006UL):
        // STRING: VC1 0x004cf620
        return "E_HANDLE";
    case static_cast<long>(0x8007000cUL):
        // STRING: VC1 0x004cf62c
        return "DIERR_NOTACQUIRED";
    case static_cast<long>(0x8007000eUL):
        // STRING: VC1 0x004cf640
        return "E_OUTOFMEMORY";
    case static_cast<long>(0x80070015UL):
        // STRING: VC1 0x004cf5e8
        return "DIERR_NOTINITIALIZED";
    case static_cast<long>(0x8007001eUL):
        // STRING: VC1 0x004cf5a4
        return "DIERR_INPUTLOST";
    case static_cast<long>(0x80070057UL):
        // STRING: VC1 0x004cf5b4
        return "E_INVALIDARG";
    case static_cast<long>(0x80070077UL):
        // STRING: VC1 0x004cf5c4
        return "DIERR_BADDRIVERVER";
    case static_cast<long>(0x800700aaUL):
        // STRING: VC1 0x004cf5d8
        return "DIERR_ACQUIRED";
    case static_cast<long>(0x80070103UL):
        // STRING: VC1 0x004cf590
        return "DIERR_NOMOREITEMS";
    case static_cast<long>(0x8007047eUL):
        // STRING: VC1 0x004cf510
        return "DIERR_OLDDIRECTINPUTVERSION";
    case static_cast<long>(0x80070481UL):
        // STRING: VC1 0x004cf52c
        return "DIERR_BETADIRECTINPUTVERSION";
    case static_cast<long>(0x800704dfUL):
        // STRING: VC1 0x004cf54c
        return "DIERR_ALREADYINITIALIZED";
    case static_cast<long>(0x8015001eUL):
        // STRING: VC1 0x004cf568
        return "DVERR_BUFFERTOOSMALL";
    case static_cast<long>(0x8015004aUL):
        // STRING: VC1 0x004cf580
        return "DVERR_EXCEPTION";
    case static_cast<long>(0x80150078UL):
        // STRING: VC1 0x004cf4fc
        return "DVERR_INVALIDFLAGS";
    case static_cast<long>(0x80150082UL):
        // STRING: VC1 0x004cf4ac
        return "DVERR_INVALIDOBJECT";
    case static_cast<long>(0x80150087UL):
        // STRING: VC1 0x004cf4c0
        return "DVERR_INVALIDPLAYER";
    case static_cast<long>(0x80150091UL):
        // STRING: VC1 0x004cf4d4
        return "DVERR_INVALIDGROUP";
    case static_cast<long>(0x80150096UL):
        // STRING: VC1 0x004cf4e8
        return "DVERR_INVALIDHANDLE";
    case static_cast<long>(0x8015012cUL):
        // STRING: VC1 0x004cf498
        return "DVERR_SESSIONLOST";
    case static_cast<long>(0x8015012eUL):
        // STRING: VC1 0x004cf42c
        return "DVERR_NOVOICESESSION";
    case static_cast<long>(0x80150168UL):
        // STRING: VC1 0x004cf444
        return "DVERR_CONNECTIONLOST";
    case static_cast<long>(0x80150169UL):
        // STRING: VC1 0x004cf45c
        return "DVERR_NOTINITIALIZED";
    case static_cast<long>(0x8015016aUL):
        // STRING: VC1 0x004cf474
        return "DVERR_CONNECTED";
    case static_cast<long>(0x8015016bUL):
        // STRING: VC1 0x004cf484
        return "DVERR_NOTCONNECTED";
    case static_cast<long>(0x8015016eUL):
        // STRING: VC1 0x004cf414
        return "DVERR_CONNECTABORTING";
    case static_cast<long>(0x8015016fUL):
        // STRING: VC1 0x004cf3b4
        return "DVERR_NOTALLOWED";
    case static_cast<long>(0x80150170UL):
        // STRING: VC1 0x004cf3c8
        return "DVERR_INVALIDTARGET";
    case static_cast<long>(0x80150171UL):
        // STRING: VC1 0x004cf3dc
        return "DVERR_TRANSPORTNOTHOST";
    case static_cast<long>(0x80150172UL):
        // STRING: VC1 0x004cf3f4
        return "DVERR_COMPRESSIONNOTSUPPORTED";
    case static_cast<long>(0x80150173UL):
        // STRING: VC1 0x004cf39c
        return "DVERR_ALREADYPENDING";
    case static_cast<long>(0x80150174UL):
        // STRING: VC1 0x004cf384
        return "DVERR_SOUNDINITFAILURE";
    case static_cast<long>(0x80150175UL):
        // STRING: VC1 0x004cf374
        return "DVERR_TIMEOUT";
    case static_cast<long>(0x80150176UL):
        // STRING: VC1 0x004cf35c
        return "DVERR_CONNECTABORTED";
    case static_cast<long>(0x80150177UL):
        // STRING: VC1 0x004cf34c
        return "DVERR_NO3DSOUND";
    case static_cast<long>(0x80150178UL):
        // STRING: VC1 0x004cf334
        return "DVERR_ALREADYBUFFERED";
    case static_cast<long>(0x80150179UL):
        // STRING: VC1 0x004cf320
        return "DVERR_NOTBUFFERED";
    case static_cast<long>(0x8015017aUL):
        // STRING: VC1 0x004cf310
        return "DVERR_HOSTING";
    case static_cast<long>(0x8015017bUL):
        // STRING: VC1 0x004cf2fc
        return "DVERR_NOTHOSTING";
    case static_cast<long>(0x8015017cUL):
        // STRING: VC1 0x004cf2e8
        return "DVERR_INVALIDDEVICE";
    case static_cast<long>(0x8015017dUL):
        // STRING: VC1 0x004cf2d0
        return "DVERR_RECORDSYSTEMERROR";
    case static_cast<long>(0x8015017eUL):
        // STRING: VC1 0x004cf2b4
        return "DVERR_PLAYBACKSYSTEMERROR";
    case static_cast<long>(0x8015017fUL):
        // STRING: VC1 0x004cf2a4
        return "DVERR_SENDERROR";
    case static_cast<long>(0x80150180UL):
        // STRING: VC1 0x004cf290
        return "DVERR_USERCANCEL";
    case static_cast<long>(0x80150183UL):
        // STRING: VC1 0x004cf280
        return "DVERR_RUNSETUP";
    case static_cast<long>(0x80150184UL):
        // STRING: VC1 0x004cf264
        return "DVERR_INCOMPATIBLEVERSION";
    case static_cast<long>(0x80150187UL):
        // STRING: VC1 0x004cf250
        return "DVERR_INITIALIZED";
    case static_cast<long>(0x80150188UL):
        // STRING: VC1 0x004cf23c
        return "DVERR_NOTRANSPORT";
    case static_cast<long>(0x80150189UL):
        // STRING: VC1 0x004cf228
        return "DVERR_NOCALLBACK";
    case static_cast<long>(0x8015018aUL):
        // STRING: VC1 0x004cf210
        return "DVERR_TRANSPORTNOTINIT";
    case static_cast<long>(0x8015018bUL):
        // STRING: VC1 0x004cf1f4
        return "DVERR_TRANSPORTNOSESSION";
    case static_cast<long>(0x8015018cUL):
        // STRING: VC1 0x004cf1dc
        return "DVERR_TRANSPORTNOPLAYER";
    case static_cast<long>(0x8015018dUL):
        // STRING: VC1 0x004cf1cc
        return "DVERR_USERBACK";
    case static_cast<long>(0x8015018eUL):
        // STRING: VC1 0x004cf168
        return "DVERR_NORECVOLAVAILABLE";
    case static_cast<long>(0x8015018fUL):
        // STRING: VC1 0x004cf180
        return "DVERR_INVALIDBUFFER";
    case static_cast<long>(0x80150190UL):
        // STRING: VC1 0x004cf194
        return "DVERR_LOCKEDBUFFER";
    case static_cast<long>(0x80158030UL):
        // STRING: VC1 0x004cf1a8
        return "DPNERR_ABORTED";
    case static_cast<long>(0x80158040UL):
        // STRING: VC1 0x004cf1b8
        return "DPNERR_ADDRESSING";
    case static_cast<long>(0x80158050UL):
        // STRING: VC1 0x004cf150
        return "DPNERR_ALREADYCLOSING";
    case static_cast<long>(0x80158060UL):
        // STRING: VC1 0x004cf0e4
        return "DPNERR_ALREADYCONNECTED";
    case static_cast<long>(0x80158070UL):
        // STRING: VC1 0x004cf0fc
        return "DPNERR_ALREADYDISCONNECTING";
    case static_cast<long>(0x80158080UL):
        // STRING: VC1 0x004cf118
        return "DPNERR_ALREADYINITIALIZED";
    case static_cast<long>(0x80158090UL):
        // STRING: VC1 0x004cf134
        return "DPNERR_ALREADYREGISTERED";
    case static_cast<long>(0x80158100UL):
        // STRING: VC1 0x004cf0cc
        return "DPNERR_BUFFERTOOSMALL";
    case static_cast<long>(0x80158110UL):
        // STRING: VC1 0x004cf054
        return "DPNERR_CANNOTCANCEL";
    case static_cast<long>(0x80158120UL):
        // STRING: VC1 0x004cf068
        return "DPNERR_CANTCREATEGROUP";
    case static_cast<long>(0x80158130UL):
        // STRING: VC1 0x004cf080
        return "DPNERR_CANTCREATEPLAYER";
    case static_cast<long>(0x80158140UL):
        // STRING: VC1 0x004cf098
        return "DPNERR_CANTLAUNCHAPPLICATION";
    case static_cast<long>(0x80158150UL):
        // STRING: VC1 0x004cf0b8
        return "DPNERR_CONNECTING";
    case static_cast<long>(0x80158160UL):
        // STRING: VC1 0x004cf03c
        return "DPNERR_CONNECTIONLOST";
    case static_cast<long>(0x80158170UL):
        // STRING: VC1 0x004cefe8
        return "DPNERR_CONVERSION";
    case static_cast<long>(0x80158175UL):
        // STRING: VC1 0x004ceffc
        return "DPNERR_DATATOOLARGE";
    case static_cast<long>(0x80158180UL):
        // STRING: VC1 0x004cf010
        return "DPNERR_DOESNOTEXIST";
    case static_cast<long>(0x80158190UL):
        // STRING: VC1 0x004cf024
        return "DPNERR_DUPLICATECOMMAND";
    case static_cast<long>(0x80158200UL):
        // STRING: VC1 0x004cefcc
        return "DPNERR_ENDPOINTNOTRECEIVING";
    case static_cast<long>(0x80158210UL):
        // STRING: VC1 0x004cef58
        return "DPNERR_ENUMQUERYTOOLARGE";
    case static_cast<long>(0x80158220UL):
        // STRING: VC1 0x004cef74
        return "DPNERR_ENUMRESPONSETOOLARGE";
    case static_cast<long>(0x80158230UL):
        // STRING: VC1 0x004cef90
        return "DPNERR_EXCEPTION";
    case static_cast<long>(0x80158240UL):
        // STRING: VC1 0x004cefa4
        return "DPNERR_GROUPNOTEMPTY";
    case static_cast<long>(0x80158250UL):
        // STRING: VC1 0x004cefbc
        return "DPNERR_HOSTING";
    case static_cast<long>(0x80158260UL):
        // STRING: VC1 0x004cef38
        return "DPNERR_HOSTREJECTEDCONNECTION";
    case static_cast<long>(0x80158270UL):
        // STRING: VC1 0x004ceec4
        return "DPNERR_HOSTTERMINATEDSESSION";
    case static_cast<long>(0x80158280UL):
        // STRING: VC1 0x004ceee4
        return "DPNERR_INCOMPLETEADDRESS";
    case static_cast<long>(0x80158290UL):
        // STRING: VC1 0x004cef00
        return "DPNERR_INVALIDADDRESSFORMAT";
    case static_cast<long>(0x80158300UL):
        // STRING: VC1 0x004cef1c
        return "DPNERR_INVALIDAPPLICATION";
    case static_cast<long>(0x80158310UL):
        // STRING: VC1 0x004ceeac
        return "DPNERR_INVALIDCOMMAND";
    case static_cast<long>(0x80158320UL):
        // STRING: VC1 0x004cee38
        return "DPNERR_INVALIDDEVICEADDRESS";
    case static_cast<long>(0x80158330UL):
        // STRING: VC1 0x004cee54
        return "DPNERR_INVALIDENDPOINT";
    case static_cast<long>(0x80158340UL):
        // STRING: VC1 0x004cee6c
        return "DPNERR_INVALIDFLAGS";
    case static_cast<long>(0x80158350UL):
        // STRING: VC1 0x004cee80
        return "DPNERR_INVALIDGROUP";
    case static_cast<long>(0x80158360UL):
        // STRING: VC1 0x004cee94
        return "DPNERR_INVALIDHANDLE";
    case static_cast<long>(0x80158370UL):
        // STRING: VC1 0x004cee1c
        return "DPNERR_INVALIDHOSTADDRESS";
    case static_cast<long>(0x80158380UL):
        // STRING: VC1 0x004cedbc
        return "DPNERR_INVALIDINSTANCE";
    case static_cast<long>(0x80158390UL):
        // STRING: VC1 0x004cedd4
        return "DPNERR_INVALIDINTERFACE";
    case static_cast<long>(0x80158400UL):
        // STRING: VC1 0x004cedec
        return "DPNERR_INVALIDOBJECT";
    case static_cast<long>(0x80158410UL):
        // STRING: VC1 0x004cee04
        return "DPNERR_INVALIDPASSWORD";
    case static_cast<long>(0x80158420UL):
        // STRING: VC1 0x004ceda4
        return "DPNERR_INVALIDPLAYER";
    case static_cast<long>(0x80158430UL):
        // STRING: VC1 0x004ced38
        return "DPNERR_INVALIDPRIORITY";
    case static_cast<long>(0x80158440UL):
        // STRING: VC1 0x004ced50
        return "DPNERR_INVALIDSTRING";
    case static_cast<long>(0x80158450UL):
        // STRING: VC1 0x004ced68
        return "DPNERR_INVALIDURL";
    case static_cast<long>(0x80158460UL):
        // STRING: VC1 0x004ced7c
        return "DPNERR_INVALIDVERSION";
    case static_cast<long>(0x80158470UL):
        // STRING: VC1 0x004ced94
        return "DPNERR_NOCAPS";
    case static_cast<long>(0x80158480UL):
        // STRING: VC1 0x004ced24
        return "DPNERR_NOCONNECTION";
    case static_cast<long>(0x80158490UL):
        // STRING: VC1 0x004cecc8
        return "DPNERR_NOHOSTPLAYER";
    case static_cast<long>(0x80158500UL):
        // STRING: VC1 0x004cecdc
        return "DPNERR_NOMOREADDRESSCOMPONENTS";
    case static_cast<long>(0x80158510UL):
        // STRING: VC1 0x004cecfc
        return "DPNERR_NORESPONSE";
    case static_cast<long>(0x80158520UL):
        // STRING: VC1 0x004ced10
        return "DPNERR_NOTALLOWED";
    case static_cast<long>(0x80158530UL):
        // STRING: VC1 0x004cecb8
        return "DPNERR_NOTHOST";
    case static_cast<long>(0x80158540UL):
        // STRING: VC1 0x004cec4c
        return "DPNERR_NOTREADY";
    case static_cast<long>(0x80158550UL):
        // STRING: VC1 0x004cec5c
        return "DPNERR_NOTREGISTERED";
    case static_cast<long>(0x80158570UL):
        // STRING: VC1 0x004cec74
        return "DPNERR_PLAYERLOST";
    case static_cast<long>(0x80158590UL):
        // STRING: VC1 0x004cec88
        return "DPNERR_PLAYERNOTREACHABLE";
    case static_cast<long>(0x80158600UL):
        // STRING: VC1 0x004ceca4
        return "DPNERR_SENDTOOLARGE";
    case static_cast<long>(0x80158610UL):
        // STRING: VC1 0x004cec38
        return "DPNERR_SESSIONFULL";
    case static_cast<long>(0x80158620UL):
        // STRING: VC1 0x004cebe8
        return "DPNERR_TABLEFULL";
    case static_cast<long>(0x80158630UL):
        // STRING: VC1 0x004cebfc
        return "DPNERR_TIMEDOUT";
    case static_cast<long>(0x80158640UL):
        // STRING: VC1 0x004cec0c
        return "DPNERR_UNINITIALIZED";
    case static_cast<long>(0x80158650UL):
        // STRING: VC1 0x004cec24
        return "DPNERR_USERCANCEL";
    case static_cast<long>(0x8876017cUL):
        // STRING: VC1 0x004cebd0
        return "D3DERR_OUTOFVIDEOMEMORY";
    case static_cast<long>(0x88760352UL):
        // STRING: VC1 0x004cebbc
        return "DXFILEERR_BADOBJECT";
    case static_cast<long>(0x88760353UL):
        // STRING: VC1 0x004ceba8
        return "DXFILEERR_BADVALUE";
    case static_cast<long>(0x88760354UL):
        // STRING: VC1 0x004ceb94
        return "DXFILEERR_BADTYPE";
    case static_cast<long>(0x88760355UL):
        // STRING: VC1 0x004ceb78
        return "DXFILEERR_BADSTREAMHANDLE";
    case static_cast<long>(0x88760356UL):
        // STRING: VC1 0x004ceb64
        return "DXFILEERR_BADALLOC";
    case static_cast<long>(0x88760357UL):
        // STRING: VC1 0x004ceb50
        return "DXFILEERR_NOTFOUND";
    case static_cast<long>(0x88760358UL):
        // STRING: VC1 0x004ceb38
        return "DXFILEERR_NOTDONEYET";
    case static_cast<long>(0x88760359UL):
        // STRING: VC1 0x004ceb20
        return "DXFILEERR_FILENOTFOUND";
    case static_cast<long>(0x8876035aUL):
        // STRING: VC1 0x004ceb04
        return "DXFILEERR_RESOURCENOTFOUND";
    case static_cast<long>(0x8876035bUL):
        // STRING: VC1 0x004ceaec
        return "DXFILEERR_URLNOTFOUND";
    case static_cast<long>(0x8876035cUL):
        // STRING: VC1 0x004cead4
        return "DXFILEERR_BADRESOURCE";
    case static_cast<long>(0x8876035dUL):
        // STRING: VC1 0x004ceabc
        return "DXFILEERR_BADFILETYPE";
    case static_cast<long>(0x8876035eUL):
        // STRING: VC1 0x004ceaa0
        return "DXFILEERR_BADFILEVERSION";
    case static_cast<long>(0x8876035fUL):
        // STRING: VC1 0x004cea84
        return "DXFILEERR_BADFILEFLOATSIZE";
    case static_cast<long>(0x88760360UL):
        // STRING: VC1 0x004cea60
        return "DXFILEERR_BADFILECOMPRESSIONTYPE";
    case static_cast<long>(0x88760361UL):
        // STRING: VC1 0x004cea4c
        return "DXFILEERR_BADFILE";
    case static_cast<long>(0x88760362UL):
        // STRING: VC1 0x004cea34
        return "DXFILEERR_PARSEERROR";
    case static_cast<long>(0x88760363UL):
        // STRING: VC1 0x004cea1c
        return "DXFILEERR_NOTEMPLATE";
    case static_cast<long>(0x88760364UL):
        // STRING: VC1 0x004cea04
        return "DXFILEERR_BADARRAYSIZE";
    case static_cast<long>(0x88760365UL):
        // STRING: VC1 0x004ce9e8
        return "DXFILEERR_BADDATAREFERENCE";
    case static_cast<long>(0x88760366UL):
        // STRING: VC1 0x004ce9d0
        return "DXFILEERR_INTERNALERROR";
    case static_cast<long>(0x88760367UL):
        // STRING: VC1 0x004ce9b8
        return "DXFILEERR_NOMOREOBJECTS";
    case static_cast<long>(0x88760368UL):
        // STRING: VC1 0x004ce938
        return "DXFILEERR_BADINTRINSICS";
    case static_cast<long>(0x88760369UL):
        // STRING: VC1 0x004ce950
        return "DXFILEERR_NOMORESTREAMHANDLES";
    case static_cast<long>(0x8876036aUL):
        // STRING: VC1 0x004ce970
        return "DXFILEERR_NOMOREDATA";
    case static_cast<long>(0x8876036bUL):
        // STRING: VC1 0x004ce988
        return "DXFILEERR_BADCACHEFILE";
    case static_cast<long>(0x8876036cUL):
        // STRING: VC1 0x004ce9a0
        return "DXFILEERR_NOINTERNET";
    case static_cast<long>(0x88760818UL):
        // STRING: VC1 0x004ce91c
        return "D3DERR_WRONGTEXTUREFORMAT";
    case static_cast<long>(0x88760819UL):
        // STRING: VC1 0x004ce89c
        return "D3DERR_UNSUPPORTEDCOLOROPERATION";
    case static_cast<long>(0x8876081aUL):
        // STRING: VC1 0x004ce8c0
        return "D3DERR_UNSUPPORTEDCOLORARG";
    case static_cast<long>(0x8876081bUL):
        // STRING: VC1 0x004ce8dc
        return "D3DERR_UNSUPPORTEDALPHAOPERATION";
    case static_cast<long>(0x8876081cUL):
        // STRING: VC1 0x004ce900
        return "D3DERR_UNSUPPORTEDALPHAARG";
    case static_cast<long>(0x8876081dUL):
        // STRING: VC1 0x004ce880
        return "D3DERR_TOOMANYOPERATIONS";
    case static_cast<long>(0x8876081eUL):
        // STRING: VC1 0x004ce7dc
        return "D3DERR_CONFLICTINGTEXTUREFILTER";
    case static_cast<long>(0x8876081fUL):
        // STRING: VC1 0x004ce7fc
        return "D3DERR_UNSUPPORTEDFACTORVALUE";
    case static_cast<long>(0x88760821UL):
        // STRING: VC1 0x004ce81c
        return "D3DERR_CONFLICTINGRENDERSTATE";
    case static_cast<long>(0x88760822UL):
        // STRING: VC1 0x004ce83c
        return "D3DERR_UNSUPPORTEDTEXTUREFILTER";
    case static_cast<long>(0x88760826UL):
        // STRING: VC1 0x004ce85c
        return "D3DERR_CONFLICTINGTEXTUREPALETTE";
    case static_cast<long>(0x88760827UL):
        // STRING: VC1 0x004ce7c0
        return "D3DERR_DRIVERINTERNALERROR";
    case static_cast<long>(0x88760866UL):
        // STRING: VC1 0x004ce774
        return "D3DERR_NOTFOUND";
    case static_cast<long>(0x88760867UL):
        // STRING: VC1 0x004ce784
        return "D3DERR_MOREDATA";
    case static_cast<long>(0x88760868UL):
        // STRING: VC1 0x004ce794
        return "D3DERR_DEVICELOST";
    case static_cast<long>(0x88760869UL):
        // STRING: VC1 0x004ce7a8
        return "D3DERR_DEVICENOTRESET";
    case static_cast<long>(0x8876086aUL):
        // STRING: VC1 0x004ce760
        return "D3DERR_NOTAVAILABLE";
    case static_cast<long>(0x8876086bUL):
        // STRING: VC1 0x004ce6f0
        return "D3DERR_INVALIDDEVICE";
    case static_cast<long>(0x8876086cUL):
        // STRING: VC1 0x004ce708
        return "D3DERR_INVALIDCALL";
    case static_cast<long>(0x8876086dUL):
        // STRING: VC1 0x004ce71c
        return "D3DERR_DRIVERINVALIDCALL";
    case static_cast<long>(0x8878000aUL):
        // STRING: VC1 0x004ce738
        return "DSERR_ALLOCATED";
    case static_cast<long>(0x8878001eUL):
        // STRING: VC1 0x004ce748
        return "DSERR_CONTROLUNAVAIL";
    case static_cast<long>(0x88780032UL):
        // STRING: VC1 0x004ce6dc
        return "DSERR_INVALIDCALL";
    case static_cast<long>(0x88780046UL):
        // STRING: VC1 0x004ce688
        return "DSERR_PRIOLEVELNEEDED";
    case static_cast<long>(0x88780064UL):
        // STRING: VC1 0x004ce6a0
        return "DSERR_BADFORMAT";
    case static_cast<long>(0x88780078UL):
        // STRING: VC1 0x004ce6b0
        return "DSERR_NODRIVER";
    case static_cast<long>(0x88780082UL):
        // STRING: VC1 0x004ce6c0
        return "DSERR_ALREADYINITIALIZED";
    case static_cast<long>(0x88780096UL):
        // STRING: VC1 0x004ce674
        return "DSERR_BUFFERLOST";
    case static_cast<long>(0x887800a0UL):
        // STRING: VC1 0x004ce60c
        return "DSERR_OTHERAPPHASPRIO";
    case static_cast<long>(0x887800aaUL):
        // STRING: VC1 0x004ce624
        return "DSERR_UNINITIALIZED";
    case static_cast<long>(0x887800b4UL):
        // STRING: VC1 0x004ce638
        return "DSERR_BUFFERTOOSMALL";
    case static_cast<long>(0x887800beUL):
        // STRING: VC1 0x004ce650
        return "DSERR_DS8_REQUIRED";
    case static_cast<long>(0x887800c8UL):
        // STRING: VC1 0x004ce664
        return "DSERR_SENDLOOP";
    case static_cast<long>(0x887800d2UL):
        // STRING: VC1 0x004ce5f4
        return "DSERR_BADSENDBUFFERGUID";
    case static_cast<long>(0x88781101UL):
        // STRING: VC1 0x004ce594
        return "DMUS_E_DRIVER_FAILED";
    case static_cast<long>(0x88781102UL):
        // STRING: VC1 0x004ce5ac
        return "DMUS_E_PORTS_OPEN";
    case static_cast<long>(0x88781103UL):
        // STRING: VC1 0x004ce5c0
        return "DMUS_E_DEVICE_IN_USE";
    case static_cast<long>(0x88781104UL):
        // STRING: VC1 0x004ce5d8
        return "DMUS_E_INSUFFICIENTBUFFER";
    case static_cast<long>(0x88781105UL):
        // STRING: VC1 0x004ce580
        return "DMUS_E_BUFFERNOTSET";
    case static_cast<long>(0x88781106UL):
        // STRING: VC1 0x004ce564
        return "DMUS_E_BUFFERNOTAVAILABLE";
    case static_cast<long>(0x88781108UL):
        // STRING: VC1 0x004ce550
        return "DMUS_E_NOTADLSCOL";
    case static_cast<long>(0x88781109UL):
        // STRING: VC1 0x004ce538
        return "DMUS_E_INVALIDOFFSET";
    case static_cast<long>(0x88781111UL):
        // STRING: VC1 0x004ce520
        return "DMUS_E_ALREADY_LOADED";
    case static_cast<long>(0x88781113UL):
        // STRING: VC1 0x004ce50c
        return "DMUS_E_INVALIDPOS";
    case static_cast<long>(0x88781114UL):
        // STRING: VC1 0x004ce4f8
        return "DMUS_E_INVALIDPATCH";
    case static_cast<long>(0x88781115UL):
        // STRING: VC1 0x004ce4e4
        return "DMUS_E_CANNOTSEEK";
    case static_cast<long>(0x88781116UL):
        // STRING: VC1 0x004ce4d0
        return "DMUS_E_CANNOTWRITE";
    case static_cast<long>(0x88781117UL):
        // STRING: VC1 0x004ce4b8
        return "DMUS_E_CHUNKNOTFOUND";
    case static_cast<long>(0x88781119UL):
        // STRING: VC1 0x004ce49c
        return "DMUS_E_INVALID_DOWNLOADID";
    case static_cast<long>(0x88781120UL):
        // STRING: VC1 0x004ce47c
        return "DMUS_E_NOT_DOWNLOADED_TO_PORT";
    case static_cast<long>(0x88781121UL):
        // STRING: VC1 0x004ce460
        return "DMUS_E_ALREADY_DOWNLOADED";
    case static_cast<long>(0x88781122UL):
        // STRING: VC1 0x004ce448
        return "DMUS_E_UNKNOWN_PROPERTY";
    case static_cast<long>(0x88781123UL):
        // STRING: VC1 0x004ce430
        return "DMUS_E_SET_UNSUPPORTED";
    case static_cast<long>(0x88781124UL):
        // STRING: VC1 0x004ce418
        return "DMUS_E_GET_UNSUPPORTED";
    case static_cast<long>(0x88781125UL):
        // STRING: VC1 0x004ce408
        return "DMUS_E_NOTMONO";
    case static_cast<long>(0x88781126UL):
        // STRING: VC1 0x004ce3f0
        return "DMUS_E_BADARTICULATION";
    case static_cast<long>(0x88781127UL):
        // STRING: VC1 0x004ce3d8
        return "DMUS_E_BADINSTRUMENT";
    case static_cast<long>(0x88781128UL):
        // STRING: VC1 0x004ce3c4
        return "DMUS_E_BADWAVELINK";
    case static_cast<long>(0x88781129UL):
        // STRING: VC1 0x004ce3ac
        return "DMUS_E_NOARTICULATION";
    case static_cast<long>(0x8878112aUL):
        // STRING: VC1 0x004ce39c
        return "DMUS_E_NOTPCM";
    case static_cast<long>(0x8878112bUL):
        // STRING: VC1 0x004ce38c
        return "DMUS_E_BADWAVE";
    case static_cast<long>(0x8878112cUL):
        // STRING: VC1 0x004ce374
        return "DMUS_E_BADOFFSETTABLE";
    case static_cast<long>(0x8878112dUL):
        // STRING: VC1 0x004ce35c
        return "DMUS_E_UNKNOWNDOWNLOAD";
    case static_cast<long>(0x8878112eUL):
        // STRING: VC1 0x004ce348
        return "DMUS_E_NOSYNTHSINK";
    case static_cast<long>(0x8878112fUL):
        // STRING: VC1 0x004ce334
        return "DMUS_E_ALREADYOPEN";
    case static_cast<long>(0x88781130UL):
        // STRING: VC1 0x004ce31c
        return "DMUS_E_ALREADYCLOSED";
    case static_cast<long>(0x88781131UL):
        // STRING: VC1 0x004ce300
        return "DMUS_E_SYNTHNOTCONFIGURED";
    case static_cast<long>(0x88781132UL):
        // STRING: VC1 0x004ce2ec
        return "DMUS_E_SYNTHACTIVE";
    case static_cast<long>(0x88781133UL):
        // STRING: VC1 0x004ce2d8
        return "DMUS_E_CANNOTREAD";
    case static_cast<long>(0x88781134UL):
        // STRING: VC1 0x004ce2c0
        return "DMUS_E_DMUSIC_RELEASED";
    case static_cast<long>(0x88781135UL):
        // STRING: VC1 0x004ce2ac
        return "DMUS_E_BUFFER_EMPTY";
    case static_cast<long>(0x88781136UL):
        // STRING: VC1 0x004ce298
        return "DMUS_E_BUFFER_FULL";
    case static_cast<long>(0x88781137UL):
        // STRING: VC1 0x004ce280
        return "DMUS_E_PORT_NOT_CAPTURE";
    case static_cast<long>(0x88781138UL):
        // STRING: VC1 0x004ce268
        return "DMUS_E_PORT_NOT_RENDER";
    case static_cast<long>(0x88781139UL):
        // STRING: VC1 0x004ce250
        return "DMUS_E_DSOUND_NOT_SET";
    case static_cast<long>(0x8878113aUL):
        // STRING: VC1 0x004ce234
        return "DMUS_E_ALREADY_ACTIVATED";
    case static_cast<long>(0x8878113bUL):
        // STRING: VC1 0x004ce21c
        return "DMUS_E_INVALIDBUFFER";
    case static_cast<long>(0x8878113cUL):
        // STRING: VC1 0x004ce1fc
        return "DMUS_E_WAVEFORMATNOTSUPPORTED";
    case static_cast<long>(0x8878113dUL):
        // STRING: VC1 0x004ce1e4
        return "DMUS_E_SYNTHINACTIVE";
    case static_cast<long>(0x8878113eUL):
        // STRING: VC1 0x004ce1c8
        return "DMUS_E_DSOUND_ALREADY_SET";
    case static_cast<long>(0x8878113fUL):
        // STRING: VC1 0x004ce1b0
        return "DMUS_E_INVALID_EVENT";
    case static_cast<long>(0x88781150UL):
        // STRING: VC1 0x004ce194
        return "DMUS_E_UNSUPPORTED_STREAM";
    case static_cast<long>(0x88781151UL):
        // STRING: VC1 0x004ce17c
        return "DMUS_E_ALREADY_INITED";
    case static_cast<long>(0x88781152UL):
        // STRING: VC1 0x004ce150
        return "DMUS_E_INVALID_BAND";
    case static_cast<long>(0x88781155UL):
        // STRING: VC1 0x004ce130
        return "DMUS_E_TRACK_HDR_NOT_FIRST_CK";
    case static_cast<long>(0x88781156UL):
        // STRING: VC1 0x004ce110
        return "DMUS_E_TOOL_HDR_NOT_FIRST_CK";
    case static_cast<long>(0x88781157UL):
        // STRING: VC1 0x004ce0f4
        return "DMUS_E_INVALID_TRACK_HDR";
    case static_cast<long>(0x88781158UL):
        // STRING: VC1 0x004ce0dc
        return "DMUS_E_INVALID_TOOL_HDR";
    case static_cast<long>(0x88781159UL):
        // STRING: VC1 0x004ce0c4
        return "DMUS_E_ALL_TOOLS_FAILED";
    case static_cast<long>(0x88781160UL):
        // STRING: VC1 0x004ce0a8
        return "DMUS_E_ALL_TRACKS_FAILED";
    case static_cast<long>(0x88781161UL):
        // STRING: VC1 0x004ce164
        return "DSERR_OBJECTNOTFOUND";
    case static_cast<long>(0x88781162UL):
        // STRING: VC1 0x004ce098
        return "DMUS_E_NOT_INIT";
    case static_cast<long>(0x88781163UL):
        // STRING: VC1 0x004ce080
        return "DMUS_E_TYPE_DISABLED";
    case static_cast<long>(0x88781164UL):
        // STRING: VC1 0x004ce068
        return "DMUS_E_TYPE_UNSUPPORTED";
    case static_cast<long>(0x88781165UL):
        // STRING: VC1 0x004ce054
        return "DMUS_E_TIME_PAST";
    case static_cast<long>(0x88781166UL):
        // STRING: VC1 0x004ce03c
        return "DMUS_E_TRACK_NOT_FOUND";
    case static_cast<long>(0x88781167UL):
        // STRING: VC1 0x004ce018
        return "DMUS_E_TRACK_NO_CLOCKTIME_SUPPORT";
    case static_cast<long>(0x88781170UL):
        // STRING: VC1 0x004ce000
        return "DMUS_E_NO_MASTER_CLOCK";
    case static_cast<long>(0x88781180UL):
        // STRING: VC1 0x004cdfe8
        return "DMUS_E_LOADER_NOCLASSID";
    case static_cast<long>(0x88781181UL):
        // STRING: VC1 0x004cdfd0
        return "DMUS_E_LOADER_BADPATH";
    case static_cast<long>(0x88781182UL):
        // STRING: VC1 0x004cdfb4
        return "DMUS_E_LOADER_FAILEDOPEN";
    case static_cast<long>(0x88781183UL):
        // STRING: VC1 0x004cdf90
        return "DMUS_E_LOADER_FORMATNOTSUPPORTED";
    case static_cast<long>(0x88781184UL):
        // STRING: VC1 0x004cdf74
        return "DMUS_E_LOADER_FAILEDCREATE";
    case static_cast<long>(0x88781185UL):
        // STRING: VC1 0x004cdf54
        return "DMUS_E_LOADER_OBJECTNOTFOUND";
    case static_cast<long>(0x88781186UL):
        // STRING: VC1 0x004cdf38
        return "DMUS_E_LOADER_NOFILENAME";
    case static_cast<long>(0x88781200UL):
        // STRING: VC1 0x004cdf24
        return "DMUS_E_INVALIDFILE";
    case static_cast<long>(0x88781201UL):
        // STRING: VC1 0x004cdf0c
        return "DMUS_E_ALREADY_EXISTS";
    case static_cast<long>(0x88781202UL):
        // STRING: VC1 0x004cdef8
        return "DMUS_E_OUT_OF_RANGE";
    case static_cast<long>(0x88781203UL):
        // STRING: VC1 0x004cdedc
        return "DMUS_E_SEGMENT_INIT_FAILED";
    case static_cast<long>(0x88781204UL):
        // STRING: VC1 0x004cdec8
        return "DMUS_E_ALREADY_SENT";
    case static_cast<long>(0x88781205UL):
        // STRING: VC1 0x004cdeb4
        return "DMUS_E_CANNOT_FREE";
    case static_cast<long>(0x88781206UL):
        // STRING: VC1 0x004cde9c
        return "DMUS_E_CANNOT_OPEN_PORT";
    case static_cast<long>(0x88781207UL):
        // STRING: VC1 0x004cde84
        return "DMUS_E_CANNOT_CONVERT";
    case static_cast<long>(0x88781210UL):
        // STRING: VC1 0x004cde68
        return "DMUS_E_DESCEND_CHUNK_FAIL";
    case static_cast<long>(0x88781211UL):
        // STRING: VC1 0x004cde54
        return "DMUS_E_NOT_LOADED";
    case static_cast<long>(0x88781213UL):
        // STRING: VC1 0x004cde30
        return "DMUS_E_SCRIPT_LANGUAGE_INCOMPATIBLE";
    case static_cast<long>(0x88781214UL):
        // STRING: VC1 0x004cde0c
        return "DMUS_E_SCRIPT_UNSUPPORTED_VARTYPE";
    case static_cast<long>(0x88781215UL):
        // STRING: VC1 0x004cddec
        return "DMUS_E_SCRIPT_ERROR_IN_SCRIPT";
    case static_cast<long>(0x88781216UL):
        // STRING: VC1 0x004cddcc
        return "DMUS_E_SCRIPT_CANTLOAD_OLEAUT32";
    case static_cast<long>(0x88781217UL):
        // STRING: VC1 0x004cddac
        return "DMUS_E_SCRIPT_LOADSCRIPT_ERROR";
    case static_cast<long>(0x88781218UL):
        // STRING: VC1 0x004cdd90
        return "DMUS_E_SCRIPT_INVALID_FILE";
    case static_cast<long>(0x88781219UL):
        // STRING: VC1 0x004cdd74
        return "DMUS_E_INVALID_SCRIPTTRACK";
    case static_cast<long>(0x8878121aUL):
        // STRING: VC1 0x004cdd50
        return "DMUS_E_SCRIPT_VARIABLE_NOT_FOUND";
    case static_cast<long>(0x8878121bUL):
        // STRING: VC1 0x004cdd30
        return "DMUS_E_SCRIPT_ROUTINE_NOT_FOUND";
    case static_cast<long>(0x88781220UL):
        // STRING: VC1 0x004cdd0c
        return "DMUS_E_INVALID_SEGMENTTRIGGERTRACK";
    case static_cast<long>(0x88781221UL):
        // STRING: VC1 0x004cdcf0
        return "DMUS_E_INVALID_LYRICSTRACK";
    case static_cast<long>(0x88781222UL):
        // STRING: VC1 0x004cdccc
        return "DMUS_E_INVALID_PARAMCONTROLTRACK";
    case static_cast<long>(0x88781223UL):
        // STRING: VC1 0x004cdca8
        return "DMUS_E_AUDIOVBSCRIPT_SYNTAXERROR";
    case static_cast<long>(0x88781224UL):
        // STRING: VC1 0x004cdc84
        return "DMUS_E_AUDIOVBSCRIPT_RUNTIMEERROR";
    case static_cast<long>(0x88781225UL):
        // STRING: VC1 0x004cdc5c
        return "DMUS_E_AUDIOVBSCRIPT_OPERATIONFAILURE";
    case static_cast<long>(0x88781226UL):
        // STRING: VC1 0x004cdc40
        return "DMUS_E_AUDIOPATHS_NOT_VALID";
    case static_cast<long>(0x88781227UL):
        // STRING: VC1 0x004cdc24
        return "DMUS_E_AUDIOPATHS_IN_USE";
    case static_cast<long>(0x88781228UL):
        // STRING: VC1 0x004cdc08
        return "DMUS_E_NO_AUDIOPATH_CONFIG";
    case static_cast<long>(0x88781229UL):
        // STRING: VC1 0x004cdbec
        return "DMUS_E_AUDIOPATH_INACTIVE";
    case static_cast<long>(0x8878122aUL):
        // STRING: VC1 0x004cdbd0
        return "DMUS_E_AUDIOPATH_NOBUFFER";
    case static_cast<long>(0x8878122bUL):
        // STRING: VC1 0x004cdbb8
        return "DMUS_E_AUDIOPATH_NOPORT";
    case static_cast<long>(0x8878122cUL):
        // STRING: VC1 0x004cdba4
        return "DMUS_E_NO_AUDIOPATH";
    case static_cast<long>(0x8878122dUL):
        // STRING: VC1 0x004cdb90
        return "DMUS_E_INVALIDCHUNK";
    case static_cast<long>(0x8878122eUL):
        // STRING: VC1 0x004cdb6c
        return "DMUS_E_AUDIOPATH_NOGLOBALFXBUFFER";
    case static_cast<long>(0x8878122fUL):
        // STRING: VC1 0x004cdb4c
        return "DMUS_E_INVALID_CONTAINER_OBJECT";
    }
    // STRING: VC1 0x004cd9a8
    return "Unknown";
}

// XOR EAX, EAX; RET.
// FUNCTION: VC1 0x00463c70
int KdDisplayDevice::IsRgba()
{
    return 0;
}

// RET 0x10. DrawQuadBuffer with the PyroDeviceBuffer face of the same KdPyroBuffer.
// FUNCTION: VC1 0x00463c80
int KdDisplayDevice::DrawQuads(PyroVertexBuffer* buffer, int first, int count, int flags)
{
    return DrawQuadBuffer(static_cast<KdPyroBuffer*>(buffer), first, count, flags);
}

// RET 8. Stores value at +0x14.
// FUNCTION: VC1 0x00463cb0
int KdDisplayDevice::SetTexture(void* a, PyroDeviceObject* texture)
{
    (void)a;
    pyroTexture = texture;
    return 0;
}

// XOR EAX, EAX; RET 4.
// FUNCTION: VC1 0x00463cc0
int KdDisplayDevice::SetWorldMatrix(void* a)
{
    (void)a;
    return 0;
}

// MOV EAX, 1; RET 0x10.
// FUNCTION: VC1 0x00463cd0
int KdDisplayDevice::Unused(void* a, void* b, void* c, void* d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return 1;
}

// RET 8. mode18 = 0 for (4, 1), else 2.
// FUNCTION: VC1 0x00463ce0
int KdDisplayDevice::SetBlendMode(unsigned mode, unsigned value)
{
    if (mode == 4 && value == 1) {
        mode18 = 0;
        return 0;
    }
    mode18 = 2;
    return 0;
}

// XOR EAX, EAX; RET 8.
// FUNCTION: VC1 0x00463d10
int KdDisplayDevice::SetRenderState(int a, int b)
{
    (void)a;
    (void)b;
    return 0;
}

// RET 0x10. Draws count quads from quad first of the buffer through the
// current graphics (1 when there is none) with the SetTexture texture and the
// SetBlendMode blend mode, restoring the previous mode.
// FUNCTION: VC1 0x00463d20
int KdDisplayDevice::DrawQuadBuffer(PyroDeviceBuffer* buffer, int first, int count, int flags)
{
    (void)flags;
    if (!g_graphics) {
        return 1;
    }
    KdPtr<ITexture> texture;
    if (pyroTexture) {
        ITexture* const value = static_cast<KdPyroTexture*>(pyroTexture)->texture.ptr;
        if (value) {
            value->AddRef();
            if (texture.ptr) {
                texture.ptr->Release();
            }
        }
        texture.ptr = static_cast<KdPyroTexture*>(pyroTexture)->texture.ptr;
    }
    const unsigned mode = static_cast<IGraphics*>(g_graphics.ptr)->GetMode();
    static_cast<IGraphics*>(g_graphics.ptr)->SetMode(mode18);
    static_cast<IGraphics*>(g_graphics.ptr)->DrawQuads(texture,
        static_cast<unsigned char*>(buffer->GetData()) + first * 0x60, count);
    static_cast<IGraphics*>(g_graphics.ptr)->SetMode(mode);
    return 0;
}

// RET 0x14. Wraps the decoded pixels (adopted) in a device image.
// FUNCTION: VC1 0x00463df0
int KdDisplayDevice::CreateImage(PyroDeviceImage** out, void* pixels, int width, int height,
    int flags)
{
    *out = new PyroDeviceImage(pixels, width, height, flags);
    return 0;
}

// this = IDisplay (+0x04). IDisplay, IShutdownable +4 and IAbstract +8; the
// IAbstract case has no null check in the original.
// FUNCTION: VC1 0x00463e60
void* KdDisplayDevice::QueryInterface(const char** iid)
{
    const char* name = *iid;
    if (name == g_iidDisplay) {
        return static_cast<IDisplay*>(this);
    }
    if (name == g_iidShutdownable) {
        return static_cast<IShutdownable*>(this);
    }
    if (name == g_iidAbstract) {
        return reinterpret_cast<char*>(static_cast<IDisplay*>(this)) + 8;
    }
    return 0;
}

// this = IDisplay. ADD ECX, 8; refs at IAbstract+4; deleting dtor on zero.
// FUNCTION: VC1 0x00463ea0
unsigned KdDisplayDevice::Release()
{
    unsigned value = --refs;
    if (value == 0) {
        delete static_cast<IAbstract*>(this);
        return 0;
    }
    return value;
}

// this = IDisplay. refs at IDisplay+0x0c.
// FUNCTION: VC1 0x00463ec0
unsigned KdDisplayDevice::AddRef()
{
    unsigned value = refs;
    if (value != 0) {
        ++value;
        refs = value;
    }
    return value;
}

// RET 0xc. A KD texture from IDisplay::CreateTexture (QI ITexture; the
// original never null-checks it), filled with the image's 32-bit pixels and
// wrapped in a KdPyroTexture.
// FUNCTION: VC1 0x00463ed0
int KdDisplayDevice::CreateImageTexture(PyroDeviceObject** out, PyroDeviceImage* image, int flags)
{
    IUnknown* created;
    IUnknown** slot;
    ITexture* texture;
    int region[4];
    void* bits;
    KdPyroTexture* object;

    (void)flags;
    slot = reinterpret_cast<IUnknown**>(
        static_cast<IDisplay*>(this)->CreateTexture(reinterpret_cast<void**>(&created)));
    if (!*slot) {
        texture = 0;
    }
    else {
        texture = static_cast<ITexture*>((*slot)->QueryInterface(&g_iidTexture));
        if (texture) {
            texture->AddRef();
        }
    }
    if (created) {
        created->Release();
    }
    region[2] = image->width;
    created = 0;
    region[3] = image->height;
    region[0] = 0;
    region[1] = 0;
    bits = texture->Lock(region, false);
    memcpy(bits, image->pixels, image->width * image->height * 4);
    texture->Unlock();
    object = static_cast<KdPyroTexture*>(malloc(sizeof(KdPyroTexture)));
    *out = object ? ::new (object) KdPyroTexture(KdPtr<ITexture>(texture)) : 0;
    texture->Release();
    return 0;
}

// RET 4. A new KdEffect's IAbstract face (CreateTexture's pattern).
// FUNCTION: VC1 0x00463fc0
void** KdDisplayDevice::CreateEffect(void** out)
{
    KdEffect* object;
    IUnknown* queried;
    IUnknown* face;

    object = static_cast<KdEffect*>(malloc(sizeof(KdEffect)));
    if (!object) {
        queried = 0;
    }
    else {
        new (object) KdEffect();
        if (!object) {
            queried = 0;
        }
        else {
            face = static_cast<IUnknown*>(static_cast<IAbstract*>(object));
            if (!face) {
                queried = 0;
            }
            else {
                queried = static_cast<IUnknown*>(face->QueryInterface(&g_iidAbstract));
            }
        }
    }
    *out = queried;
    if (queried) {
        queried->AddRef();
    }
    queried = static_cast<IUnknown*>(*out);
    queried->Release();
    return out;
}

// this = IShutdownable. Rebind g_graphics from g_graphicsHost, clear +0x14,
// then Done and free the Pyro library.
// FUNCTION: VC1 0x00464010
int KdDisplayDevice::Shutdown()
{
    IUnknown* queried;

    if (g_graphics) {
        g_graphics->Release();
    }
    if (!g_graphicsHost) {
        g_graphics = 0;
    }
    else {
        queried = static_cast<IUnknown*>(g_graphicsHost->QueryInterface(&g_iidGraphics));
        g_graphics = queried;
        if (queried) {
            queried->AddRef();
        }
    }
    pyroTexture = 0;
    if (g_displayPyro) {
        g_displayPyro->Done();
    }
    if (g_displayPyro) {
        free(g_displayPyro);
    }
    g_displayPyro = 0;
    return 0;
}

// RET 0x14. A KdPyroBuffer's PyroVertexBuffer face.
// FUNCTION: VC1 0x004640e0
int KdDisplayDevice::CreateVertexBuffer(PyroVertexBuffer** out, int count, int stride, unsigned format,
    int flags)
{
    KdPyroBuffer* buffer;

    (void)flags;
    buffer = static_cast<KdPyroBuffer*>(malloc(sizeof(KdPyroBuffer)));
    if (buffer) {
        ::new (buffer) KdPyroBuffer(count, stride, format);
        *out = buffer;
        return 0;
    }
    *out = 0;
    return 0;
}

// RET 0x14. A KdPyroBuffer's PyroDeviceBuffer face.
// FUNCTION: VC1 0x00464150
int KdDisplayDevice::CreateBuffer(PyroDeviceBuffer** out, int count, int stride, unsigned format,
    int flags)
{
    KdPyroBuffer* buffer;

    (void)flags;
    buffer = static_cast<KdPyroBuffer*>(malloc(sizeof(KdPyroBuffer)));
    if (buffer) {
        ::new (buffer) KdPyroBuffer(count, stride, format);
        *out = buffer;
        return 0;
    }
    *out = 0;
    return 0;
}

// RET 8. IDisplay vtable+0x20. Builds the Pyro library into g_displayPyro and
// calls its Init with this IPyroDevice base (ADD ESI, -4).
// FUNCTION: VC1 0x004641b0
unsigned char KdDisplayDevice::SetSize(unsigned width, unsigned height)
{
    PyroLibrary* pyro;

    (void)width;
    (void)height;
    pyro = static_cast<PyroLibrary*>(malloc(sizeof(PyroLibrary)));
    if (pyro) {
        new (pyro) PyroLibrary();
    }
    g_displayPyro = pyro;
    if (!pyro) {
        HostLogError(&g_kdNameDisplay, kCantInitPyro);
        return 0;
    }
    pyro->Init(static_cast<IPyroDevice*>(this));
    return 1;
}

// GetWindowLong/SetWindowPos after skip-device change. GWL_STYLE -16, GWL_EXSTYLE -20.
// FUNCTION: VC1 0x00464210
static int UpdateWindowFrame()
{
    LONG style;
    LONG ex;
    const RECT* rect;
    unsigned extra;

    style = GetWindowLongA(g_kdDisplayWindow, GWL_STYLE);
    if (g_displaySkipDevice == 0) {
        rect = &g_displayViewRect;
        extra = g_displayViewStyle;
    }
    else {
        rect = &g_displayWindowRect;
        extra = g_displayWindowStyle;
    }
    SetWindowLongA(g_kdDisplayWindow, GWL_STYLE, (style & WS_VISIBLE) | extra);
    ex = GetWindowLongA(g_kdDisplayWindow, GWL_EXSTYLE);
    if (g_displaySkipDevice != 0) {
        SetWindowLongA(g_kdDisplayWindow, GWL_EXSTYLE, ex & ~WS_EX_TOPMOST);
        return SetWindowPos(g_kdDisplayWindow, HWND_NOTOPMOST,
            rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top,
            SWP_FRAMECHANGED);
    }
    SetWindowLongA(g_kdDisplayWindow, GWL_EXSTYLE, ex | WS_EX_TOPMOST);
    return SetWindowPos(g_kdDisplayWindow, HWND_TOPMOST,
        rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top,
            SWP_FRAMECHANGED);
}

// EAX=D3DFORMAT from the mode record +8. Maps 0x15..0x19 to present intervals.
// FUNCTION: VC1 0x004642e0
static int FormatPresentInterval(unsigned format)
{
    switch (format) {
    case 0x15:
        return 5;
    case 0x16:
        return 4;
    case 0x17:
        return 1;
    case 0x18:
        return 2;
    case 0x19:
        return 3;
    default:
        return 0;
    }
}

// ESI=flags. Inferred name. Applies only the bits that differ from
// g_batchFlags through device +0xc8/+0xfc: 2 -> D3DRS_DESTBLEND INVSRCALPHA
// or ONE, 4 -> D3DRS_ZWRITEENABLE, 1 -> stage 0 D3DTSS_COLOROP ADD or MODULATE.
// FUNCTION: VC1 0x00464330
static void SetBatchFlags(unsigned flags)
{

    if ((flags & 2) != (g_batchFlags & 2)) {
        if (flags & 2) {
            g_d3dDevice->SetRenderState(0x14, 6);
        }
        else {
            g_d3dDevice->SetRenderState(0x14, 2);
        }
    }
    if ((flags & 4) != (g_batchFlags & 4)) {
        if (flags & 4) {
            g_d3dDevice->SetRenderState(0xe, 1);
        }
        else {
            g_d3dDevice->SetRenderState(0xe, 0);
        }
    }
    if ((flags & 1) != (g_batchFlags & 1)) {
        if (flags & 1) {
            g_d3dDevice->SetTextureStageState(0, 1, 7);
        }
        else {
            g_d3dDevice->SetTextureStageState(0, 1, 4);
        }
    }
    g_batchFlags = flags;
}

// cdecl char. Inferred name. Unlocks the vertex buffer, draws the queued
// primitives (DrawPrimitive +0x118 for kinds 2/3, DrawIndexedPrimitive +0x11c
// for quads), then relocks unless this is the final flush before Present.
// FUNCTION: VC1 0x004643f0
static void FlushBatch(char final)
{

    if (g_batchVertices == 0) {
        return;
    }
    g_d3dVertexBuffer->Unlock();
    if (g_batchCount) {
        if (g_batchKind == 2) {
            g_d3dDevice->DrawPrimitive(2, 0, g_batchCount);
        }
        else if (g_batchKind == 3) {
            g_d3dDevice->DrawPrimitive(4, 0, g_batchCount);
        }
        else if (g_batchKind == 4) {
            g_d3dDevice->DrawIndexedPrimitive(4, 0, g_batchCount * 4, 0, g_batchCount * 2);
        }
        g_batchCount = 0;
    }
    if (final == 0) {
        g_d3dVertexBuffer->Lock(0, 0, reinterpret_cast<unsigned char**>(&g_batchVertices), 0);
        return;
    }
    g_batchVertices = 0;
}

// cdecl, EAX=flags, EDI=texture. Inferred name. Starts a fresh quad batch with
// those render flags and texture; returns the locked vertices with room for
// *capacity quads (0 when the buffer is not locked).
// FUNCTION: VC1 0x00464530
void* BeginQuadBatch(unsigned flags, IDirect3DTexture8* texture, int* capacity)
{

    if (g_batchVertices == 0) {
        return 0;
    }
    FlushBatch(0);
    g_batchKind = 4;
    if (g_batchFlags != flags) {
        SetBatchFlags(flags);
    }
    if (texture != g_batchTexture) {
        g_d3dDevice->SetTexture(0, texture);
        g_batchTexture = texture;
    }
    *capacity = 1000;
    return reinterpret_cast<void*>(g_batchVertices);
}

struct BatchLineEntry
{
    BatchVertex from;
    BatchVertex to;
};

// The batch pointer is re-read for every field, as in the original.
static __forceinline BatchLineEntry& BatchLineAt(int index)
{
    return reinterpret_cast<BatchLineEntry*>(g_batchVertices)[index];
}

// cdecl, EDI=color. Inferred name. Appends one untextured line (two 0x18-byte
// vertices at depth z) to a line-list batch, flushing first unless the batch
// already holds lines (fewer than 2000) with flags 2 and no texture.
// FUNCTION: VC1 0x004645a0
void BatchLine(float x0, float y0, float x1, float y1, float z, unsigned color)
{

    if (g_batchVertices == 0) {
        return;
    }
    if (g_batchKind != 2 || g_batchCount >= 2000 || g_batchTexture != 0 || g_batchFlags != 2) {
        FlushBatch(0);
        g_batchKind = 2;
        if (g_batchFlags != 2) {
            SetBatchFlags(2);
        }
        if (g_batchTexture != 0) {
            g_d3dDevice->SetTexture(0, 0);
            g_batchTexture = 0;
        }
    }
    const int index = g_batchCount;
    BatchLineAt(index).from.x = x0;
    g_batchCount = index + 1;
    BatchLineAt(index).to.x = x1;
    BatchLineAt(index).from.y = y0;
    BatchLineAt(index).to.y = y1;
    BatchLineAt(index).to.z = z;
    BatchLineAt(index).from.z = z;
    BatchLineAt(index).to.color = color;
    BatchLineAt(index).from.color = color;
    BatchLineAt(index).to.v = 0.0f;
    BatchLineAt(index).from.v = 0.0f;
    BatchLineAt(index).to.u = 0.0f;
    BatchLineAt(index).from.u = 0.0f;
}

// EAX=matrix. View = world with the 2D axes/origin of matrix, then
// SetTransform(D3DTS_VIEW, g_d3dView) via device vtable+0x94.
// FUNCTION: VC1 0x00464710
void DisplayD3dSetView(const Matrix3* matrix)
{
    int i;

    for (i = 0; i < 16; i++) {
        g_d3dView[i] = Matrix4::Identity.m[i];
    }
    g_d3dView[0] = matrix->m[0];
    g_d3dView[1] = matrix->m[1];
    g_d3dView[4] = matrix->m[3];
    g_d3dView[5] = matrix->m[4];
    g_d3dView[12] = matrix->m[6];
    g_d3dView[13] = matrix->m[7];
    FlushBatch(0);
    g_d3dDevice->SetTransform(2, g_d3dView);
}

// ECX=D3DCOLOR. Clear target, Z=1. Called from DisplayD3DInit with 0.
// FUNCTION: VC1 0x00464780
__declspec(noinline) static void DisplayD3dClear(unsigned color)
{

    g_d3dDevice->Clear(0, 0, 1, color, 1.0f, 0);
}

// ESI=width*, EDI=height*. GetDeviceCaps TextureCaps: 0x20 square, 0x2 pow2.
// FUNCTION: VC1 0x004647a0
static void DisplayD3dPow2Caps(unsigned* w, unsigned* h)
{
    unsigned char caps[0xd4];
    unsigned value;
    unsigned packed;

    if (g_d3dDevice->GetDeviceCaps(caps) < 0) {
        return;
    }
    packed = caps[0x3c];
    if (packed & 0x20) {
        value = *h;
        if (*w > value) {
            value = *w;
        }
        *w = value;
        *h = value;
    }
    if (packed & 0x2) {
        value = 1;
        if (*w > 1) {
            do {
                value = value * 2;
            } while (value < *w);
        }
        *w = value;
        value = 1;
        if (*h > 1) {
            do {
                value = value * 2;
            } while (value < *h);
        }
        *h = value;
    }
}

// ESI=texture. Inferred name. Unlinks the texture's node from g_d3dChain2 (if
// tracked) and releases the texture.
// FUNCTION: VC1 0x00464810
void ReleaseTrackedTexture(IDirect3DTexture8* texture)
{
    TrackedTexture* node = g_d3dChain2;
    TrackedTexture* prev = 0;

    while (node) {
        if (node->texture == texture) {
            if (prev) {
                prev->next = node->next;
            }
            else {
                g_d3dChain2 = node->next;
            }
            free(node);
            break;
        }
        prev = node;
        node = node->next;
    }
    if (texture) {
        texture->Release();
    }
}

// ECX=node. Inferred name. ~KdRenderTarget: unlinks the node from g_d3dChain,
// releases its texture and frees it; a node not on the chain is left alone.
// FUNCTION: VC1 0x00464860
void ReleaseRenderTargetNode(void* node)
{
    KdRenderTargetNode* current = static_cast<KdRenderTargetNode*>(g_d3dChain);
    KdRenderTargetNode* prev = 0;

    if (!current) {
        return;
    }
    while (current != node) {
        prev = current;
        current = current->next;
        if (!current) {
            return;
        }
    }
    if (prev) {
        prev->next = current->next;
    }
    else {
        g_d3dChain = current->next;
    }
    if (current->texture) {
        current->texture->Release();
    }
    free(current);
}

// EDI=quad. Inferred name. Callers: KdGraphics::FillRect (0x0045ccb0) and
// KdGraphics::DrawImage (0x0045cb10). Flushes when the batch kind, the
// 1000-quad limit, the texture or the flags change, then appends the four
// vertices (0x60 bytes) to the locked buffer.
// FUNCTION: VC1 0x004648b0
void PushQuad(const BatchQuad* quad)
{

    if (g_batchVertices == 0) {
        return;
    }
    if (g_batchKind != 4 || g_batchCount >= 1000 || g_batchTexture != quad->texture ||
        g_batchFlags != quad->flags) {
        FlushBatch(0);
        g_batchKind = 4;
        if (g_batchFlags != quad->flags) {
            SetBatchFlags(quad->flags);
        }
        if (g_batchTexture != quad->texture) {
            g_d3dDevice->SetTexture(0, quad->texture);
            g_batchTexture = quad->texture;
        }
    }
    memcpy(reinterpret_cast<unsigned char*>(g_batchVertices) + g_batchCount * 0x60,
        quad->vertices, 0x60);
    g_batchCount++;
}

// Release D3D device, swap chain, surfaces, and linked lists.
// FUNCTION: VC1 0x00464960
static void DisplayD3DTeardown()
{
    void* node;
    void* next;
    IDirect3DTexture8* object;

    while (g_d3dChain2) {
        ReleaseTrackedTexture(g_d3dChain2->texture);
    }

    if (g_d3dBackBuffer) {
        g_d3dBackBuffer->Release();
        g_d3dBackBuffer = 0;
    }
    node = g_d3dChain;
    while (node) {
        object = *reinterpret_cast<IDirect3DTexture8**>(static_cast<unsigned char*>(node) + 8);
        if (object) {
            object->Release();
        }
        next = *reinterpret_cast<void**>(static_cast<unsigned char*>(node) + 0xc);
        free(node);
        node = next;
    }
    g_d3dChain = 0;
    if (g_d3dIndexBuffer) {
        if (g_d3dDevice) {
            g_d3dDevice->SetIndices(0, 0);
        }
        g_d3dIndexBuffer->Release();
        g_d3dIndexBuffer = 0;
    }
    if (g_d3dVertexBuffer) {
        if (g_batchVertices) {
            g_d3dVertexBuffer->Unlock();
            g_batchVertices = 0;
        }
        if (g_d3dDevice) {
            g_d3dDevice->SetStreamSource(0, 0, 0x18);
        }
        g_d3dVertexBuffer->Release();
        g_d3dVertexBuffer = 0;
    }
    if (g_d3dDevice) {
        g_d3dDevice->Release();
        g_d3dDevice = 0;
    }
    if (g_d3dObject) {
        g_d3dObject->Release();
        g_d3dObject = 0;
    }
}

// EAX=bottom, EDX=right, cdecl left/top. Y-flip then ortho into g_d3dProj.
// FUNCTION: VC1 0x00464ab0
static void DisplayD3dSetBuffers(int right, int bottom, int left, int top)
{
    float yflip[16];
    float ortho[16];
    float product[16];
    int width;
    int height;
    int i;

    width = right - left;
    height = bottom - top;
    yflip[0] = 1.0f;
    yflip[1] = 0.0f;
    yflip[2] = 0.0f;
    yflip[3] = 0.0f;
    yflip[4] = 0.0f;
    yflip[5] = -1.0f;
    yflip[6] = 0.0f;
    yflip[7] = 0.0f;
    yflip[8] = 0.0f;
    yflip[9] = 0.0f;
    yflip[10] = 1.0f;
    yflip[11] = 0.0f;
    yflip[12] = -0.5f;
    yflip[13] = 0.5f;
    yflip[14] = 0.0f;
    yflip[15] = 1.0f;
    for (i = 0; i < 16; i++) {
        g_d3dProj[i] = yflip[i];
    }
    ortho[0] = 2.0f / static_cast<float>(width);
    ortho[1] = 0.0f;
    ortho[2] = 0.0f;
    ortho[3] = 0.0f;
    ortho[4] = 0.0f;
    ortho[5] = 2.0f / static_cast<float>(height);
    ortho[6] = 0.0f;
    ortho[7] = 0.0f;
    ortho[8] = 0.0f;
    ortho[9] = 0.0f;
    ortho[10] = 1.0f;
    ortho[11] = 0.0f;
    ortho[12] = -1.0f - (static_cast<float>(left) * 2.0f) / static_cast<float>(width);
    ortho[13] = (static_cast<float>(top) * 2.0f) / static_cast<float>(height) + 1.0f;
    ortho[14] = 0.0f;
    ortho[15] = 1.0f;
    *reinterpret_cast<Matrix4*>(product) =
        *reinterpret_cast<const Matrix4*>(ortho) * *reinterpret_cast<const Matrix4*>(g_d3dProj);
    for (i = 0; i < 16; i++) {
        g_d3dProj[i] = product[i];
    }
}

// cdecl x, y; ECX=width, EAX=height. Clamps to the mixer texture (level 0 desc)
// or g_displayMode, SetViewport (+0xa0), rebuilds g_d3dProj and applies it as
// D3DTS_PROJECTION (+0x94).
// FUNCTION: VC1 0x00464c10
void DisplayD3dSetClip(int x, int y, int width, int height)
{
    D3dSurfaceDesc desc;
    D3dViewport viewport;
    IDirect3DTexture8* texture;
    int maxWidth;
    int maxHeight;

    if (!g_d3dLastMixer) {
        maxWidth = g_displayMode[0];
        maxHeight = g_displayMode[1];
    }
    else {
        texture = *reinterpret_cast<IDirect3DTexture8**>(
            static_cast<unsigned char*>(g_d3dLastMixer) + 8);
        maxWidth = texture->GetLevelDesc(0, &desc) < 0 ? 0 : static_cast<int>(desc.width);
        texture = *reinterpret_cast<IDirect3DTexture8**>(
            static_cast<unsigned char*>(g_d3dLastMixer) + 8);
        maxHeight = texture->GetLevelDesc(0, &desc) < 0 ? 0 : static_cast<int>(desc.height);
    }

    if (x < 0) {
        width += x;
        x = 0;
    }
    if (y < 0) {
        height += y;
        y = 0;
    }
    if (x + width > maxWidth) {
        width = maxWidth - x;
    }
    if (y + height > maxHeight) {
        height = maxHeight - y;
    }

    viewport.x = x;
    viewport.y = y;
    viewport.width = width < 0 ? 0 : width;
    viewport.height = height < 0 ? 0 : height;
    viewport.minZ = 0.0f;
    viewport.maxZ = 1.0f;
    FlushBatch(0);
    if (viewport.width != 0 && viewport.height != 0) {
        g_d3dDevice->SetViewport(&viewport);
    }
    DisplayD3dSetBuffers(viewport.x + viewport.width, viewport.y + viewport.height,
        viewport.x, viewport.y);
    g_d3dDevice->SetTransform(3, g_d3dProj);
}

// GLOBAL: VC1 0x004d248c
extern const char kPercentS[] = "%s";
// STRING: VC1 0x004d5f80
static const char kDxErrorFmt[] = "%s: %s";
// STRING: VC1 0x004d5f88
static const char kDirectXError[] = "DirectX Error";
// STRING: VC1 0x004d5f98
static const char kSceneAlreadyRendered[] = "Gfx_BeginScene: Scene is already being rendered";
// STRING: VC1 0x004d5ff0
static const char kCantCreateRtTexture[] = "Can't create render target texture";
// STRING: VC1 0x004d6170
static const char kCantCreateVb[] = "Can't create D3D vertex buffer";
// STRING: VC1 0x004d6190
static const char kCantCreateIb[] = "Can't create D3D index buffer";
// STRING: VC1 0x004d61b0
static const char kCantLockIb[] = "Can't lock D3D index buffer";
// STRING: VC1 0x004d5ed0
static const char kFmtA8R8G8B8[] = "A8R8G8B8";
// STRING: VC1 0x004d5edc
static const char kFmtX8R8G8B8[] = "X8R8G8B8";
// STRING: VC1 0x004d5ee8
static const char kFmtA1R5G5B5[] = "A1R5G5B5";
// STRING: VC1 0x004d5ef4
static const char kFmtX1R5G5B5[] = "X1R5G5B5";
// STRING: VC1 0x004d5f00
static const char kFmtR5G6B5[] = "R5G6B5";
// STRING: VC1 0x004d5f08
static const char kFmtUnknown[] = "UNKNOWN";
// STRING: VC1 0x004d6090
static const char kCantCreateD3d[] = "Can't create D3D interface";
// STRING: VC1 0x004d60ac
static const char kD3dDriver[] = "D3D Driver: %s";
// STRING: VC1 0x004d60bc
static const char kD3dDescription[] = "Description: %s";
// STRING: VC1 0x004d60cc
static const char kD3dVersion[] = "Version: %d.%d.%d.%d";
// STRING: VC1 0x004d60e4
static const char kCantDesktopMode[] = "Can't determine desktop video mode";
// STRING: VC1 0x004d6108
static const char kCantFindFsMode[] = "Can't find appropriate full screen video mode";
// STRING: VC1 0x004d6138
static const char kFullScreenMode[] = "Full screen mode: %d x %d x %s\n";
// STRING: VC1 0x004d6158
static const char kCantCreateDevice[] = "Can't create D3D device";
// GLOBAL: VC1 0x004e9a0c
static const char* g_d3dFormatNames[6] = {
    kFmtUnknown, kFmtR5G6B5, kFmtX1R5G5B5, kFmtA1R5G5B5, kFmtX8R8G8B8, kFmtA8R8G8B8
};

// EAX=HRESULT, ESI=message. cdecl flag. Optional MB_ICONERROR.
// FUNCTION: VC1 0x00464d60
static void DisplayD3dLogHresult(int hr, const char* message, char flag)
{
    char buf[512];

    if (hr == 0) {
        FormatBufferOpen(buf, kPercentS, message);
    }
    else {
        FormatBufferOpen(buf, kDxErrorFmt, message, DxGetErrorString(hr));
    }
    HostLogError(&g_kdNameDisplay, buf);
    if (flag != 0) {
        MessageBoxA(g_kdDisplayWindow, buf, kDirectXError, MB_ICONHAND);
    }
}

// Recreate VB/IB, chain textures, render states after Reset. Returns AL.
// FUNCTION: VC1 0x00464de0
static unsigned char DisplayD3dRecreate()
{
    void* node;
    IDirect3DTexture8** texture;
    unsigned width;
    unsigned height;
    int hr;
    short* indices;
    short base;
    int i;
    char buf[520];

    g_d3dBackBuffer = 0;
    g_d3dDevice->GetRenderTarget(&g_d3dBackBuffer);
    for (node = g_d3dChain; node;
        node = *reinterpret_cast<void**>(static_cast<unsigned char*>(node) + 0xc)) {
        texture = reinterpret_cast<IDirect3DTexture8**>(static_cast<unsigned char*>(node) + 8);
        if (*texture) {
            width = *static_cast<unsigned*>(node);
            height = *(static_cast<unsigned*>(node) + 1);
            DisplayD3dPow2Caps(&width, &height);
            g_d3dDevice->CreateTexture(width, height, 1, 1,
                g_displayMode ? static_cast<unsigned>(g_displayMode[2]) : 0, 0, texture);
        }
    }
    hr = g_d3dDevice->CreateVertexBuffer(0x17700, 8, 0x142, 0, &g_d3dVertexBuffer);
    if (hr < 0) {
        if (hr == 0) {
            FormatBufferOpen(buf, kPercentS, kCantCreateVb);
        }
        else {
            FormatBufferOpen(buf, kDxErrorFmt, kCantCreateVb, DxGetErrorString(hr));
        }
        HostLogError(&g_kdNameDisplay, buf);
        return 0;
    }
    g_d3dDevice->SetVertexShader(0x142);
    g_d3dDevice->SetStreamSource(0, g_d3dVertexBuffer, 0x18);
    hr = g_d3dDevice->CreateIndexBuffer(0x2ee0, 8, 0x65, 0, &g_d3dIndexBuffer);
    if (hr < 0) {
        if (hr == 0) {
            FormatBufferOpen(buf, kPercentS, kCantCreateIb);
        }
        else {
            FormatBufferOpen(buf, kDxErrorFmt, kCantCreateIb, DxGetErrorString(hr));
        }
        HostLogError(&g_kdNameDisplay, buf);
        return 0;
    }
    hr = g_d3dIndexBuffer->Lock(0, 0, reinterpret_cast<unsigned char**>(&indices), 0);
    if (hr < 0) {
        DisplayD3dLogHresult(hr, kCantLockIb, 0);
        return 0;
    }
    base = 2;
    for (i = 0; i < 1000; i++) {
        indices[0] = static_cast<short>(base - 2);
        indices[1] = static_cast<short>(base - 1);
        indices[2] = base;
        indices[3] = base;
        indices[4] = static_cast<short>(base + 1);
        indices[5] = static_cast<short>(base - 2);
        indices += 6;
        base = static_cast<short>(base + 4);
    }
    g_d3dIndexBuffer->Unlock();
    g_d3dDevice->SetIndices(g_d3dIndexBuffer, 0);
    g_d3dDevice->SetRenderState(0x16, 1);
    g_d3dDevice->SetRenderState(0x89, 0);
    g_d3dDevice->SetRenderState(0x1b, 1);
    g_d3dDevice->SetRenderState(0x13, 5);
    g_d3dDevice->SetRenderState(0x14, 6);
    g_d3dDevice->SetRenderState(0xf, 1);
    g_d3dDevice->SetRenderState(0x18, 1);
    g_d3dDevice->SetRenderState(0x19, 7);
    g_d3dDevice->SetTextureStageState(0, 1, 4);
    g_d3dDevice->SetTextureStageState(0, 2, 2);
    g_d3dDevice->SetTextureStageState(0, 3, 0);
    g_d3dDevice->SetTextureStageState(0, 4, 4);
    g_d3dDevice->SetTextureStageState(0, 5, 2);
    g_d3dDevice->SetTextureStageState(0, 6, 0);
    g_d3dDevice->SetTextureStageState(0, 0x12, 1);
    g_d3dDevice->SetTextureStageState(0, 0x10, 2);
    g_d3dDevice->SetTextureStageState(0, 0x11, 2);
    g_batchCount = 0;
    g_batchKind = 4;
    g_batchFlags = 2;
    g_batchTexture = 0;
    g_d3dDevice->SetTransform(2, g_d3dView);
    g_d3dDevice->SetTransform(3, g_d3dProj);
    return 1;
}

// Release buffers then Reset(g_displayMode); recreate. Returns AL.
// FUNCTION: VC1 0x00465200
static unsigned char DisplayD3dReset()
{
    void* node;
    IDirect3DTexture8* object;

    if (g_d3dBackBuffer) {
        g_d3dBackBuffer->Release();
    }
    for (node = g_d3dChain; node;
        node = *reinterpret_cast<void**>(static_cast<unsigned char*>(node) + 0xc)) {
        object = *reinterpret_cast<IDirect3DTexture8**>(static_cast<unsigned char*>(node) + 8);
        if (object) {
            object->Release();
        }
    }
    if (g_d3dIndexBuffer) {
        g_d3dDevice->SetIndices(0, 0);
        g_d3dIndexBuffer->Release();
    }
    if (g_d3dVertexBuffer) {
        g_d3dDevice->SetStreamSource(0, 0, 0x18);
        g_d3dVertexBuffer->Release();
    }
    g_d3dDevice->Reset(g_displayMode);
    return DisplayD3dRecreate() != 0;
}

// Direct3DCreate8, present params, CreateDevice, Recreate, Clear(0). Returns AL.
// FUNCTION: VC1 0x004652b0
static unsigned char DisplayD3DInit()
{
    D3dDisplayMode desktop;
    D3dDisplayMode enumMode;
    unsigned ident[0x10c];
    char buf[512];
    unsigned* present;
    unsigned format;
    unsigned count;
    unsigned index;
    int hr;
    int i;
    unsigned viewW;
    unsigned viewH;

    g_d3dObject = Direct3DCreate8(0x78);
    if (!g_d3dObject) {
        FormatBufferOpen(buf, kDxErrorFmt, kCantCreateD3d,
            DxGetErrorString(static_cast<int>(0x8876086a)));
        HostLogError(&g_kdNameDisplay, buf);
        MessageBoxA(g_kdDisplayWindow, buf, kDirectXError, MB_ICONHAND);
        return 0;
    }
    g_d3dObject->GetAdapterIdentifier(0, 2, ident);
    HostLog(&g_kdNameDisplay, kD3dDriver, ident);
    HostLog(&g_kdNameDisplay, kD3dDescription, reinterpret_cast<char*>(ident) + 0x200);
    HostLog(&g_kdNameDisplay, kD3dVersion,
        (ident[0x105] >> 16) & 0xffff, ident[0x105] & 0xffff,
        (ident[0x104] >> 16) & 0xffff, ident[0x104] & 0xffff);
    hr = g_d3dObject->GetAdapterDisplayMode(0, &desktop);
    if (hr < 0 || desktop.format == 0) {
        DisplayD3dLogHresult(hr, kCantDesktopMode, g_displaySkipDevice);
        return 0;
    }
    memset(g_d3dPresentWindowed, 0, sizeof(g_d3dPresentWindowed));
    g_d3dPresentWindowed[0] = static_cast<unsigned>(g_displayWidth);
    g_d3dPresentWindowed[1] = static_cast<unsigned>(g_displayHeight);
    g_d3dPresentWindowed[2] = desktop.format;
    g_d3dPresentWindowed[3] = 1;
    g_d3dPresentWindowed[4] = 0;
    g_d3dPresentWindowed[5] = 4;
    g_d3dPresentWindowed[6] = reinterpret_cast<unsigned>(g_kdDisplayWindow);
    g_d3dPresentWindowed[7] = 1;
    if (GetKdEnv(kKdParentWindow)) {
        g_displaySkipDevice = 1;
        present = g_d3dPresentWindowed;
    }
    else {
        viewW = static_cast<unsigned>(g_displayViewRect.right - g_displayViewRect.left);
        viewH = static_cast<unsigned>(g_displayViewRect.bottom - g_displayViewRect.top);
        format = 0;
        if (viewW == desktop.width && viewH == desktop.height) {
            format = desktop.format;
        }
        else {
            count = g_d3dObject->GetAdapterModeCount(0);
            for (index = 0; index < count; index++) {
                g_d3dObject->EnumAdapterModes(0, index, &enumMode);
                if (enumMode.width != viewW || enumMode.height != viewH) {
                    continue;
                }
                if (g_displayPresentInterval == 0x10) {
                    if (FormatPresentInterval(enumMode.format) > FormatPresentInterval(0x19)) {
                        continue;
                    }
                }
                if (FormatPresentInterval(enumMode.format) > FormatPresentInterval(format)) {
                    format = enumMode.format;
                }
            }
            if (format == 0) {
                FormatBufferOpen(buf, kDxErrorFmt, kCantFindFsMode,
                    DxGetErrorString(static_cast<int>(0x88760866)));
                HostLogError(&g_kdNameDisplay, buf);
                if (g_displaySkipDevice == 0) {
                    MessageBoxA(g_kdDisplayWindow, buf, kDirectXError, MB_ICONHAND);
                }
                return 0;
            }
        }
        memset(g_d3dPresentFull, 0, sizeof(g_d3dPresentFull));
        g_d3dPresentFull[0] = viewW;
        g_d3dPresentFull[1] = viewH;
        g_d3dPresentFull[2] = format;
        g_d3dPresentFull[3] = 1;
        g_d3dPresentFull[4] = 0;
        g_d3dPresentFull[5] = 2;
        g_d3dPresentFull[6] = reinterpret_cast<unsigned>(g_kdDisplayWindow);
        g_d3dPresentFull[7] = 0;
        g_d3dPresentFull[11] = 0;
        g_d3dPresentFull[12] = 1;
        HostLog(&g_kdNameDisplay, kFullScreenMode, viewW, viewH,
            g_d3dFormatNames[FormatPresentInterval(format)]);
        if (g_displaySkipDevice == 0) {
            present = g_d3dPresentFull;
        }
        else {
            present = g_d3dPresentWindowed;
        }
    }
    g_displayMode = reinterpret_cast<int*>(present);
    g_displayPresentInterval = ((FormatPresentInterval(present[2]) < 4) - 1 & 0xfffffff0) + 0x20;
    hr = g_d3dObject->CreateDevice(0, 1, g_kdDisplayWindow, 0x20, present, &g_d3dDevice);
    if (hr < 0) {
        DisplayD3dLogHresult(hr, kCantCreateDevice, 1);
        return 0;
    }
    UpdateWindowFrame();
    g_batchVertices = 0;
    g_d3dChain2 = 0;
    DisplayD3dSetBuffers(present[0], present[1], 0, 0);
    for (i = 0; i < 16; i++) {
        g_d3dView[i] = Matrix4::Identity.m[i];
    }
    if (!DisplayD3dRecreate()) {
        return 0;
    }
    DisplayD3dClear(0);
    return 1;
}

// STRING: VC1 0x004d607c
static const char kCantLockTexture[] = "Can't lock texture";

// ESI=texture, ECX=x, EAX=y, cdecl readOnly/width/height. Inferred name.
// Only A8R8G8B8/X8R8G8B8 textures lock; a zero width or height locks the whole
// level. Returns the bits.
// FUNCTION: VC1 0x00465690
void* LockTextureRect(IDirect3DTexture8* texture, int x, int y, bool readOnly, int width, int height)
{
    D3dSurfaceDesc desc;
    RECT rect;
    const RECT* area;
    D3dLockedRect locked;
    int hr;
    char buf[512];

    texture->GetLevelDesc(0, &desc);
    if (desc.format != 0x15 && desc.format != 0x16) {
        return 0;
    }
    if (width && height) {
        rect.left = x;
        rect.top = y;
        rect.right = x + width;
        rect.bottom = y + height;
        area = &rect;
    }
    else {
        area = 0;
    }
    hr = texture->LockRect(0, &locked, area, readOnly ? 0x10 : 0);
    if (hr >= 0) {
        return locked.bits;
    }
    if (hr == 0) {
        FormatBufferOpen(buf, kPercentS, kCantLockTexture);
    }
    else {
        FormatBufferOpen(buf, kDxErrorFmt, kCantLockTexture, DxGetErrorString(hr));
    }
    HostLogError(&g_kdNameDisplay, buf);
    return 0;
}

// GLOBAL: VC1 0x004d240c
extern const char kReadBinary[] = "rb";
// STRING: VC1 0x004d6014
static const char kCantCreateTexture[] = "Can't create texture";
// STRING: VC1 0x004d6044
static const char kUnsupportedTgaDepth[] = "Unsupported tga depth";

// EAX=name, cdecl out width/height (the image size). Inferred name. 24- or
// 32-bit TGA through libtga into a managed A8R8G8B8/X8R8G8B8 texture sized by
// the power-of-two caps; rows are flipped unless the image is top-down and
// 24-bit rows are widened in place.
// FUNCTION: VC1 0x00465770
static IDirect3DTexture8* LoadTgaTexture(const char* name, int* width, int* height)
{
    IDirect3DTexture8* texture;
    unsigned texWidth;
    unsigned texHeight;
    D3dLockedRect locked;
    unsigned char* row;
    int step;
    int y;
    int x;
    int hr;
    char buf[512];

    TGA* const tga = TGAOpen(const_cast<char*>(name), const_cast<char*>(kReadBinary));
    if (!tga) {
        return 0;
    }
    if (tga->last != TGA_OK || TGAReadHeader(tga) != TGA_OK) {
        FormatBufferOpen(buf, kPercentS, TGAStrError(static_cast<tuint8>(tga->last)));
    }
    else if (tga->hdr.depth != 32 && tga->hdr.depth != 24) {
        FormatBufferOpen(buf, kPercentS, kUnsupportedTgaDepth);
    }
    else {
        *width = tga->hdr.width;
        *height = tga->hdr.height;
        texWidth = tga->hdr.width;
        texHeight = tga->hdr.height;
        DisplayD3dPow2Caps(&texWidth, &texHeight);
        hr = g_d3dDevice->CreateTexture(texWidth, texHeight, 1, 0, tga->hdr.depth != 32 ? 0x16 : 0x15, 1,
            &texture);
        if (hr < 0) {
            DisplayD3dLogHresult(hr, kCantCreateTexture, 0);
            TGAClose(tga);
            return 0;
        }
        hr = texture->LockRect(0, &locked, 0, 0);
        if (hr < 0) {
            DisplayD3dLogHresult(hr, kCantCreateTexture, 0);
            texture->Release();
            TGAClose(tga);
            return 0;
        }
        row = static_cast<unsigned char*>(locked.bits);
        step = locked.pitch;
        if (!tga->hdr.vert) {
            row += (*height - 1) * locked.pitch;
            step = -locked.pitch;
        }
        for (y = 0; y < *height; y++) {
            if (TGAReadScanlines(tga, row, y, 1, 0) != 1) {
                break;
            }
            if (tga->hdr.depth == 24) {
                for (x = *width - 1; x >= 0; x--) {
                    reinterpret_cast<unsigned*>(row)[x] =
                        *reinterpret_cast<unsigned*>(row + x * 3) | 0xff000000;
                }
            }
            row += step;
        }
        texture->UnlockRect(0);
        TGAClose(tga);
        return texture;
    }
    HostLogError(&g_kdNameDisplay, buf);
    TGAClose(tga);
    return 0;
}

// STRING: VC1 0x004d602c
static const char kUnsupportedPvrDepth[] = "Unsupported pvr depth";

// Inferred name. The PVR v2 file header (0x34 bytes).
struct PvrHeader
{
    unsigned headerSize; // 0x00
    unsigned height;     // 0x04
    unsigned width;      // 0x08
    unsigned mipCount;   // 0x0c
    unsigned flags;      // 0x10
    unsigned dataSize;   // 0x14
    unsigned bitCount;   // 0x18
    unsigned redMask;    // 0x1c
    unsigned greenMask;  // 0x20
    unsigned blueMask;   // 0x24
    unsigned alphaMask;  // 0x28
    unsigned tag;        // 0x2c "PVR!"
    unsigned surfaces;   // 0x30
};

typedef char Vc1PvrHeaderSizeCheck[sizeof(PvrHeader) == 0x34 ? 1 : -1];

// EAX=name, cdecl out width/height. Inferred name. Only 32-bit PVR files: the
// pixel data is read straight into a managed A8R8G8B8 texture of the file's
// size (no power-of-two fitting, one level).
// FUNCTION: VC1 0x00465990
static IDirect3DTexture8* LoadPvrTexture(const char* name, int* width, int* height)
{
    IDirect3DTexture8* texture;
    D3dLockedRect locked;
    PvrHeader header;
    int hr;
    char buf[512];

    KdFileHandle* const file = static_cast<KdFileHandle*>(OpenKdFile(name, kReadBinary));
    if (!file) {
        return 0;
    }
    if (!file->driver->read(file->driver, &header, sizeof(header), 1, file->native)) {
        file->driver->close(file->driver, file->native);
        free(file);
        return 0;
    }
    if (header.bitCount != 32) {
        FormatBufferOpen(buf, kPercentS, kUnsupportedPvrDepth);
        HostLogError(&g_kdNameDisplay, buf);
        KdFclose(file);
        return 0;
    }
    *width = header.width;
    *height = header.height;
    hr = g_d3dDevice->CreateTexture(header.width, header.height, 1, 0, 0x15, 1, &texture);
    if (hr < 0) {
        DisplayD3dLogHresult(hr, kCantCreateTexture, 0);
        KdFclose(file);
        return 0;
    }
    hr = texture->LockRect(0, &locked, 0, 0);
    if (hr < 0) {
        DisplayD3dLogHresult(hr, kCantCreateTexture, 0);
        texture->Release();
        KdFclose(file);
        return 0;
    }
    KdFread(locked.bits, header.dataSize, 1, file);
    texture->UnlockRect(0);
    KdFclose(file);
    return texture;
}

// cdecl width/height. Inferred name. A managed one-level A8R8G8B8 texture at
// the power-of-two size (KdTexture::Lock's lazy storage).
// FUNCTION: VC1 0x00465b10
IDirect3DTexture8* CreateManagedTexture(unsigned width, unsigned height)
{
    IDirect3DTexture8* texture;
    int hr;
    char buf[512];

    DisplayD3dPow2Caps(&width, &height);
    hr = g_d3dDevice->CreateTexture(width, height, 1, 0, 0x15, 1, &texture);
    if (hr < 0) {
        if (hr == 0) {
            FormatBufferOpen(buf, kPercentS, kCantCreateTexture);
        }
        else {
            FormatBufferOpen(buf, kDxErrorFmt, kCantCreateTexture, DxGetErrorString(hr));
        }
        HostLogError(&g_kdNameDisplay, buf);
        return 0;
    }
    return texture;
}

// cdecl width/height. CreateTexture + GetLevelDesc; link node onto g_d3dChain.
// FUNCTION: VC1 0x00465bc0
void* LookupRenderTargetTexture(unsigned width, unsigned height)
{
    unsigned* node;
    IDirect3DTexture8** texture;
    int hr;
    D3dSurfaceDesc desc;
    char buf[544];

    node = static_cast<unsigned*>(malloc(0x10));
    texture = reinterpret_cast<IDirect3DTexture8**>(node + 2);
    *texture = 0;
    DisplayD3dPow2Caps(&width, &height);
    hr = g_d3dDevice->CreateTexture(width, height, 1, 1,
        g_displayMode ? static_cast<unsigned>(g_displayMode[2]) : 0, 0, texture);
    if (hr < 0) {
        if (hr == 0) {
            FormatBufferOpen(buf, kPercentS, kCantCreateRtTexture);
        }
        else {
            FormatBufferOpen(buf, kDxErrorFmt, kCantCreateRtTexture, DxGetErrorString(hr));
        }
        HostLogError(&g_kdNameDisplay, buf);
        free(node);
        return 0;
    }
    (*texture)->GetLevelDesc(0, &desc);
    node[0] = desc.width;
    node[1] = desc.height;
    node[3] = reinterpret_cast<unsigned>(g_d3dChain);
    g_d3dChain = node;
    return node;
}

// STRING: VC1 0x004d5fc8
static const char kCantSetRenderTarget[] = "Gfx_BeginScene: Can't set render target";

// EBX=node (IRenderTarget::extra; null for the back buffer). Returns AL.
// Inferred name. TestCooperativeLevel / reset, then, when the node differs
// from g_d3dLastMixer, binds its texture's level-0 surface (or the back
// buffer) as the D3D render target with Z off and resets viewport,
// projection and view; BeginScene and lock the batch vertex buffer.
// FUNCTION: VC1 0x00465cc0
static unsigned char DisplayQueryReady(KdRenderTargetNode* node)
{
    IDirect3DSurface8* surface;
    int hr;
    int i;
    char buf[520];

    buf[0] = 0;
    hr = g_d3dDevice->TestCooperativeLevel();
    if (hr == static_cast<int>(0x88760868)) {
        return 0;
    }
    if (hr == static_cast<int>(0x88760869)) {
        if (!DisplayD3dReset()) {
            return 0;
        }
    }
    if (g_batchVertices) {
        FormatBufferOpen(buf, kPercentS, kSceneAlreadyRendered);
        HostLogError(&g_kdNameDisplay, buf);
        return 0;
    }
    if (node != g_d3dLastMixer) {
        if (node) {
            node->texture->GetSurfaceLevel(0, &surface);
        }
        else {
            surface = g_d3dBackBuffer;
        }
        hr = g_d3dDevice->SetRenderTarget(surface, 0);
        if (hr < 0) {
            if (node) {
                surface->Release();
            }
            DisplayD3dLogHresult(hr, kCantSetRenderTarget, 0);
            return 0;
        }
        if (node) {
            surface->Release();
            g_d3dDevice->SetRenderState(7, 0);
            DisplayD3dSetBuffers(static_cast<int>(node->width), static_cast<int>(node->height),
                0, 0);
        }
        else {
            g_d3dDevice->SetRenderState(7, 0);
            DisplayD3dSetBuffers(g_displayMode[0], g_displayMode[1], 0, 0);
        }
        g_d3dDevice->SetTransform(3, g_d3dProj);
        for (i = 0; i < 16; i++) {
            g_d3dView[i] = Matrix4::Identity.m[i];
        }
        g_d3dDevice->SetTransform(2, g_d3dView);
        g_d3dLastMixer = node;
    }
    g_d3dDevice->BeginScene();
    g_d3dVertexBuffer->Lock(0, 0, reinterpret_cast<unsigned char**>(&g_batchVertices), 0);
    return 1;
}

// BL=windowed (SetWindowed argument). Switches g_displaySkipDevice and resets
// the device when the mode actually changes.
// FUNCTION: VC1 0x00465e70
static int ApplyWindowedMode(bool windowed)
{
    int interval;

    if (g_batchVertices || g_displayBusy) {
        return 0;
    }
    if (!g_d3dDevice || g_displaySkipDevice == windowed) {
        g_displaySkipDevice = windowed;
        return 0;
    }
    if (!g_d3dPresentWindowed[2] || !g_d3dPresentFull[2]) {
        return 0;
    }
    if (g_displaySkipDevice) {
        GetWindowRect(g_kdDisplayWindow, &g_displayWindowRect);
    }
    g_displaySkipDevice = windowed;
    g_displayMode = reinterpret_cast<int*>(windowed ? g_d3dPresentWindowed : g_d3dPresentFull);
    interval = FormatPresentInterval(static_cast<unsigned>(g_displayMode[2]));
    g_displayPresentInterval = ((3 < interval) - 1 & 0xfffffff0) + 0x20;
    DisplayD3dReset();
    return UpdateWindowFrame();
}

// cdecl name, out width/height (the image size). Inferred name. jpgd 0.93b
// (jpeg_decoder_file_stream on a KD file, jpeg_decoder 0x00459b00 built on
// the stack) into a managed X8R8G8B8 texture sized by the power-of-two caps;
// the decoder's RGBX scanlines are swizzled to BGRA with alpha 0xff. Any
// decode error after the texture exists releases it.
// FUNCTION: VC1 0x00465f10
static IDirect3DTexture8* LoadJpgTexture(const char* name, int* width, int* height)
{
    IDirect3DTexture8* texture;
    unsigned texWidth;
    unsigned texHeight;
    D3dLockedRect locked;
    unsigned char* row;
    void* line;
    uint lineLen;
    unsigned* dst;
    const unsigned* src;
    const unsigned* end;
    unsigned pixel;
    int y;
    int hr;

    jpeg_decoder_file_stream stream;
    if (stream.open(name)) {
        return 0;
    }
    jpeg_decoder decoder(&stream, false);
    if (decoder.get_error_code() == 0) {
        *width = decoder.get_width();
        *height = decoder.get_height();
        texWidth = *width;
        texHeight = *height;
        DisplayD3dPow2Caps(&texWidth, &texHeight);
        hr = g_d3dDevice->CreateTexture(texWidth, texHeight, 1, 0, 0x16, 1, &texture);
        if (hr < 0) {
            DisplayD3dLogHresult(hr, kCantCreateTexture, 0);
            return 0;
        }
        hr = texture->LockRect(0, &locked, 0, 0);
        if (hr < 0) {
            DisplayD3dLogHresult(hr, kCantCreateTexture, 0);
            texture->Release();
            return 0;
        }
        row = static_cast<unsigned char*>(locked.bits);
        decoder.begin();
        for (y = 0; y < *height; y++) {
            if (decoder.decode(&line, &lineLen)) {
                break;
            }
            dst = reinterpret_cast<unsigned*>(row);
            src = static_cast<const unsigned*>(line);
            end = reinterpret_cast<const unsigned*>(static_cast<const uchar*>(line) + lineLen);
            for (; src < end; src++) {
                pixel = *src;
                *dst++ = ((pixel >> 16) & 0xff) | ((pixel | 0xffffff00) << 16) | (pixel & 0xff00);
            }
            row += locked.pitch;
        }
        texture->UnlockRect(0);
        if (decoder.get_error_code() == 0) {
            return texture;
        }
        texture->Release();
    }
    return 0;
}

// STRING: VC1 0x004d605c
static const char kExtPvr[] = "pvr";
// STRING: VC1 0x004d6060
static const char kExtTga[] = "tga";
// STRING: VC1 0x004d6064
static const char kExtJpg[] = "jpg";
// STRING: VC1 0x004d6068
static const char kUnknownFormat[] = "unknown format: %s";

// EDI=name. Inferred name. Picks the loader by extension and tracks the
// texture with its image size on g_d3dChain2.
// FUNCTION: VC1 0x00466100
IDirect3DTexture8* LoadTextureFile(const char* name)
{
    int width;
    int height;
    IDirect3DTexture8* texture;
    TrackedTexture* node;
    const char* const ext = strchr(name, '.') + 1;

    if (_stricmp(ext, kExtPvr) == 0) {
        texture = LoadPvrTexture(name, &width, &height);
    }
    else if (_stricmp(ext, kExtTga) == 0) {
        texture = LoadTgaTexture(name, &width, &height);
    }
    else if (_stricmp(ext, kExtJpg) == 0) {
        texture = LoadJpgTexture(name, &width, &height);
    }
    else {
        HostLogError(&g_kdNameTexture, kUnknownFormat, ext);
        return 0;
    }
    if (!texture) {
        return 0;
    }
#if VC1_MODS
    // Addon, not original: replace tiles with the enabled texture mods.
    ModTexture(name, texture);
#endif
    node = static_cast<TrackedTexture*>(malloc(sizeof(TrackedTexture)));
    node->texture = texture;
    node->width = width;
    node->height = height;
    node->next = g_d3dChain2;
    g_d3dChain2 = node;
    return texture;
}

// Primary vtable+0x18. Centers the window rect on the primary monitor and
// resets the view rect to the display size.
// FUNCTION: VC1 0x004661e0
int KdDisplayBase::FitWindow()
{
    int width;
    int height;

    GetWindowRect(g_kdDisplayWindow, &g_displayWindowRect);
    width = g_displayWindowRect.right - g_displayWindowRect.left;
    height = g_displayWindowRect.bottom - g_displayWindowRect.top;
    g_displayWindowRect.left = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
    g_displayWindowRect.top = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
    g_displayWindowRect.bottom = g_displayWindowRect.top + height;
    g_displayWindowRect.right = g_displayWindowRect.left + width;
    g_displayWindowStyle = 0xca0000;
    g_displayViewRect.left = 0;
    g_displayViewRect.top = 0;
    g_displayViewRect.right = g_displayWidth;
    g_displayViewRect.bottom = g_displayHeight;
    g_displayViewStyle = 0;
    return g_displayHeight;
}

// MOV EAX, [0x004e8998]; RET. IRenderTarget vtable+0x18; IDisplay via WBA.
// FUNCTION: VC1 0x00466290
int KdDisplayBase::GetHeight()
{
    return g_displayHeight;
}

// MOV EAX, [0x004e8744]; RET. IRenderTarget vtable+0x14; IDisplay via WBA.
// FUNCTION: VC1 0x004662a0
int KdDisplayBase::GetWidth()
{
    return g_displayWidth;
}

// this = IDisplay (+0x14). mixer at IDisplay+0x18 = object+0x2c.
// FUNCTION: VC1 0x004662b0
void* KdDisplayBase::GetMixer()
{
    return mixer;
}

// MOV AL, [0x004e74b6]; RET. IDisplay vtable+0x24.
// FUNCTION: VC1 0x004662c0
bool KdDisplayBase::IsWindowed()
{
    return g_displaySkipDevice;
}

// XOR AL, AL; RET 8. IScriptObject vtable+0x10.
// FUNCTION: VC1 0x004662d0
unsigned char KdDisplayBase::SetMember(void* a, void* b)
{
    (void)a;
    (void)b;
    return 0;
}

// Guarded static IUnknown*. IRenderTarget vtable+0x1c; IDisplay thunk SUB ECX, 0x10.
// FUNCTION: VC1 0x004662e0
void* KdDisplayBase::GetRenderTexture()
{
    if ((g_nullRenderTextureGuard & 1) == 0) {
        g_nullRenderTextureGuard |= 1;
        g_nullRenderTexture = 0;
        atexit(ReleaseNullRenderTexture);
    }
    return &g_nullRenderTexture;
}

// RET 4. IDisplay vtable+0x30.
// FUNCTION: VC1 0x00466310
void KdDisplayBase::ShowKeyboard(bool show)
{
    (void)show;
}

// IScriptObject this, IRenderTarget +4, then KdDisplayDevice QI (IDisplay this),
// then IAbstract +0x1c.
// FUNCTION: VC1 0x00466320
void* KdDisplayBase::QueryInterface(const char** iid)
{
    void* found;
    const char* name = *iid;

    if (name == g_iidScriptObject) {
        return static_cast<IScriptObject*>(this);
    }
    if (name == g_iidRenderTarget) {
        return static_cast<IRenderTarget*>(this);
    }
    found = KdDisplayDevice::QueryInterface(iid);
    if (found) {
        return found;
    }
    if (name == g_iidAbstract) {
        return static_cast<IAbstract*>(this);
    }
    return 0;
}

// ADD ECX, 0x1c; refs at IAbstract+4; deleting dtor on zero.
// FUNCTION: VC1 0x00466380
unsigned KdDisplayBase::Release()
{
    unsigned value = --refs;
    if (value == 0) {
        delete static_cast<IAbstract*>(this);
        return 0;
    }
    return value;
}

// refs at +0x20.
// FUNCTION: VC1 0x004663a0
unsigned KdDisplayBase::AddRef()
{
    unsigned value = refs;
    if (value != 0) {
        ++value;
        refs = value;
    }
    return value;
}

// malloc 0x24, construct, QI IAbstract at +0x14. RET 4. IDisplay vtable+0x34.
// FUNCTION: VC1 0x004663b0
void** KdDisplayBase::CreateTexture(void** out)
{
    KdTexture* object;
    IUnknown* queried;
    IUnknown* face;

    object = static_cast<KdTexture*>(malloc(sizeof(KdTexture)));
    if (!object) {
        queried = 0;
    }
    else {
        new (object) KdTexture();
        if (!object) {
            queried = 0;
        }
        else {
            face = static_cast<IUnknown*>(static_cast<IAbstract*>(object));
            if (!face) {
                queried = 0;
            }
            else {
                queried = static_cast<IUnknown*>(face->QueryInterface(&g_iidAbstract));
            }
        }
    }
    *out = queried;
    if (queried) {
        queried->AddRef();
    }
    queried = static_cast<IUnknown*>(*out);
    queried->Release();
    return out;
}

// this = IShutdownable (+0x18). KdDisplayDevice::Shutdown, D3D teardown, then
// drop the window's kdInstallCallback record (inlined) and destroy it.
// FUNCTION: VC1 0x00466400
int KdDisplayBase::Shutdown() throw()
{
    unsigned char* window;
    HWND hwnd;
    KdCallback* node;

    KdDisplayDevice::Shutdown();
    DisplayD3DTeardown();

    window = static_cast<unsigned char*>(mixer);
    if (!window) {
        return 0;
    }

    node = g_kdCallbacks;
    while (node->next) {
        if (node->type == 0 && node->userptr == window) {
            if (g_kdCallbacks == node) {
                g_kdCallbacks = node->next;
            }
            else {
                node->prev->next = node->next;
            }
            free(node);
            break;
        }
        node = node->next;
    }

    hwnd = *reinterpret_cast<HWND*>(window + 4);
    KillTimer(hwnd, 1);
    SetWindowLongA(hwnd, GWL_USERDATA, 0);
    free(window);
    DestroyWindow(hwnd);
    mixer = 0;
    return 0;
}

// ESI=this. Own vptrs, Shutdown(), then the inlined ~KdDisplayDevice
// (g_kdDisplay = 0) and ~IRenderTarget (release graphics).
// FUNCTION: VC1 0x004664a0
KdDisplayBase::~KdDisplayBase()
{
    KdDisplayBase::Shutdown();
}

// Window callback installed by SetSize for every event of the window:
// KD_EVENT_WINDOW_CLOSE (0x2c) logs and posts KD_EVENT_QUIT, focus (0x3c)
// posts KD_EVENT_RESUME or KD_EVENT_PAUSE. Inferred name.
// FUNCTION: VC1 0x00466510
static void DisplayEventCallback(const KdEvent* event)
{
    KdEvent* posted;

    if (event->type == 0x2c) {
        // STRING: VC1 0x004d5dd0
        HostLog(&g_kdNameDisplay, "close window");
        posted = AllocEvent();
        posted->type = KD_EVENT_QUIT;
        QueueEvent(posted);
    }
    else if (event->type == 0x3c) {
        posted = AllocEvent();
        posted->type = event->data[0] != 0 ? KD_EVENT_RESUME : KD_EVENT_PAUSE;
        QueueEvent(posted);
    }
}

// IRenderTarget vtable+0x10. Rebind the graphics scene, end and present.
// FUNCTION: VC1 0x00466570
void* KdDisplayBase::EndFrame()
{

    static_cast<KdGraphics*>(graphics)->BindScene();
    FlushBatch(1);
    g_d3dDevice->EndScene();
    if (g_d3dLastMixer == 0) {
        g_d3dDevice->Present(0, 0, 0, 0);
    }
    return 0;
}

// ESI=this, EDI=IRenderTarget. IRenderTarget() and the inlined KdDisplayDevice()
// run first; then own vptrs and mixer = 0.
// FUNCTION: VC1 0x004665c0
KdDisplayBase::KdDisplayBase()
{
    mixer = 0;
}

// RET 8. IRenderTarget::BeginFrame(a, b); return a.
// FUNCTION: VC1 0x00466640
void* KdDisplayBase::BeginFrame(void* a, void* b)
{
    IRenderTarget::BeginFrame(a, b);
    return a;
}

// RET 0xc. IDisplay vtable+0x38. Loads 3 stdcall args then cdecl-calls 0x0045bf40.
// FUNCTION: VC1 0x00466660
KdPtr<IRenderTarget> KdDisplayBase::CreateRenderTarget(unsigned width, unsigned height)
{
    KdPtr<IRenderTarget> result;
    CreateRenderTargetBody(reinterpret_cast<void**>(&result.ptr), width, height);
    return result;
}

// RET 4. BL=arg into ApplyWindowedMode. The script closure (0x00466830) reads
// the argument with SqPlus Get(TypeWrapper<bool>), so scripts pass bools. No
// result: the closure is shared with ShowKeyboard as void(bool).
// FUNCTION: VC1 0x00466680
void KdDisplayBase::SetWindowed(bool arg)
{
    ApplyWindowedMode(arg);
}

// this=IDisplay. HostLog Init, CreateKdWindow into mixer, ShowWindow, D3D init,
// then KdDisplayDevice::SetSize (Pyro).
// FUNCTION: VC1 0x00466690
unsigned char KdDisplayBase::SetSize(unsigned width, unsigned height)
{
    unsigned char* window;
    HWND hwnd;
    RECT client;

    HostLog(&g_kdNameDisplay, kInitSize, width, height);
    mixer = CreateKdWindow();
    window = static_cast<unsigned char*>(mixer);
    if (!window) {
        return 0;
    }

    hwnd = *reinterpret_cast<HWND*>(window + 4);
    if (GetKdEnv(kKdParentWindow)) {
        if (GetClientRect(hwnd, &client)) {
            width = static_cast<unsigned>(client.right - client.left);
            height = static_cast<unsigned>(client.bottom - client.top);
        }
    }
    else {
        int frameY;
        int caption;
        int frameX;
        *reinterpret_cast<unsigned*>(window + 0x10) = width;
        *reinterpret_cast<unsigned*>(window + 0x14) = height;
        frameY = GetSystemMetrics(SM_CYFIXEDFRAME);
        caption = GetSystemMetrics(SM_CYCAPTION);
        frameX = GetSystemMetrics(SM_CXDLGFRAME);
        SetWindowPos(hwnd, 0, 0, 0,
            static_cast<int>(width) + frameX * 2,
            caption + frameY * 2 + static_cast<int>(height),
            SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    }

    SetWindowTextA(hwnd, kVirtualCity);
    KdInstallCallback(DisplayEventCallback, 0, mixer);
    g_kdDisplayWindow = hwnd;
    ShowWindow(hwnd, SW_SHOWNORMAL);
    g_displayWidth = static_cast<int>(width);
    g_displayHeight = static_cast<int>(height);

    if (!GetKdEnv(kKdParentWindow)) {
        FitWindow();
        SetWindowPos(g_kdDisplayWindow, 0, g_displayWindowRect.left, g_displayWindowRect.top, 0, 0,
            SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    }

    if (!DisplayD3DInit()) {
        HostLogError(&g_kdNameDisplay, kCantInit);
        return 0;
    }
    return KdDisplayDevice::SetSize(width, height);
}

// RET 8. Script method lookup (table 0x004ee5ec, guard bit 0x004ee5fc,
// atexit destructor 0x004cacd0). "GetHeight" is bound to the same vcall
// thunk as "GetWidth" (0x0041be20, vtable+0x14) in the original.
// FUNCTION: VC1 0x00466b00
unsigned char KdDisplayBase::GetMember(const char* name, SquirrelObject* out)
{
    static ScriptMethodTable methods;

    if (methods.Empty()) {
        // STRING: VC1 0x004d5d7c
        methods["CreateRenderTarget"] = CreateScriptMethod<KdDisplayBase>(&KdDisplayBase::CreateRenderTarget);
        methods[kScriptGetWidth] = CreateScriptMethod<KdDisplayBase>(&KdDisplayBase::GetWidth);
        methods[kScriptGetHeight] = CreateScriptMethod<KdDisplayBase>(&KdDisplayBase::GetWidth);
        // STRING: VC1 0x004d5d90
        methods["IsWindowed"] = CreateScriptMethod<KdDisplayBase>(&KdDisplayBase::IsWindowed);
        // STRING: VC1 0x004d5d9c
        methods["SetWindowed"] = CreateScriptMethod<KdDisplayBase>(&KdDisplayBase::SetWindowed);
        // STRING: VC1 0x004d5da8
        methods["ShowKeyboard"] = CreateScriptMethod<KdDisplayBase>(&KdDisplayBase::ShowKeyboard);
    }
    return methods.Find(name, out);
}

// Primary vtable+0x18. Base recenter, then enabled58 = window fits the work
// area at the current desktop mode.
// FUNCTION: VC1 0x004b1c90
int KdDisplay::FitWindow()
{
    DEVMODEA mode;
    RECT work;

    KdDisplayBase::FitWindow();
    EnumDisplaySettingsA(0, ENUM_CURRENT_SETTINGS, &mode);
    g_displayViewRect.right = static_cast<int>(mode.dmPelsWidth);
    g_displayViewRect.bottom = static_cast<int>(mode.dmPelsHeight);
    SystemParametersInfoA(SPI_GETWORKAREA, 0, &work, 0);
    if (g_displayWindowRect.right - g_displayWindowRect.left <= work.right - work.left
        && g_displayWindowRect.bottom - g_displayWindowRect.top <= work.bottom - work.top) {
        enabled58 = 1;
        return 1;
    }
    enabled58 = 0;
    return 0;
}

// FUNCTION: VC1 0x004b1d30
unsigned char KdDisplay::SetMember(void* a, void* b)
{
    (void)a;
    (void)b;
    return 0;
}

// FUNCTION: VC1 0x004b1d40
void* KdDisplay::QueryInterface(const char** iid)
{
    void* found = KdDisplayBase::QueryInterface(iid);
    if (found) {
        return found;
    }
    if (*iid == g_iidAbstract) {
        return static_cast<IAbstract*>(this);
    }
    return 0;
}

// FUNCTION: VC1 0x004b1d70
unsigned KdDisplay::Release()
{
    IAbstract* iface = static_cast<IAbstract*>(this);
    unsigned n;

    iface->refs -= 1;
    n = iface->refs;
    if (n == 0) {
        delete iface;
        return 0;
    }
    return n;
}

// FUNCTION: VC1 0x004b1d90
unsigned KdDisplay::AddRef()
{
    unsigned n = refs;
    if (n != 0) {
        n += 1;
        refs = n;
    }
    return n;
}

// this = IShutdownable (+0x18). KdDisplayBase::Shutdown, then rebind target
// from g_kdQueryUnknown QI IRenderTarget.
// FUNCTION: VC1 0x004b1da0
int KdDisplay::Shutdown() throw()
{
    IUnknown* queried;

    KdDisplayBase::Shutdown();
    if (target) {
        target->Release();
    }
    if (!g_kdQueryUnknown) {
        target = 0;
        return 0;
    }
    queried = static_cast<IUnknown*>(g_kdQueryUnknown->QueryInterface(&g_iidRenderTarget));
    target = queried;
    if (!queried) {
        return 0;
    }
    return static_cast<int>(queried->AddRef());
}

// EAX=this. Calls KdDisplayBase() (0x004665c0, ESI=this), own vptrs, then
// target = 0, identity matrix (Matrix3()) and enabled58 = 1.
// FUNCTION: VC1 0x004b1e40
KdDisplay::KdDisplay() throw()
    : target(0), enabled58(1)
{
}

// FUNCTION: VC1 0x004b1ea0
void** CreateKdDisplay(void** out)
{
    IUnknown* queried;
    KdDisplay* object;

    if (!g_kdDisplay) {
        object = static_cast<KdDisplay*>(malloc(sizeof(KdDisplay)));
        if (object) {
            object = new (object) KdDisplay();
        }
        g_kdDisplay = static_cast<IAbstract*>(object);
        if (g_kdDisplay) {
            queried = static_cast<IUnknown*>(g_kdDisplay->QueryInterface(&g_iidAbstract));
        }
        else {
            queried = 0;
        }
        if (queried) {
            queried->AddRef();
        }
        KdRelease(queried);
        *out = queried;
        if (queried) {
            queried->AddRef();
        }
        KdRelease(queried);
        return out;
    }

    queried = static_cast<IUnknown*>(g_kdDisplay->QueryInterface(&g_iidAbstract));
    *out = queried;
    if (queried) {
        queried->AddRef();
    }
    return out;
}

// this = IRenderTarget. If target is set, draws it into this display's
// graphics through matrix, then rebinds and presents like KdDisplayBase.
// FUNCTION: VC1 0x004b1f50
void* KdDisplay::EndFrame()
{
    IRenderTarget* source;
    IUnknown* camera;
    IGraphics* view;

    if (g_displaySkipDevice == 0) {
        source = static_cast<IRenderTarget*>(static_cast<void*>(target));
        if (source) {
            source->EndFrame();
            if (g_kdQueryUnknown) {
                camera = static_cast<IUnknown*>(
                    g_kdQueryUnknown->QueryInterface(&g_iidCamera));
                if (camera) {
                    camera->AddRef();
                }
            }
            else {
                camera = 0;
            }
            IRenderTarget::BeginFrame(&view, &camera);
            if (camera) {
                camera->Release();
            }
            if (!view) {
                return 0;
            }
            view->SetTransform(matrix);
            Rect clip(*reinterpret_cast<Rect*>(&g_displayViewRect));
            view->SetClipRect(clip);
            view->Clear(0xff000000);
            view->DrawImage(*static_cast<const KdPtr<ITexture>*>(static_cast<IRenderTarget*>(
                static_cast<void*>(target))->GetRenderTexture()), 0.0f, 0.0f, 0.0f, 0.0f,
                static_cast<float>(g_displayWidth), static_cast<float>(g_displayHeight));
            view->Release();
        }
    }

    static_cast<KdGraphics*>(graphics)->BindScene();
    FlushBatch(1);
    g_d3dDevice->EndScene();
    if (g_d3dLastMixer == 0) {
        g_d3dDevice->Present(0, 0, 0, 0);
    }
    return 0;
}

// this = IRenderTarget. RET 8. Forwards to target when drawing offscreen.
// FUNCTION: VC1 0x004b20c0
void* KdDisplay::BeginFrame(void* a, void* b)
{
    IRenderTarget* source = static_cast<IRenderTarget*>(static_cast<void*>(target));

    if (g_displaySkipDevice == 0 && source) {
        source->BeginFrame(a, b);
        return a;
    }
    IRenderTarget::BeginFrame(a, b);
    return a;
}

// this = IDisplay. Leaving windowed mode needs enabled58 (window fits). RET 4.
// FUNCTION: VC1 0x004b2100
void KdDisplay::SetWindowed(bool arg)
{
    if (arg == 0 || enabled58 != 0) {
        ApplyWindowedMode(arg);
    }
}

// this = IDisplay. Calls 0x00466690 then rebuilds the Matrix3 at IDisplay+0x20.
// FUNCTION: VC1 0x004b2120
unsigned char KdDisplay::SetSize(unsigned width, unsigned height)
{
    int viewW;
    int viewH;
    float scaleX;
    float scaleY;
    float scale;

    if (!KdDisplayBase::SetSize(width, height)) {
        return 0;
    }
    if (GetKdEnv(kKdParentWindow)) {
        return 1;
    }

    viewW = g_displayViewRect.right - g_displayViewRect.left;
    viewH = g_displayViewRect.bottom - g_displayViewRect.top;
    if (g_displayWidth == viewW && g_displayHeight == viewH) {
        return 1;
    }

    {
        KdPtr<IRenderTarget> slot(CreateRenderTarget(g_displayWidth, g_displayHeight));
        DisplayAssignTarget(reinterpret_cast<void**>(&slot.ptr), &target);
    }
    if (!target) {
        HostLogError(&g_kdNameDisplay, kCantCreateRt);
        return 0;
    }

    scaleX = static_cast<float>(viewW) / static_cast<float>(g_displayWidth);
    scaleY = static_cast<float>(viewH) / static_cast<float>(g_displayHeight);
    scale = MinFloat(scaleX, scaleY);
    matrix = Matrix3::Identity;
    matrix.m[0] *= scale;
    matrix.m[1] *= scale;
    matrix.m[3] *= scale;
    matrix.m[4] *= scale;
    if (scaleY < scaleX) {
        matrix.m[6] = (static_cast<float>(viewW) - static_cast<float>(g_displayWidth) * scale)
            * 0.5f;
        matrix.m[7] = 0.0f;
    }
    else {
        matrix.m[6] = 0.0f;
        matrix.m[7] = (static_cast<float>(viewH) - static_cast<float>(g_displayHeight) * scale)
            * 0.5f;
    }
    g_displayMatrixInv = Matrix3::Inverse(matrix);
    return 1;
}

// RET 8. Only the base table (0x00466b00); the result is normalized to 0/1.
// FUNCTION: VC1 0x004b22e0
unsigned char KdDisplay::GetMember(const char* name, SquirrelObject* out)
{
    return KdDisplayBase::GetMember(name, out) != 0;
}

// Release the GetRenderTexture static if set. Registered with atexit from 0x004662e0.
// FUNCTION: VC1 0x004cad30
static void ReleaseNullRenderTexture()
{
    IUnknown* object = g_nullRenderTexture;
    if (object) {
        object->Release();
    }
    g_nullRenderTexture = 0;
}

}
