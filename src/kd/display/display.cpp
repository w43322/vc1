#include "kd/display/display.h"
#include "kd/display/graphics.h"
#include "kd/display/pyro.h"
#include "kd/display/render_target.h"
#include "kd/display/texture.h"
#include "kd/factory.h"

#include "kd/iface/iunknown.h"
#include "kd/iface/iid.h"
#include "host/runtime.h"
#include "bindings/matrix4.h"
#include "bindings/matrix3.h"

#include <errno.h>
#include <new>
#include <stdlib.h>
#include <string.h>

#pragma function(memcpy)

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
// GLOBAL: VC1 0x004ed590
int g_displayWindowX;
// GLOBAL: VC1 0x004ed594
int g_displayWindowY;
// GLOBAL: VC1 0x004ed598
int g_displayWindowRight;
// GLOBAL: VC1 0x004ed59c
int g_displayWindowBottom;
// GLOBAL: VC1 0x004ed5a0
unsigned g_displayWindowStyle;
// GLOBAL: VC1 0x004ed5d8
int g_displayViewLeft;
// GLOBAL: VC1 0x004ed5dc
int g_displayViewTop;
// GLOBAL: VC1 0x004ed5e0
int g_displayViewRight;
// GLOBAL: VC1 0x004ed5e4
int g_displayViewBottom;
// GLOBAL: VC1 0x004ed5e8
unsigned g_displayViewStyle;
// GLOBAL: VC1 0x004ed5f0
IUnknown* g_d3dDevice;
// GLOBAL: VC1 0x004ed5ec
IUnknown* g_d3dObject;
// Inferred name. CreateVertexBuffer(0x17700, D3DUSAGE_WRITEONLY, FVF 0x142).
// GLOBAL: VC1 0x004ed5f4
IUnknown* g_d3dVertexBuffer;
// GLOBAL: VC1 0x004ed5f8
IUnknown* g_d3dSurface;
// GLOBAL: VC1 0x004ed5fc
IUnknown* g_d3dBackBuffer;
// GLOBAL: VC1 0x004ed600
void* g_d3dChain;
// GLOBAL: VC1 0x004ed604
void* g_d3dLastMixer;
// GLOBAL: VC1 0x004ed608
void* g_d3dChain2;
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
IUnknown* g_batchTexture;
// GLOBAL: VC1 0x004e960c
Matrix3 g_displayMatrixInv;
// GLOBAL: VC1 0x004e9788
float g_d3dWorld[16];
// GLOBAL: VC1 0x004e98c0
float g_d3dView[16];
// GLOBAL: VC1 0x004e9900
float g_d3dProj[16];
// GLOBAL: VC1 0x004ed700
IUnknown* g_displayScene;
// GLOBAL: VC1 0x004ed6ec
IUnknown* g_graphicsQuery;
// GLOBAL: VC1 0x004ed628
PyroLibrary* g_displayPyro;
// GLOBAL: VC1 0x004ed65c
IUnknown* g_kdQueryUnknown;
// GLOBAL: VC1 0x004ed6f0
IUnknown* g_graphics;
// GLOBAL: VC1 0x004ed6f4
IUnknown* g_graphicsHost;
// GLOBAL: VC1 0x004e74b6
unsigned char g_displaySkipDevice;
// GLOBAL: VC1 0x004e8744
int g_displayWidth;
// GLOBAL: VC1 0x004e8998
int g_displayHeight;
// GLOBAL: VC1 0x004e899c
int g_displayPresentInterval;
// GLOBAL: VC1 0x004ed450
void* g_kdWindowMixer;
// GLOBAL: VC1 0x004e95a0
WNDCLASSA g_kdDisplayClass;

// STRING: VC1 0x004d4c38
static const char kTexture[] = "Texture";
// STRING: VC1 0x004d4c4c
static const char kDisplay[] = "Display";
// GLOBAL: VC1 0x004d0b38
extern const char kKdParentWindow[] = "KD_PARENT_WINDOW";
// GLOBAL: VC1 0x004d0630
const char* g_kdNameDisplay = kDisplay;
// GLOBAL: VC1 0x004d0638
const char* g_kdNameTexture = kTexture;
// STRING: VC1 0x004d5db8
static const char kInitSize[] = "Init: %dx%d";
// STRING: VC1 0x004d5dc4
static const char kCantInit[] = "Can't init";
// STRING: VC1 0x004d0b4c
static const char kCantCreateRt[] = "Can't create render target for full screen";
// STRING: VC1 0x004d2854
static const char kWindowName[] = "Window";
// STRING: VC1 0x004d61ec
static const char kCantInitPyro[] = "Can't init Pyro";

// GLOBAL: VC1 0x004ee284
IUnknown* g_display1cObject;
// GLOBAL: VC1 0x004ee288
unsigned g_display1cGuard;

static void Display1cAtexit();
static unsigned char DisplayQueryReady();
static int Display28Finish();
static int Display28Mode(unsigned format);
static const char* DxGetErrorString(int hr);
static void DisplayD3dPow2Caps(unsigned* w, unsigned* h);
static void FlushBatch(char final);
static void DisplayD3DTeardown();
static void DisplayD3dSetBuffers(int right, int bottom, int left, int top);
static void DisplayD3dLogHresult(int hr, char flag);
static unsigned char DisplayD3dRecreate();
static unsigned char DisplayD3dReset();
static unsigned char DisplayD3DInit();
static int Display28Apply(unsigned char windowed);
static WNDCLASSA* GetKdWindowClass();
static void* CreateKdWindow();
static IUnknown** DisplayAssignTarget(void** out, IUnknown** slot);
static float MinFloat(float a, float b);

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
static float MinFloat(float a, float b)
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
void* IRenderTarget::Display0c(void* a, void* b)
{
    IUnknown** out = static_cast<IUnknown**>(a);
    IUnknown* queried;
    Rect rect;

    if (!DisplayQueryReady()) {
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

// Uses graphics at +0x08, updates the graphics device, then clears if needed.
// STUB: VC1 0x0041a600
void* IRenderTarget::Display10()
{
    return 0;
}

// MOV EAX, 0x004d4c4c; RET. IScriptObject vtable+0x14.
// FUNCTION: VC1 0x0041a650
const char* KdDisplayBase::Script14()
{
    return kDisplay;
}

// MOV EAX, 1; RET 0x14.
// FUNCTION: VC1 0x0041aca0
int IPyroDevice::Pyro08(void** out, void* b, void* c, void* d, void* e)
{
    (void)out;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    return 1;
}

// RET 0x14. Pyro08 into a local (reusing the last argument slot); on success
// wraps it in an 8-byte object (vftable 0x004d6278, value at +4) into *out.
// The wrapper class is not recovered yet.
// STUB: VC1 0x0041acb0
int IPyroDevice::Pyro0c(void** out, void* b, void* c, void* d, void* e)
{
    void* inner;
    int result;

    result = Pyro08(&inner, b, c, d, e);
    if (result) {
        return result;
    }
    *out = 0;
    return 0;
}

// MOV EAX, 1; RET 0x10.
// FUNCTION: VC1 0x0041ad10
int IPyroDevice::Pyro24(void* a, void* b, void* c, void* d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return 1;
}

// RET 0x10. Pyro24(a->+4, b, c, d) through the 0x004d6278 wrapper; the wrapper
// class is not recovered yet.
// STUB: VC1 0x0041ad20
int IPyroDevice::Pyro28(void* a, void* b, void* c, void* d)
{
    return Pyro24(a, b, c, d);
}

// EAX=HRESULT. DXGetErrorString-style lookup; huge table, recovered as a stub.
// STUB: VC1 0x00442e14
static const char* DxGetErrorString(int hr)
{
    (void)hr;
    return "";
}

// XOR EAX, EAX; RET.
// FUNCTION: VC1 0x00463c70
int KdDisplayDevice::Pyro2c()
{
    return 0;
}

// RET 0x10. Pyro24(a ? a - 4 : 0, b, c, d) through vtable+0x24.
// FUNCTION: VC1 0x00463c80
int KdDisplayDevice::Pyro28(void* a, void* b, void* c, void* d)
{
    void* inner = a ? static_cast<char*>(a) - 4 : 0;
    return Pyro24(inner, b, c, d);
}

// RET 8. Stores value at +0x14.
// FUNCTION: VC1 0x00463cb0
int KdDisplayDevice::Pyro20(void* a, unsigned value)
{
    (void)a;
    value14 = value;
    return 0;
}

// XOR EAX, EAX; RET 4.
// FUNCTION: VC1 0x00463cc0
int KdDisplayDevice::Pyro1c(void* a)
{
    (void)a;
    return 0;
}

// MOV EAX, 1; RET 0x10.
// FUNCTION: VC1 0x00463cd0
int KdDisplayDevice::Pyro10(void* a, void* b, void* c, void* d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return 1;
}

// RET 8. mode18 = 0 for (4, 1), else 2.
// FUNCTION: VC1 0x00463ce0
int KdDisplayDevice::Pyro04(unsigned mode, unsigned value)
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
int KdDisplayDevice::Pyro00(void* a, void* b)
{
    (void)a;
    (void)b;
    return 0;
}

// RET 0x10. Uses g_graphics (0x004ed6f0); returns 1 when it is null. Body not
// recovered.
// STUB: VC1 0x00463d20
int KdDisplayDevice::Pyro24(void* a, void* b, void* c, void* d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return 0;
}

// RET 0x14. malloc(0x14) object (vftable 0x004d6240) holding b..e; *out = it.
// The object class is not recovered yet.
// STUB: VC1 0x00463df0
int KdDisplayDevice::Pyro18(void** out, void* b, void* c, void* d, void* e)
{
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    *out = 0;
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

// RET 0xc. QIs ITexture from the IDisplay CreateTexture result. Body not
// recovered.
// STUB: VC1 0x00463ed0
int KdDisplayDevice::Pyro14(void* a, void* b, void* c)
{
    (void)a;
    (void)b;
    (void)c;
    return 0;
}

// RET 4. malloc(0x2c), construct via 0x0045b6e0 (EAX=this), QI IAbstract at
// +0x10 into *out. That class is not recovered yet.
// STUB: VC1 0x00463fc0
void** KdDisplayDevice::Display3c(void** out)
{
    *out = 0;
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
    value14 = 0;
    if (g_displayPyro) {
        g_displayPyro->Done();
    }
    if (g_displayPyro) {
        free(g_displayPyro);
    }
    g_displayPyro = 0;
    return 0;
}

// RET 0x14. malloc(0x14) buffer object (vftables 0x004d6268 then 0x004d6368)
// with a b*c byte block; *out = its +4 face. Not recovered yet.
// STUB: VC1 0x004640e0
int KdDisplayDevice::Pyro0c(void** out, void* b, void* c, void* d, void* e)
{
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    *out = 0;
    return 0;
}

// RET 0x14. Same buffer object as Pyro0c; *out = the object itself.
// STUB: VC1 0x00464150
int KdDisplayDevice::Pyro08(void** out, void* b, void* c, void* d, void* e)
{
    (void)b;
    (void)c;
    (void)d;
    (void)e;
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
static int Display28Finish()
{
    LONG style;
    LONG ex;
    int* rect;
    unsigned extra;

    style = GetWindowLongA(g_kdDisplayWindow, GWL_STYLE);
    if (g_displaySkipDevice == 0) {
        rect = &g_displayViewLeft;
        extra = g_displayViewStyle;
    }
    else {
        rect = &g_displayWindowX;
        extra = g_displayWindowStyle;
    }
    SetWindowLongA(g_kdDisplayWindow, GWL_STYLE, (style & WS_VISIBLE) | extra);
    ex = GetWindowLongA(g_kdDisplayWindow, GWL_EXSTYLE);
    if (g_displaySkipDevice != 0) {
        SetWindowLongA(g_kdDisplayWindow, GWL_EXSTYLE, ex & ~WS_EX_TOPMOST);
        return SetWindowPos(g_kdDisplayWindow, HWND_NOTOPMOST,
            rect[0], rect[1], rect[2] - rect[0], rect[3] - rect[1], SWP_FRAMECHANGED);
    }
    SetWindowLongA(g_kdDisplayWindow, GWL_EXSTYLE, ex | WS_EX_TOPMOST);
    return SetWindowPos(g_kdDisplayWindow, HWND_TOPMOST,
        rect[0], rect[1], rect[2] - rect[0], rect[3] - rect[1], SWP_FRAMECHANGED);
}

// EAX=D3DFORMAT from the mode record +8. Maps 0x15..0x19 to present intervals.
// FUNCTION: VC1 0x004642e0
static int Display28Mode(unsigned format)
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
    typedef int (__stdcall *FnRenderState)(IUnknown* device, unsigned state, unsigned value);
    typedef int (__stdcall *FnStageState)(IUnknown* device, unsigned stage, unsigned type,
        unsigned value);

    if ((flags & 2) != (g_batchFlags & 2)) {
        if (flags & 2) {
            reinterpret_cast<FnRenderState>((*reinterpret_cast<void***>(g_d3dDevice))[0x32])(
                g_d3dDevice, 0x14, 6);
        }
        else {
            reinterpret_cast<FnRenderState>((*reinterpret_cast<void***>(g_d3dDevice))[0x32])(
                g_d3dDevice, 0x14, 2);
        }
    }
    if ((flags & 4) != (g_batchFlags & 4)) {
        if (flags & 4) {
            reinterpret_cast<FnRenderState>((*reinterpret_cast<void***>(g_d3dDevice))[0x32])(
                g_d3dDevice, 0xe, 1);
        }
        else {
            reinterpret_cast<FnRenderState>((*reinterpret_cast<void***>(g_d3dDevice))[0x32])(
                g_d3dDevice, 0xe, 0);
        }
    }
    if ((flags & 1) != (g_batchFlags & 1)) {
        if (flags & 1) {
            reinterpret_cast<FnStageState>((*reinterpret_cast<void***>(g_d3dDevice))[0x3f])(
                g_d3dDevice, 0, 1, 7);
        }
        else {
            reinterpret_cast<FnStageState>((*reinterpret_cast<void***>(g_d3dDevice))[0x3f])(
                g_d3dDevice, 0, 1, 4);
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
    typedef int (__stdcall *FnUnlock)(IUnknown* buffer);
    typedef int (__stdcall *FnDraw)(IUnknown* device, int type, int start, unsigned count);
    typedef int (__stdcall *FnDrawIndexed)(IUnknown* device, int type, int minIndex,
        unsigned vertices, int start, unsigned count);
    typedef int (__stdcall *FnLock)(IUnknown* buffer, int offset, int size, unsigned* out,
        int flags);

    if (g_batchVertices == 0) {
        return;
    }
    reinterpret_cast<FnUnlock>((*reinterpret_cast<void***>(g_d3dVertexBuffer))[0xc])(g_d3dVertexBuffer);
    if (g_batchCount) {
        if (g_batchKind == 2) {
            reinterpret_cast<FnDraw>((*reinterpret_cast<void***>(g_d3dDevice))[0x46])(
                g_d3dDevice, 2, 0, g_batchCount);
        }
        else if (g_batchKind == 3) {
            reinterpret_cast<FnDraw>((*reinterpret_cast<void***>(g_d3dDevice))[0x46])(
                g_d3dDevice, 4, 0, g_batchCount);
        }
        else if (g_batchKind == 4) {
            reinterpret_cast<FnDrawIndexed>((*reinterpret_cast<void***>(g_d3dDevice))[0x47])(
                g_d3dDevice, 4, 0, g_batchCount * 4, 0, g_batchCount * 2);
        }
        g_batchCount = 0;
    }
    if (final == 0) {
        reinterpret_cast<FnLock>((*reinterpret_cast<void***>(g_d3dVertexBuffer))[0xb])(
            g_d3dVertexBuffer, 0, 0, &g_batchVertices, 0);
        return;
    }
    g_batchVertices = 0;
}

// EAX=matrix. View = world with the 2D axes/origin of matrix, then
// SetTransform(D3DTS_VIEW, g_d3dView) via device vtable+0x94.
// FUNCTION: VC1 0x00464710
void DisplayD3dSetView(const Matrix3* matrix)
{
    typedef int (__stdcall *FnSetTransform)(IUnknown* device, unsigned state,
        const float* value);
    int i;

    for (i = 0; i < 16; i++) {
        g_d3dView[i] = g_d3dWorld[i];
    }
    g_d3dView[0] = matrix->m[0];
    g_d3dView[1] = matrix->m[1];
    g_d3dView[4] = matrix->m[3];
    g_d3dView[5] = matrix->m[4];
    g_d3dView[12] = matrix->m[6];
    g_d3dView[13] = matrix->m[7];
    FlushBatch(0);
    reinterpret_cast<FnSetTransform>((*reinterpret_cast<void***>(g_d3dDevice))[0x25])(
        g_d3dDevice, 2, g_d3dView);
}

// ECX=D3DCOLOR. Clear target, Z=1. Called from DisplayD3DInit with 0.
// FUNCTION: VC1 0x00464780
static void DisplayD3dClear(unsigned color)
{
    typedef int (__stdcall *FnClear)(IUnknown* device, unsigned count, void* rects,
        unsigned flags, unsigned col, float z, unsigned stencil);

    reinterpret_cast<FnClear>((*reinterpret_cast<void***>(g_d3dDevice))[0x24])(
        g_d3dDevice, 0, 0, 1, color, 1.0f, 0);
}

// ESI=width*, EDI=height*. GetDeviceCaps TextureCaps: 0x20 square, 0x2 pow2.
// FUNCTION: VC1 0x004647a0
static void DisplayD3dPow2Caps(unsigned* w, unsigned* h)
{
    typedef int (__stdcall *FnCaps)(IUnknown* device, unsigned char* caps);
    unsigned char caps[0xd4];
    unsigned value;
    unsigned packed;

    if (reinterpret_cast<FnCaps>((*reinterpret_cast<void***>(g_d3dDevice))[7])(
            g_d3dDevice, caps) < 0) {
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

// EDI=quad. Inferred name. Callers: KdGraphics::FillRect (0x0045ccb0) and
// KdGraphics::DrawImage (0x0045cb10). Flushes when the batch kind, the
// 1000-quad limit, the texture or the flags change, then appends the four
// vertices (0x60 bytes) to the locked buffer.
// FUNCTION: VC1 0x004648b0
void PushQuad(const BatchQuad* quad)
{
    typedef int (__stdcall *FnSetTexture)(IUnknown* device, unsigned stage, IUnknown* texture);

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
            reinterpret_cast<FnSetTexture>((*reinterpret_cast<void***>(g_d3dDevice))[0x3d])(
                g_d3dDevice, 0, quad->texture);
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
    IUnknown* object;
    typedef int (__stdcall *Fn154)(IUnknown* device, int a, int b);
    typedef int (__stdcall *Fn14c)(IUnknown* device, int a, int b, int c);
    typedef int (__stdcall *Fn30)(IUnknown* object);

    while (g_d3dChain2) {
        object = *reinterpret_cast<IUnknown**>(g_d3dChain2);
        next = *reinterpret_cast<void**>(static_cast<unsigned char*>(g_d3dChain2) + 0xc);
        if (g_d3dChain2 == g_d3dChain2) {
            g_d3dChain2 = next;
        }
        free(g_d3dChain2);
        if (object) {
            object->Release();
        }
        g_d3dChain2 = next;
        break;
    }

    if (g_d3dBackBuffer) {
        g_d3dBackBuffer->Release();
        g_d3dBackBuffer = 0;
    }
    node = g_d3dChain;
    while (node) {
        object = *reinterpret_cast<IUnknown**>(static_cast<unsigned char*>(node) + 8);
        if (object) {
            object->Release();
        }
        next = *reinterpret_cast<void**>(static_cast<unsigned char*>(node) + 0xc);
        free(node);
        node = next;
    }
    g_d3dChain = 0;
    if (g_d3dSurface) {
        if (g_d3dDevice) {
            reinterpret_cast<Fn154>((*reinterpret_cast<void***>(g_d3dDevice))[0x55])(
                g_d3dDevice, 0, 0);
        }
        g_d3dSurface->Release();
        g_d3dSurface = 0;
    }
    if (g_d3dVertexBuffer) {
        if (g_batchVertices) {
            reinterpret_cast<Fn30>((*reinterpret_cast<void***>(g_d3dVertexBuffer))[0xc])(g_d3dVertexBuffer);
            g_batchVertices = 0;
        }
        if (g_d3dDevice) {
            reinterpret_cast<Fn14c>((*reinterpret_cast<void***>(g_d3dDevice))[0x53])(
                g_d3dDevice, 0, 0, 0x18);
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
    MulMatrix4(product, ortho, g_d3dProj);
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
    typedef int (__stdcall *FnGetLevelDesc)(IUnknown* texture, unsigned level,
        unsigned* desc);
    typedef int (__stdcall *FnSetViewport)(IUnknown* device, const void* viewport);
    typedef int (__stdcall *FnSetTransform)(IUnknown* device, unsigned state,
        const float* value);
    struct Viewport
    {
        int x;
        int y;
        int width;
        int height;
        float minZ;
        float maxZ;
    };
    unsigned desc[8];
    Viewport viewport;
    IUnknown* texture;
    int maxWidth;
    int maxHeight;

    if (!g_d3dLastMixer) {
        maxWidth = g_displayMode[0];
        maxHeight = g_displayMode[1];
    }
    else {
        texture = *reinterpret_cast<IUnknown**>(
            static_cast<unsigned char*>(g_d3dLastMixer) + 8);
        maxWidth = reinterpret_cast<FnGetLevelDesc>((*reinterpret_cast<void***>(texture))[0xe])(
            texture, 0, desc) < 0 ? 0 : static_cast<int>(desc[6]);
        texture = *reinterpret_cast<IUnknown**>(
            static_cast<unsigned char*>(g_d3dLastMixer) + 8);
        maxHeight = reinterpret_cast<FnGetLevelDesc>((*reinterpret_cast<void***>(texture))[0xe])(
            texture, 0, desc) < 0 ? 0 : static_cast<int>(desc[7]);
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
        reinterpret_cast<FnSetViewport>((*reinterpret_cast<void***>(g_d3dDevice))[0x28])(
            g_d3dDevice, &viewport);
    }
    DisplayD3dSetBuffers(viewport.x + viewport.width, viewport.y + viewport.height,
        viewport.x, viewport.y);
    reinterpret_cast<FnSetTransform>((*reinterpret_cast<void***>(g_d3dDevice))[0x25])(
        g_d3dDevice, 3, g_d3dProj);
}

// STRING: VC1 0x004d248c
static const char kPercentS[] = "%s";
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

// EAX=HRESULT, ESI=prefix. cdecl flag. Logs via FormatBuffer; optional MB_ICONERROR.
// FUNCTION: VC1 0x00464d60
static void DisplayD3dLogHresult(int hr, char flag)
{
    char buf[512];

    if (hr == 0) {
        FormatBuffer(buf, sizeof(buf), kPercentS, g_kdNameDisplay);
    }
    else {
        FormatBuffer(buf, sizeof(buf), kDxErrorFmt, g_kdNameDisplay, DxGetErrorString(hr));
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
    typedef int (__stdcall *FnGetRt)(IUnknown* device, IUnknown** out);
    typedef int (__stdcall *FnCreateTex)(IUnknown* device, unsigned w, unsigned h,
        unsigned levels, unsigned usage, unsigned format, unsigned pool, IUnknown** out);
    typedef int (__stdcall *FnCreateVb)(IUnknown* device, unsigned bytes, unsigned usage,
        unsigned fvf, unsigned pool, IUnknown** out);
    typedef int (__stdcall *FnSetFvf)(IUnknown* device, unsigned fvf);
    typedef int (__stdcall *FnStream)(IUnknown* device, unsigned stream, IUnknown* vb,
        unsigned stride);
    typedef int (__stdcall *FnCreateIb)(IUnknown* device, unsigned bytes, unsigned usage,
        unsigned format, unsigned pool, IUnknown** out);
    typedef int (__stdcall *FnLock)(IUnknown* object, unsigned offset, unsigned size,
        short** out, unsigned flags);
    typedef int (__stdcall *FnUnlock)(IUnknown* object);
    typedef int (__stdcall *FnIndices)(IUnknown* device, IUnknown* ib, unsigned base);
    typedef int (__stdcall *FnState)(IUnknown* device, unsigned state, unsigned value);
    typedef int (__stdcall *FnTss)(IUnknown* device, unsigned stage, unsigned type,
        unsigned value);
    typedef int (__stdcall *FnXform)(IUnknown* device, unsigned type, float* matrix);
    void* node;
    IUnknown** texture;
    unsigned width;
    unsigned height;
    int hr;
    short* indices;
    short base;
    int i;
    char buf[520];

    g_d3dBackBuffer = 0;
    reinterpret_cast<FnGetRt>((*reinterpret_cast<void***>(g_d3dDevice))[0x20])(
        g_d3dDevice, &g_d3dBackBuffer);
    for (node = g_d3dChain; node;
        node = *reinterpret_cast<void**>(static_cast<unsigned char*>(node) + 0xc)) {
        texture = reinterpret_cast<IUnknown**>(static_cast<unsigned char*>(node) + 8);
        if (*texture) {
            width = *static_cast<unsigned*>(node);
            height = *(static_cast<unsigned*>(node) + 1);
            DisplayD3dPow2Caps(&width, &height);
            reinterpret_cast<FnCreateTex>((*reinterpret_cast<void***>(g_d3dDevice))[0x14])(
                g_d3dDevice, width, height, 1, 1,
                g_displayMode ? static_cast<unsigned>(g_displayMode[2]) : 0, 0, texture);
        }
    }
    hr = reinterpret_cast<FnCreateVb>((*reinterpret_cast<void***>(g_d3dDevice))[0x17])(
        g_d3dDevice, 0x17700, 8, 0x142, 0, &g_d3dVertexBuffer);
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
    reinterpret_cast<FnSetFvf>((*reinterpret_cast<void***>(g_d3dDevice))[0x4c])(
        g_d3dDevice, 0x142);
    reinterpret_cast<FnStream>((*reinterpret_cast<void***>(g_d3dDevice))[0x53])(
        g_d3dDevice, 0, g_d3dVertexBuffer, 0x18);
    hr = reinterpret_cast<FnCreateIb>((*reinterpret_cast<void***>(g_d3dDevice))[0x18])(
        g_d3dDevice, 0x2ee0, 8, 0x65, 0, &g_d3dSurface);
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
    hr = reinterpret_cast<FnLock>((*reinterpret_cast<void***>(g_d3dSurface))[0xb])(
        g_d3dSurface, 0, 0, &indices, 0);
    if (hr < 0) {
        DisplayD3dLogHresult(hr, 0);
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
    reinterpret_cast<FnUnlock>((*reinterpret_cast<void***>(g_d3dSurface))[0xc])(g_d3dSurface);
    reinterpret_cast<FnIndices>((*reinterpret_cast<void***>(g_d3dDevice))[0x55])(
        g_d3dDevice, g_d3dSurface, 0);
    reinterpret_cast<FnState>((*reinterpret_cast<void***>(g_d3dDevice))[0x32])(
        g_d3dDevice, 0x16, 1);
    reinterpret_cast<FnState>((*reinterpret_cast<void***>(g_d3dDevice))[0x32])(
        g_d3dDevice, 0x89, 0);
    reinterpret_cast<FnState>((*reinterpret_cast<void***>(g_d3dDevice))[0x32])(
        g_d3dDevice, 0x1b, 1);
    reinterpret_cast<FnState>((*reinterpret_cast<void***>(g_d3dDevice))[0x32])(
        g_d3dDevice, 0x13, 5);
    reinterpret_cast<FnState>((*reinterpret_cast<void***>(g_d3dDevice))[0x32])(
        g_d3dDevice, 0x14, 6);
    reinterpret_cast<FnState>((*reinterpret_cast<void***>(g_d3dDevice))[0x32])(
        g_d3dDevice, 0xf, 1);
    reinterpret_cast<FnState>((*reinterpret_cast<void***>(g_d3dDevice))[0x32])(
        g_d3dDevice, 0x18, 1);
    reinterpret_cast<FnState>((*reinterpret_cast<void***>(g_d3dDevice))[0x32])(
        g_d3dDevice, 0x19, 7);
    reinterpret_cast<FnTss>((*reinterpret_cast<void***>(g_d3dDevice))[0x3f])(
        g_d3dDevice, 0, 1, 4);
    reinterpret_cast<FnTss>((*reinterpret_cast<void***>(g_d3dDevice))[0x3f])(
        g_d3dDevice, 0, 2, 2);
    reinterpret_cast<FnTss>((*reinterpret_cast<void***>(g_d3dDevice))[0x3f])(
        g_d3dDevice, 0, 3, 0);
    reinterpret_cast<FnTss>((*reinterpret_cast<void***>(g_d3dDevice))[0x3f])(
        g_d3dDevice, 0, 4, 4);
    reinterpret_cast<FnTss>((*reinterpret_cast<void***>(g_d3dDevice))[0x3f])(
        g_d3dDevice, 0, 5, 2);
    reinterpret_cast<FnTss>((*reinterpret_cast<void***>(g_d3dDevice))[0x3f])(
        g_d3dDevice, 0, 6, 0);
    reinterpret_cast<FnTss>((*reinterpret_cast<void***>(g_d3dDevice))[0x3f])(
        g_d3dDevice, 0, 0x12, 1);
    reinterpret_cast<FnTss>((*reinterpret_cast<void***>(g_d3dDevice))[0x3f])(
        g_d3dDevice, 0, 0x10, 2);
    reinterpret_cast<FnTss>((*reinterpret_cast<void***>(g_d3dDevice))[0x3f])(
        g_d3dDevice, 0, 0x11, 2);
    g_batchCount = 0;
    g_batchKind = 4;
    g_batchFlags = 2;
    g_batchTexture = 0;
    reinterpret_cast<FnXform>((*reinterpret_cast<void***>(g_d3dDevice))[0x25])(
        g_d3dDevice, 2, g_d3dView);
    reinterpret_cast<FnXform>((*reinterpret_cast<void***>(g_d3dDevice))[0x25])(
        g_d3dDevice, 3, g_d3dProj);
    return 1;
}

// Release buffers then Reset(g_displayMode); recreate. Returns AL.
// FUNCTION: VC1 0x00465200
static unsigned char DisplayD3dReset()
{
    void* node;
    IUnknown* object;
    typedef int (__stdcall *Fn154)(IUnknown* device, int a, int b);
    typedef int (__stdcall *Fn14c)(IUnknown* device, int a, int b, int c);
    typedef int (__stdcall *FnReset)(IUnknown* device, void* present);

    if (g_d3dBackBuffer) {
        g_d3dBackBuffer->Release();
    }
    for (node = g_d3dChain; node;
        node = *reinterpret_cast<void**>(static_cast<unsigned char*>(node) + 0xc)) {
        object = *reinterpret_cast<IUnknown**>(static_cast<unsigned char*>(node) + 8);
        if (object) {
            object->Release();
        }
    }
    if (g_d3dSurface) {
        reinterpret_cast<Fn154>((*reinterpret_cast<void***>(g_d3dDevice))[0x55])(
            g_d3dDevice, 0, 0);
        g_d3dSurface->Release();
    }
    if (g_d3dVertexBuffer) {
        reinterpret_cast<Fn14c>((*reinterpret_cast<void***>(g_d3dDevice))[0x53])(
            g_d3dDevice, 0, 0, 0x18);
        g_d3dVertexBuffer->Release();
    }
    reinterpret_cast<FnReset>((*reinterpret_cast<void***>(g_d3dDevice))[0xe])(
        g_d3dDevice, g_displayMode);
    return DisplayD3dRecreate() != 0;
}

static IUnknown* CallDirect3DCreate8(unsigned sdk)
{
    typedef IUnknown* (__stdcall *FnCreate8)(unsigned version);
    static FnCreate8 create8;
    HMODULE dll;

    if (!create8) {
        dll = LoadLibraryA("d3d8.dll");
        if (dll) {
            create8 = reinterpret_cast<FnCreate8>(GetProcAddress(dll, "Direct3DCreate8"));
        }
    }
    if (!create8) {
        return 0;
    }
    return create8(sdk);
}

// Direct3DCreate8, present params, CreateDevice, Recreate, Clear(0). Returns AL.
// FUNCTION: VC1 0x004652b0
static unsigned char DisplayD3DInit()
{
    typedef int (__stdcall *FnIdent)(IUnknown* d3d, unsigned adapter, unsigned flags,
        void* out);
    typedef int (__stdcall *FnMode)(IUnknown* d3d, unsigned adapter, unsigned* out);
    typedef unsigned (__stdcall *FnModeCount)(IUnknown* d3d, unsigned adapter);
    typedef int (__stdcall *FnEnum)(IUnknown* d3d, unsigned adapter, unsigned index,
        unsigned* out);
    typedef int (__stdcall *FnCreateDev)(IUnknown* d3d, unsigned adapter, unsigned type,
        HWND focus, unsigned behavior, unsigned* present, IUnknown** device);
    unsigned desktop[4];
    unsigned enumMode[4];
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

    g_d3dObject = CallDirect3DCreate8(0x78);
    if (!g_d3dObject) {
        FormatBufferOpen(buf, kDxErrorFmt, kCantCreateD3d,
            DxGetErrorString(static_cast<int>(0x8876086a)));
        HostLogError(&g_kdNameDisplay, buf);
        MessageBoxA(g_kdDisplayWindow, buf, kDirectXError, MB_ICONHAND);
        return 0;
    }
    reinterpret_cast<FnIdent>((*reinterpret_cast<void***>(g_d3dObject))[5])(
        g_d3dObject, 0, 2, ident);
    HostLog(&g_kdNameDisplay, kD3dDriver, ident);
    HostLog(&g_kdNameDisplay, kD3dDescription, reinterpret_cast<char*>(ident) + 0x200);
    HostLog(&g_kdNameDisplay, kD3dVersion,
        (ident[0x105] >> 16) & 0xffff, ident[0x105] & 0xffff,
        (ident[0x104] >> 16) & 0xffff, ident[0x104] & 0xffff);
    hr = reinterpret_cast<FnMode>((*reinterpret_cast<void***>(g_d3dObject))[8])(
        g_d3dObject, 0, desktop);
    if (hr < 0 || desktop[3] == 0) {
        DisplayD3dLogHresult(hr, g_displaySkipDevice);
        return 0;
    }
    memset(g_d3dPresentWindowed, 0, sizeof(g_d3dPresentWindowed));
    g_d3dPresentWindowed[0] = static_cast<unsigned>(g_displayWidth);
    g_d3dPresentWindowed[1] = static_cast<unsigned>(g_displayHeight);
    g_d3dPresentWindowed[2] = desktop[3];
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
        viewW = static_cast<unsigned>(g_displayViewRight - g_displayViewLeft);
        viewH = static_cast<unsigned>(g_displayViewBottom - g_displayViewTop);
        format = 0;
        if (viewW == desktop[0] && viewH == desktop[1]) {
            format = desktop[3];
        }
        else {
            count = reinterpret_cast<FnModeCount>((*reinterpret_cast<void***>(g_d3dObject))[6])(
                g_d3dObject, 0);
            for (index = 0; index < count; index++) {
                reinterpret_cast<FnEnum>((*reinterpret_cast<void***>(g_d3dObject))[7])(
                    g_d3dObject, 0, index, enumMode);
                if (enumMode[0] != viewW || enumMode[1] != viewH) {
                    continue;
                }
                if (g_displayPresentInterval == 0x10) {
                    if (Display28Mode(enumMode[3]) > Display28Mode(0x19)) {
                        continue;
                    }
                }
                if (Display28Mode(enumMode[3]) > Display28Mode(format)) {
                    format = enumMode[3];
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
            g_d3dFormatNames[Display28Mode(format)]);
        if (g_displaySkipDevice == 0) {
            present = g_d3dPresentFull;
        }
        else {
            present = g_d3dPresentWindowed;
        }
    }
    g_displayMode = reinterpret_cast<int*>(present);
    g_displayPresentInterval = ((Display28Mode(present[2]) < 4) - 1 & 0xfffffff0) + 0x20;
    hr = reinterpret_cast<FnCreateDev>((*reinterpret_cast<void***>(g_d3dObject))[0xf])(
        g_d3dObject, 0, 1, g_kdDisplayWindow, 0x20, present, &g_d3dDevice);
    if (hr < 0) {
        DisplayD3dLogHresult(hr, 1);
        return 0;
    }
    Display28Finish();
    g_batchVertices = 0;
    g_d3dChain2 = 0;
    DisplayD3dSetBuffers(present[0], present[1], 0, 0);
    for (i = 0; i < 16; i++) {
        g_d3dView[i] = g_d3dWorld[i];
    }
    if (!DisplayD3dRecreate()) {
        return 0;
    }
    DisplayD3dClear(0);
    return 1;
}

// cdecl width/height. CreateTexture + GetLevelDesc; link node onto g_d3dChain.
// FUNCTION: VC1 0x00465bc0
void* LookupRenderTargetTexture(unsigned width, unsigned height)
{
    typedef int (__stdcall *FnCreateTex)(IUnknown* device, unsigned w, unsigned h,
        unsigned levels, unsigned usage, unsigned format, unsigned pool, IUnknown** out);
    typedef int (__stdcall *FnLevelDesc)(IUnknown* tex, unsigned level, unsigned* desc);
    unsigned* node;
    IUnknown** texture;
    int hr;
    unsigned desc[8];
    char buf[544];

    node = static_cast<unsigned*>(malloc(0x10));
    texture = reinterpret_cast<IUnknown**>(node + 2);
    *texture = 0;
    DisplayD3dPow2Caps(&width, &height);
    hr = reinterpret_cast<FnCreateTex>((*reinterpret_cast<void***>(g_d3dDevice))[0x14])(
        g_d3dDevice, width, height, 1, 1,
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
    reinterpret_cast<FnLevelDesc>((*reinterpret_cast<void***>(*texture))[0xe])(
        *texture, 0, desc);
    node[0] = desc[6];
    node[1] = desc[7];
    node[3] = reinterpret_cast<unsigned>(g_d3dChain);
    g_d3dChain = node;
    return node;
}

// D3D TestCooperativeLevel / reset. Returns AL; EBX is the last mixer.
// FUNCTION: VC1 0x00465cc0
static unsigned char DisplayQueryReady()
{
    typedef int (__stdcall *FnTest)(IUnknown* device);
    typedef int (__stdcall *FnBegin)(IUnknown* device);
    typedef int (__stdcall *FnGetBack)(IUnknown* swap, int a, int b, unsigned* out, int c);
    typedef int (__stdcall *FnXform)(IUnknown* device, unsigned type, float* matrix);
    int hr;
    int i;
    char buf[520];

    buf[0] = 0;
    hr = reinterpret_cast<FnTest>((*reinterpret_cast<void***>(g_d3dDevice))[3])(g_d3dDevice);
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
    if (g_d3dLastMixer) {
        DisplayD3dSetBuffers(
            static_cast<int*>(g_d3dLastMixer)[0],
            static_cast<int*>(g_d3dLastMixer)[1],
            0, 0);
        reinterpret_cast<FnXform>((*reinterpret_cast<void***>(g_d3dDevice))[0x25])(
            g_d3dDevice, 3, g_d3dProj);
        for (i = 0; i < 16; i++) {
            g_d3dView[i] = g_d3dWorld[i];
        }
        reinterpret_cast<FnXform>((*reinterpret_cast<void***>(g_d3dDevice))[0x25])(
            g_d3dDevice, 2, g_d3dView);
    }
    reinterpret_cast<FnBegin>((*reinterpret_cast<void***>(g_d3dDevice))[0x22])(g_d3dDevice);
    if (g_d3dVertexBuffer) {
        reinterpret_cast<FnGetBack>((*reinterpret_cast<void***>(g_d3dVertexBuffer))[0xb])(
            g_d3dVertexBuffer, 0, 0, &g_batchVertices, 0);
    }
    return 1;
}

// BL=windowed (SetWindowed argument). Switches g_displaySkipDevice and resets
// the device when the mode actually changes.
// FUNCTION: VC1 0x00465e70
static int Display28Apply(unsigned char windowed)
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
        GetWindowRect(g_kdDisplayWindow, reinterpret_cast<LPRECT>(&g_displayWindowX));
    }
    g_displaySkipDevice = windowed;
    interval = Display28Mode(g_displayMode ? static_cast<unsigned>(g_displayMode[2]) : 0);
    g_displayPresentInterval = ((3 < interval) - 1 & 0xfffffff0) + 0x20;
    DisplayD3dReset();
    return Display28Finish();
}

// Primary vtable+0x18. Centers the window rect on the primary monitor and
// resets the view rect to the display size.
// FUNCTION: VC1 0x004661e0
int KdDisplayBase::FitWindow()
{
    int width;
    int height;

    GetWindowRect(g_kdDisplayWindow, reinterpret_cast<LPRECT>(&g_displayWindowX));
    width = g_displayWindowRight - g_displayWindowX;
    height = g_displayWindowBottom - g_displayWindowY;
    g_displayWindowX = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
    g_displayWindowY = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
    g_displayWindowBottom = g_displayWindowY + height;
    g_displayWindowRight = g_displayWindowX + width;
    g_displayWindowStyle = 0xca0000;
    g_displayViewLeft = 0;
    g_displayViewTop = 0;
    g_displayViewRight = g_displayWidth;
    g_displayViewBottom = g_displayHeight;
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
unsigned char KdDisplayBase::IsWindowed()
{
    return g_displaySkipDevice;
}

// XOR AL, AL; RET 8. IScriptObject vtable+0x10.
// FUNCTION: VC1 0x004662d0
unsigned char KdDisplayBase::Script10(void* a, void* b)
{
    (void)a;
    (void)b;
    return 0;
}

// Guarded static IUnknown*. IRenderTarget vtable+0x1c; IDisplay thunk SUB ECX, 0x10.
// FUNCTION: VC1 0x004662e0
void* KdDisplayBase::Display1c()
{
    if ((g_display1cGuard & 1) == 0) {
        g_display1cGuard |= 1;
        g_display1cObject = 0;
        atexit(Display1cAtexit);
    }
    return &g_display1cObject;
}

// RET 4. IDisplay vtable+0x30.
// FUNCTION: VC1 0x00466310
void KdDisplayBase::Display30(int unused)
{
    (void)unused;
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
// unlink the window from g_heapPageList and destroy it.
// FUNCTION: VC1 0x00466400
int KdDisplayBase::Shutdown() throw()
{
    unsigned char* window;
    HWND hwnd;
    void* node;
    void* next;
    void* prev;

    KdDisplayDevice::Shutdown();
    DisplayD3DTeardown();

    window = static_cast<unsigned char*>(mixer);
    if (!window) {
        return 0;
    }

    node = g_heapPageList;
    while (node) {
        next = *reinterpret_cast<void**>(static_cast<unsigned char*>(node) + 0xc);
        if (*reinterpret_cast<int*>(static_cast<unsigned char*>(node) + 4) == 0
            && *reinterpret_cast<void**>(static_cast<unsigned char*>(node) + 8) == window) {
            if (g_heapPageList == node) {
                g_heapPageList = next;
            }
            else {
                prev = *reinterpret_cast<void**>(static_cast<unsigned char*>(node) + 0x10);
                *reinterpret_cast<void**>(static_cast<unsigned char*>(prev) + 0xc) = next;
            }
            free(node);
            break;
        }
        node = next;
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

// IRenderTarget vtable+0x10. Rebind the graphics scene, end and present.
// FUNCTION: VC1 0x00466570
void* KdDisplayBase::Display10()
{
    typedef int (__stdcall *FnEndScene)(IUnknown* device);
    typedef int (__stdcall *FnPresent)(IUnknown* device, void* a, void* b, void* c,
        void* d);

    static_cast<KdGraphics*>(graphics)->BindScene();
    FlushBatch(1);
    reinterpret_cast<FnEndScene>((*reinterpret_cast<void***>(g_d3dDevice))[0x23])(
        g_d3dDevice);
    if (g_d3dLastMixer == 0) {
        reinterpret_cast<FnPresent>((*reinterpret_cast<void***>(g_d3dDevice))[0xf])(
            g_d3dDevice, 0, 0, 0, 0);
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

// RET 8. IRenderTarget::Display0c(a, b); return a.
// FUNCTION: VC1 0x00466640
void* KdDisplayBase::Display0c(void* a, void* b)
{
    IRenderTarget::Display0c(a, b);
    return a;
}

// RET 0xc. IDisplay vtable+0x38. Loads 3 stdcall args then cdecl-calls 0x0045bf40.
// FUNCTION: VC1 0x00466660
void** KdDisplayBase::CreateRenderTarget(void** out, unsigned width, unsigned height)
{
    CreateRenderTargetBody(out, width, height);
    return out;
}

// RET 4. BL=arg into Display28Apply.
// FUNCTION: VC1 0x00466680
int KdDisplayBase::SetWindowed(unsigned char arg)
{
    return Display28Apply(arg);
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

    SetWindowTextA(hwnd, "Virtual City");
    StartPostMixer();
    g_kdDisplayWindow = hwnd;
    ShowWindow(hwnd, SW_SHOWNORMAL);
    g_displayWidth = static_cast<int>(width);
    g_displayHeight = static_cast<int>(height);

    if (!GetKdEnv(kKdParentWindow)) {
        FitWindow();
        SetWindowPos(g_kdDisplayWindow, 0, g_displayWindowX, g_displayWindowY, 0, 0,
            SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    }

    if (!DisplayD3DInit()) {
        HostLogError(&g_kdNameDisplay, kCantInit);
        return 0;
    }
    return KdDisplayDevice::SetSize(width, height);
}

// RET 8. Script method dispatch (CreateRenderTarget, GetWidth, ...). Not recovered.
// STUB: VC1 0x00466b00
unsigned char KdDisplayBase::Script0c(void* a, void* b)
{
    (void)a;
    (void)b;
    return 0;
}

// RegisterClassA for the display window; hInstance at +0x10, class name at +0x24.
// FUNCTION: VC1 0x00496020
static WNDCLASSA* GetKdWindowClass()
{
    if (!g_kdDisplayClass.hInstance) {
        g_kdDisplayClass.hInstance = GetModuleHandleA(0);
        g_kdDisplayClass.hIcon = LoadIconA(g_kdDisplayClass.hInstance, MAKEINTRESOURCE(100));
        g_kdDisplayClass.hCursor = LoadCursorA(g_kdDisplayClass.hInstance, MAKEINTRESOURCE(100));
        g_kdDisplayClass.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
        if (!g_kdDisplayClass.hCursor) {
            g_kdDisplayClass.hCursor = LoadCursorA(0, IDC_ARROW);
        }
        RegisterClassA(&g_kdDisplayClass);
    }
    return &g_kdDisplayClass;
}

// CreateWindowExA mixer block; hwnd at +4, size at +0x10/+0x14.
// FUNCTION: VC1 0x004961b0
static void* CreateKdWindow()
{
    const char* env;
    HWND parent;
    RECT client;
    int x;
    int y;
    int width;
    int height;
    DWORD style;
    HWND hwnd;
    unsigned char* mixer;
    WNDCLASSA* wnd;

    env = GetKdEnv(kKdParentWindow);
    parent = 0;
    if (env) {
        parent = reinterpret_cast<HWND>(strtoul(env, 0, 16));
    }

    x = static_cast<int>(CW_USEDEFAULT);
    y = static_cast<int>(CW_USEDEFAULT);
    width = static_cast<int>(CW_USEDEFAULT);
    height = static_cast<int>(CW_USEDEFAULT);
    if (parent) {
        GetClientRect(parent, &client);
        x = client.left;
        y = client.top;
        width = client.right - client.left;
        height = client.bottom - client.top;
    }

    wnd = GetKdWindowClass();
    style = 0xca0000;
    if (parent) {
        style += 0x3f360000;
    }
    hwnd = CreateWindowExA(0, wnd->lpszClassName, kWindowName, style,
        x, y, width, height, parent, 0, wnd->hInstance, 0);
    if (!hwnd) {
        *_errno() = 0x1f;
        return 0;
    }

    mixer = static_cast<unsigned char*>(malloc(0x18));
    g_kdWindowMixer = mixer;
    *reinterpret_cast<void**>(mixer) = mixer;
    *reinterpret_cast<HWND*>(mixer + 4) = hwnd;
    *reinterpret_cast<unsigned*>(mixer + 8) = 0;
    *reinterpret_cast<unsigned*>(mixer + 0xc) = 0;
    *reinterpret_cast<int*>(mixer + 0x10) = width;
    *reinterpret_cast<int*>(mixer + 0x14) = height;
    SetTimer(hwnd, 1, 100, 0);
    SetWindowLongA(hwnd, GWL_USERDATA, reinterpret_cast<LONG>(mixer));
    return mixer;
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
    g_displayViewRight = static_cast<int>(mode.dmPelsWidth);
    g_displayViewBottom = static_cast<int>(mode.dmPelsHeight);
    SystemParametersInfoA(SPI_GETWORKAREA, 0, &work, 0);
    if (g_displayWindowRight - g_displayWindowX <= work.right - work.left
        && g_displayWindowBottom - g_displayWindowY <= work.bottom - work.top) {
        enabled58 = 1;
        return 1;
    }
    enabled58 = 0;
    return 0;
}

// FUNCTION: VC1 0x004b1d30
unsigned char KdDisplay::Script10(void* a, void* b)
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
int KdDisplay::Shutdown()
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
// target = 0, identity matrix and enabled58 = 1.
// FUNCTION: VC1 0x004b1e40
KdDisplay::KdDisplay() throw()
{
    target = 0;
    matrix.m[0] = 1.0f;
    matrix.m[1] = 0.0f;
    matrix.m[2] = 0.0f;
    matrix.m[3] = 0.0f;
    matrix.m[4] = 1.0f;
    matrix.m[5] = 0.0f;
    matrix.m[6] = 0.0f;
    matrix.m[7] = 0.0f;
    matrix.m[8] = 1.0f;
    enabled58 = 1;
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
void* KdDisplay::Display10()
{
    typedef int (__stdcall *FnEndScene)(IUnknown* device);
    typedef int (__stdcall *FnPresent)(IUnknown* device, void* a, void* b, void* c,
        void* d);
    IRenderTarget* source;
    IUnknown* camera;
    IGraphics* view;
    Rect clip;

    if (g_displaySkipDevice == 0) {
        source = static_cast<IRenderTarget*>(static_cast<void*>(target));
        if (source) {
            source->Display10();
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
            IRenderTarget::Display0c(&view, &camera);
            if (camera) {
                camera->Release();
            }
            if (view) {
                view->SetTransform(&matrix);
                clip = *reinterpret_cast<Rect*>(&g_displayViewLeft);
                view->SetClipRect(&clip);
                view->Clear(0xff000000);
                view->DrawImage(static_cast<ITexture* const*>(static_cast<IRenderTarget*>(
                    static_cast<void*>(target))->Display1c()), 0.0f, 0.0f, 0.0f, 0.0f,
                    static_cast<float>(g_displayWidth), static_cast<float>(g_displayHeight));
                view->Release();
            }
        }
    }

    static_cast<KdGraphics*>(graphics)->BindScene();
    FlushBatch(1);
    reinterpret_cast<FnEndScene>((*reinterpret_cast<void***>(g_d3dDevice))[0x23])(
        g_d3dDevice);
    if (g_d3dLastMixer == 0) {
        reinterpret_cast<FnPresent>((*reinterpret_cast<void***>(g_d3dDevice))[0xf])(
            g_d3dDevice, 0, 0, 0, 0);
    }
    return 0;
}

// this = IRenderTarget. RET 8. Forwards to target when drawing offscreen.
// FUNCTION: VC1 0x004b20c0
void* KdDisplay::Display0c(void* a, void* b)
{
    IRenderTarget* source = static_cast<IRenderTarget*>(static_cast<void*>(target));

    if (g_displaySkipDevice == 0 && source) {
        source->Display0c(a, b);
        return a;
    }
    IRenderTarget::Display0c(a, b);
    return a;
}

// this = IDisplay. Leaving windowed mode needs enabled58 (window fits). RET 4.
// FUNCTION: VC1 0x004b2100
int KdDisplay::SetWindowed(unsigned char arg)
{
    if (arg == 0 || enabled58 != 0) {
        return Display28Apply(arg);
    }
    return 0;
}

// this = IDisplay. Calls 0x00466690 then rebuilds the Matrix3 at IDisplay+0x20.
// FUNCTION: VC1 0x004b2120
unsigned char KdDisplay::SetSize(unsigned width, unsigned height)
{
    int viewW;
    int viewH;
    void* slot;
    float scaleX;
    float scaleY;
    float scale;
    Matrix3 inverse;

    if (!KdDisplayBase::SetSize(width, height)) {
        return 0;
    }
    if (GetKdEnv(kKdParentWindow)) {
        return 1;
    }

    viewW = g_displayViewRight - g_displayViewLeft;
    viewH = g_displayViewBottom - g_displayViewTop;
    if (g_displayWidth == viewW && g_displayHeight == viewH) {
        return 1;
    }

    slot = 0;
    CreateRenderTarget(&slot, g_displayWidth, g_displayHeight);
    DisplayAssignTarget(&slot, &target);
    if (slot) {
        static_cast<IUnknown*>(slot)->Release();
    }
    if (!target) {
        HostLogError(&g_kdNameDisplay, kCantCreateRt);
        return 0;
    }

    scaleX = static_cast<float>(viewW) / static_cast<float>(g_displayWidth);
    scaleY = static_cast<float>(viewH) / static_cast<float>(g_displayHeight);
    scale = MinFloat(scaleX, scaleY);
    matrix = g_matrix3Identity;
    matrix.m[0] *= scale;
    matrix.m[1] *= scale;
    matrix.m[3] *= scale;
    matrix.m[4] *= scale;
    if (scaleX < scaleY) {
        matrix.m[6] = (static_cast<float>(viewW) - static_cast<float>(g_displayWidth) * scale)
            * 0.5f;
        matrix.m[7] = 0.0f;
    }
    else {
        matrix.m[6] = 0.0f;
        matrix.m[7] = (static_cast<float>(viewH) - static_cast<float>(g_displayHeight) * scale)
            * 0.5f;
    }
    Matrix3::Inverse(&inverse, &matrix);
    g_displayMatrixInv = inverse;
    return 1;
}

// STUB: VC1 0x004b22e0
unsigned char KdDisplay::Script0c(void* a, void* b)
{
    return KdDisplayBase::Script0c(a, b);
}

// Release the Display1c static if set. Registered with atexit from 0x004662e0.
// FUNCTION: VC1 0x004cad30
static void Display1cAtexit()
{
    IUnknown* object = g_display1cObject;
    if (object) {
        object->Release();
    }
    g_display1cObject = 0;
}

}
