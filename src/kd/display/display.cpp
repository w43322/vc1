#include "kd/display/display.h"

#include "kd/iface/iunknown.h"
#include "kd/iface/iid.h"
#include "host/runtime.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

namespace vc1 {

// GLOBAL: VC1 0x004ed54c
IAbstract* g_kdDisplay;
// GLOBAL: VC1 0x004ed550
HWND g_kdDisplayWindow;
// GLOBAL: VC1 0x004ed554
static unsigned g_displayBusy;
// GLOBAL: VC1 0x004ed558
static int* g_displayMode;
// GLOBAL: VC1 0x004ed590
static int g_displayWindowX;
// GLOBAL: VC1 0x004ed594
static int g_displayWindowY;
// GLOBAL: VC1 0x004ed598
static int g_displayWindowRight;
// GLOBAL: VC1 0x004ed59c
static int g_displayWindowBottom;
// GLOBAL: VC1 0x004ed5a0
static unsigned g_displayWindowStyle;
// GLOBAL: VC1 0x004ed5d8
static int g_displayViewLeft;
// GLOBAL: VC1 0x004ed5dc
static int g_displayViewTop;
// GLOBAL: VC1 0x004ed5e0
static int g_displayViewRight;
// GLOBAL: VC1 0x004ed5e4
static int g_displayViewBottom;
// GLOBAL: VC1 0x004ed5e8
static unsigned g_displayViewStyle;
// GLOBAL: VC1 0x004ed5f0
static IUnknown* g_d3dDevice;
// GLOBAL: VC1 0x004ed5ec
static IUnknown* g_d3dObject;
// GLOBAL: VC1 0x004ed5f4
static IUnknown* g_d3dSwap;
// GLOBAL: VC1 0x004ed5f8
static IUnknown* g_d3dSurface;
// GLOBAL: VC1 0x004ed5fc
static IUnknown* g_d3dBackBuffer;
// GLOBAL: VC1 0x004ed600
static void* g_d3dChain;
// GLOBAL: VC1 0x004ed604
static void* g_d3dLastMixer;
// GLOBAL: VC1 0x004ed608
static void* g_d3dChain2;
// GLOBAL: VC1 0x004ed60c
static unsigned g_d3dInScene;
// GLOBAL: VC1 0x004ed610
static unsigned g_d3dGammaRamp;
// GLOBAL: VC1 0x004ed614
static unsigned g_d3dGammaKind;
// GLOBAL: VC1 0x004ed564
static unsigned g_displayReady;
// GLOBAL: VC1 0x004ed5ac
static unsigned g_displayModeReady;
// GLOBAL: VC1 0x004ed700
static IUnknown* g_displayScene;
// GLOBAL: VC1 0x004ed6ec
static IUnknown* g_graphicsQuery;
// GLOBAL: VC1 0x004ed628
static void* g_displayPyro;
// GLOBAL: VC1 0x004ed65c
IUnknown* g_kdQueryUnknown;
// GLOBAL: VC1 0x004ed6f0
static IUnknown* g_graphics;
// GLOBAL: VC1 0x004ed6f4
static IUnknown* g_graphicsHost;
// GLOBAL: VC1 0x004e74b6
unsigned char g_displaySkipDevice;
// GLOBAL: VC1 0x004e8744
int g_displayWidth;
// GLOBAL: VC1 0x004e8998
int g_displayHeight;
// GLOBAL: VC1 0x004e899c
static int g_displayPresentInterval;
// GLOBAL: VC1 0x004ed450
static void* g_kdWindowMixer;
// GLOBAL: VC1 0x004e95a0
static WNDCLASSA g_kdDisplayClass;

// STRING: VC1 0x004d4c4c
static const char kDisplay[] = "Display";
// GLOBAL: VC1 0x004d0630
const char* g_kdNameDisplay = kDisplay;
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
static IUnknown* g_display1cObject;
// GLOBAL: VC1 0x004ee288
static unsigned g_display1cGuard;

static void DisplayDtorBase(KdDisplay* self);
static void ConstructDisplayBase(KdDisplay* self);
static unsigned char Script0cBody(void* a, void* b);
static void Display1cAtexit();
static void* DisplayQueryGraphics(IRenderTarget* self, void* a, void* b);
static unsigned char DisplayQueryFromTarget(void* mixer, void* args);
static unsigned char DisplayQueryReady();
static void RenderTargetInit(IRenderTarget* self);
static void* Display34Construct(void* self);
static void Display10Scene(void* self);
static void* DisplaySceneConstruct(void* self);
static unsigned char DisplayInitPyro(IDisplay* self, unsigned width, unsigned height);
static void DisplayReleaseGraphics(IShutdownable* self);
static int Display28Finish();
static int Display28Mode(unsigned format);
static const char* DxGetErrorString(int hr);
static unsigned char DisplayD3dRecreate();
static void Display10Present(int arg);
static void DisplayD3DTeardown();
static void DisplayD3dSetBuffers(void* a, int b, int c, int d);
static void DisplayD3dLogHresult(int hr, char flag);
static unsigned char DisplayD3dReset();
static unsigned char DisplayD3DInit();
static void** Display38Body(void** out);
static int Display28Apply();
static int DisplayShutdownPrep(IShutdownable* self);
static unsigned char DisplaySetSizeBody(IDisplay* self, unsigned width, unsigned height);
static WNDCLASSA* GetKdWindowClass();
static void* CreateKdWindow();

KdDisplay::KdDisplay()
{
    target = 0;
    matrix[0] = 1.0f;
    matrix[1] = 0.0f;
    matrix[2] = 0.0f;
    matrix[3] = 0.0f;
    matrix[4] = 1.0f;
    matrix[5] = 0.0f;
    matrix[6] = 0.0f;
    matrix[7] = 0.0f;
    matrix[8] = 1.0f;
    enabled58 = 1;
}

KdDisplay::~KdDisplay()
{
    if (target) {
        target->Release();
    }
    target = 0;
    DisplayDtorBase(this);
}

// EDI=IRenderTarget (KdDisplay+0x04). Allocates a 100-byte helper and QIs IGraphics.
// FUNCTION: VC1 0x0041a460
static void RenderTargetInit(IRenderTarget* self)
{
    IUnknown* ident;
    IUnknown* graphics;
    void* block;

    *reinterpret_cast<unsigned*>(self) = 0x004d5de0;
    *reinterpret_cast<void**>(reinterpret_cast<char*>(self) + 4) = 0;
    *reinterpret_cast<void**>(reinterpret_cast<char*>(self) + 8) = 0;
    block = malloc(0x64);
    ident = 0;
    if (block) {
        block = DisplaySceneConstruct(block);
        if (block) {
            ident = static_cast<IUnknown*>(
                reinterpret_cast<IUnknown*>(
                    reinterpret_cast<char*>(block) + 0x48)->QueryInterface(
                    &g_iidAbstract));
            if (ident) {
                ident->AddRef();
            }
        }
    }
    if (ident) {
        ident->Release();
        graphics = *reinterpret_cast<IUnknown**>(reinterpret_cast<char*>(self) + 8);
        if (graphics) {
            graphics->Release();
        }
        graphics = static_cast<IUnknown*>(ident->QueryInterface(&g_iidGraphics));
        *reinterpret_cast<IUnknown**>(reinterpret_cast<char*>(self) + 8) = graphics;
        if (graphics) {
            graphics->AddRef();
        }
        ident->Release();
    }
}

// this=IRenderTarget. RET 8. Out[0] is IGraphics; extra is camera/mixer.
// FUNCTION: VC1 0x0041a500
static void* DisplayQueryGraphics(IRenderTarget* self, void* a, void* b)
{
    IUnknown** out = static_cast<IUnknown**>(a);
    IUnknown* queried;
    typedef int (__thiscall *Fn14)(IRenderTarget* object);
    void** vtbl;
    unsigned pack[4];

    (void)b;

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

    vtbl = *reinterpret_cast<void***>(self);
    pack[3] = static_cast<unsigned>(reinterpret_cast<Fn14>(vtbl[6])(self));
    pack[2] = static_cast<unsigned>(reinterpret_cast<Fn14>(vtbl[5])(self));
    pack[0] = 0;
    pack[1] = 0;
    if (DisplayQueryFromTarget(
            *reinterpret_cast<void**>(reinterpret_cast<char*>(self) + 8), pack)) {
        queried = *reinterpret_cast<IUnknown**>(reinterpret_cast<char*>(self) + 8);
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

// MOV EAX, 0x004d4c4c; RET. IScriptObject vtable+0x14.
// FUNCTION: VC1 0x0041a650
const char* KdDisplay::Script14()
{
    return kDisplay;
}

// EAX=HRESULT. DXGetErrorString-style lookup; huge table, recovered as a stub.
// STUB: VC1 0x00442e14
static const char* DxGetErrorString(int hr)
{
    (void)hr;
    return "";
}

// EAX unused. LTCG passes extra args in EBX/EDI. RET 4.
// STUB: VC1 0x0045bf40
static void** Display38Body(void** out)
{
    return out;
}

// EAX=malloc(0x24) block. Sets vptrs; returns this.
// FUNCTION: VC1 0x0045c0f0
static void* Display34Construct(void* self)
{
    unsigned* object = static_cast<unsigned*>(self);

    object[1] = 0x004d1d30;
    object[2] = 0x004d1e68;
    object[3] = 0x004d681c;
    object[4] = 0;
    object[5] = 0x004d1034;
    object[6] = 1;
    object[0] = 0x004d683c;
    object[1] = 0x004d684c;
    object[2] = 0x004d685c;
    object[3] = 0x004d6874;
    object[5] = 0x004d6894;
    object[7] = 0;
    object[8] = 0;
    return self;
}

// ESI=mixer. Rebinds camera at +0x50 and font via vtbl+0x38 from g_displayScene.
// FUNCTION: VC1 0x0045cda0
static void Display10Scene(void* self)
{
    IUnknown* camera;
    IUnknown* font;
    typedef void (__thiscall *Fn38)(void* mixer, IUnknown** fontSlot);

    camera = *reinterpret_cast<IUnknown**>(static_cast<char*>(self) + 0x50);
    if (camera) {
        camera->Release();
    }
    if (!g_displayScene) {
        *reinterpret_cast<IUnknown**>(static_cast<char*>(self) + 0x50) = 0;
    }
    else {
        camera = static_cast<IUnknown*>(g_displayScene->QueryInterface(&g_iidCamera));
        *reinterpret_cast<IUnknown**>(static_cast<char*>(self) + 0x50) = camera;
        if (camera) {
            camera->AddRef();
        }
    }
    font = 0;
    if (g_displayScene) {
        font = static_cast<IUnknown*>(g_displayScene->QueryInterface(&g_iidFont));
        if (font) {
            font->AddRef();
        }
    }
    reinterpret_cast<Fn38>((*reinterpret_cast<void***>(self))[0xe])(self, &font);
    if (font) {
        font->Release();
    }
}

// ESI=mixer, EAX=4 dwords, ECX=IUnknown**. Copies rect to mixer+0x54; rebinds camera +0x50.
// FUNCTION: VC1 0x0045ce30
static unsigned char DisplayQueryFromTarget(void* mixer, void* args)
{
    unsigned* rect = static_cast<unsigned*>(args);
    IUnknown** cameraSlot;
    IUnknown* camera;
    IUnknown* old;
    void* ident;
    unsigned char local[0x48];
    typedef void (__thiscall *Fn10)(void* object, void* arg);
    typedef void (__thiscall *Fn20)(void* object, void* arg);
    typedef void (__thiscall *Fn14)(IUnknown* object, void* a, void* b);

    (void)local;
    *reinterpret_cast<unsigned*>(static_cast<char*>(mixer) + 0x54) = rect[0];
    *reinterpret_cast<unsigned*>(static_cast<char*>(mixer) + 0x58) = rect[1];
    *reinterpret_cast<unsigned*>(static_cast<char*>(mixer) + 0x5c) = rect[2];
    *reinterpret_cast<unsigned*>(static_cast<char*>(mixer) + 0x60) = rect[3];
    reinterpret_cast<Fn10>((*reinterpret_cast<void***>(mixer))[4])(mixer, local);
    reinterpret_cast<Fn20>((*reinterpret_cast<void***>(mixer))[8])(
        mixer, static_cast<char*>(mixer) + 0x54);
    cameraSlot = static_cast<IUnknown**>(mixer);
    (void)cameraSlot;
    camera = 0;
    if (camera) {
        camera->AddRef();
    }
    old = *reinterpret_cast<IUnknown**>(static_cast<char*>(mixer) + 0x50);
    if (old) {
        old->Release();
    }
    *reinterpret_cast<IUnknown**>(static_cast<char*>(mixer) + 0x50) = camera;
    if (camera) {
        ident = *reinterpret_cast<IUnknown**>(static_cast<char*>(mixer) + 0x48);
        if (ident) {
            ident = static_cast<IUnknown*>(
                static_cast<IUnknown*>(ident)->QueryInterface(&g_iidAbstract));
            if (ident) {
                ident = static_cast<IUnknown*>(
                    static_cast<IUnknown*>(ident)->QueryInterface(&g_iidAbstract));
                if (ident) {
                    static_cast<IUnknown*>(ident)->AddRef();
                }
            }
        }
        reinterpret_cast<Fn14>((*reinterpret_cast<void***>(camera))[5])(
            camera, &ident, static_cast<char*>(mixer) + 0x54);
        if (ident) {
            static_cast<IUnknown*>(ident)->Release();
        }
    }
    return 1;
}

// EAX=malloc(0x64) scene/mixer block. Identity floats and vptrs.
// FUNCTION: VC1 0x0045cff0
static void* DisplaySceneConstruct(void* self)
{
    unsigned* object = static_cast<unsigned*>(self);

    object[5] = 0xffffffff;
    object[6] = 2;
    object[1] = 0;
    object[2] = 0;
    object[3] = 0x7fffffff;
    object[4] = 0x7fffffff;
    object[7] = 0x3f800000;
    object[0x10] = 0;
    object[0xa] = 0;
    object[0x11] = 0x004d1e68;
    object[0xd] = 0;
    object[8] = 0;
    object[0xe] = 0;
    object[9] = 0;
    object[0xc] = 0;
    object[0xb] = 0x3f800000;
    object[0xf] = 0x3f800000;
    object[0x12] = 0x004d1034;
    object[0x13] = 1;
    object[0] = 0x004d6770;
    object[0x11] = 0x004d67dc;
    object[0x12] = 0x004d67f4;
    object[0x14] = 0;
    object[0x18] = 0;
    object[0x17] = 0;
    object[0x16] = 0;
    object[0x15] = 0;
    return self;
}

// this = IDisplay (KdDisplay+0x14). IDisplay 0x004d4e50, IShutdownable +4, IAbstract +8.
// FUNCTION: VC1 0x00463e60
void* DisplayFaceQueryInterface(IDisplay* self, const char** iid)
{
    const char* name = *iid;
    if (name == g_iidDisplay) {
        if (reinterpret_cast<char*>(self) - 4 == 0) {
            return 0;
        }
        return self;
    }
    if (name == g_iidShutdownable) {
        if (reinterpret_cast<char*>(self) - 4 == 0) {
            return 0;
        }
        return reinterpret_cast<char*>(self) + 4;
    }
    if (name == g_iidAbstract) {
        return reinterpret_cast<char*>(self) + 8;
    }
    return 0;
}

// this=IShutdownable. Rebind IGraphics, destroy g_displayPyro.
// FUNCTION: VC1 0x00464010
static void DisplayReleaseGraphics(IShutdownable* self)
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
    *reinterpret_cast<unsigned*>(reinterpret_cast<char*>(self) + 0x0c) = 0;
    if (g_displayPyro) {
        free(g_displayPyro);
        g_displayPyro = 0;
    }
}

// this=IDisplay. malloc(0x18) Pyro object into g_displayPyro; RET 8.
// FUNCTION: VC1 0x004641b0
static unsigned char DisplayInitPyro(IDisplay* self, unsigned width, unsigned height)
{
    unsigned* block;

    (void)width;
    (void)height;
    block = static_cast<unsigned*>(malloc(0x18));
    if (!block) {
        g_displayPyro = 0;
        HostLogError(kCantInitPyro);
        return 0;
    }
    block[1] = 0;
    block[2] = 0;
    block[4] = 0;
    block[5] = 0;
    block[3] = 0;
    g_displayPyro = block;
    (void)self;
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

// cdecl arg. EndScene on the swap chain; optional gamma; clear g_d3dInScene if arg!=0.
// FUNCTION: VC1 0x004643f0
static void Display10Present(int arg)
{
    typedef int (__stdcall *FnEnd)(IUnknown* swap);
    typedef int (__stdcall *FnGamma)(IUnknown* device, int kind, int unused, unsigned ramp);
    typedef int (__stdcall *FnGamma4)(IUnknown* device, int a, int b, unsigned c, int d, unsigned e);
    typedef int (__stdcall *FnGetBack)(IUnknown* swap, int a, int b, unsigned* out, int c);

    if (g_d3dInScene == 0) {
        return;
    }
    reinterpret_cast<FnEnd>((*reinterpret_cast<void***>(g_d3dSwap))[0xc])(g_d3dSwap);
    if (g_d3dGammaRamp) {
        if (g_d3dGammaKind == 2) {
            reinterpret_cast<FnGamma>((*reinterpret_cast<void***>(g_d3dDevice))[0x46])(
                g_d3dDevice, 2, 0, g_d3dGammaRamp);
        }
        else if (g_d3dGammaKind == 3) {
            reinterpret_cast<FnGamma>((*reinterpret_cast<void***>(g_d3dDevice))[0x46])(
                g_d3dDevice, 4, 0, g_d3dGammaRamp);
        }
        else if (g_d3dGammaKind == 4) {
            reinterpret_cast<FnGamma4>((*reinterpret_cast<void***>(g_d3dDevice))[0x47])(
                g_d3dDevice, 4, 0, g_d3dGammaRamp * 4, 0, g_d3dGammaRamp * 2);
        }
        g_d3dGammaRamp = 0;
    }
    if (arg == 0) {
        reinterpret_cast<FnGetBack>((*reinterpret_cast<void***>(g_d3dSwap))[0xb])(
            g_d3dSwap, 0, 0, &g_d3dInScene, 0);
        return;
    }
    g_d3dInScene = 0;
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
    if (g_d3dSwap) {
        if (g_d3dInScene) {
            reinterpret_cast<Fn30>((*reinterpret_cast<void***>(g_d3dSwap))[0xc])(g_d3dSwap);
            g_d3dInScene = 0;
        }
        if (g_d3dDevice) {
            reinterpret_cast<Fn14c>((*reinterpret_cast<void***>(g_d3dDevice))[0x53])(
                g_d3dDevice, 0, 0, 0x18);
        }
        g_d3dSwap->Release();
        g_d3dSwap = 0;
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

// EAX/EDX mode size. Called after a successful device reset.
// STUB: VC1 0x00464ab0
static void DisplayD3dSetBuffers(void* a, int b, int c, int d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
}

// STRING: VC1 0x004d248c
static const char kPercentS[] = "%s";
// STRING: VC1 0x004d5f80
static const char kDxErrorFmt[] = "%s: %s";
// STRING: VC1 0x004d5f88
static const char kDirectXError[] = "DirectX Error";
// STRING: VC1 0x004d5f98
static const char kSceneAlreadyRendered[] = "Gfx_BeginScene: Scene is already being rendered";

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
    HostLogError(buf);
    if (flag != 0) {
        MessageBoxA(g_kdDisplayWindow, buf, kDirectXError, MB_ICONHAND);
    }
}

// Recreate surfaces after IDirect3DDevice8::Reset. Returns AL.
// STUB: VC1 0x00464de0
static unsigned char DisplayD3dRecreate()
{
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
    if (g_d3dSwap) {
        reinterpret_cast<Fn14c>((*reinterpret_cast<void***>(g_d3dDevice))[0x53])(
            g_d3dDevice, 0, 0, 0x18);
        g_d3dSwap->Release();
    }
    reinterpret_cast<FnReset>((*reinterpret_cast<void***>(g_d3dDevice))[0xe])(
        g_d3dDevice, g_displayMode);
    return DisplayD3dRecreate() != 0;
}

// Direct3DCreate8 path. Returns AL success.
// STUB: VC1 0x004652b0
static unsigned char DisplayD3DInit()
{
    return 1;
}

// D3D TestCooperativeLevel / reset. Returns AL; EBX is the last mixer.
// FUNCTION: VC1 0x00465cc0
static unsigned char DisplayQueryReady()
{
    typedef int (__stdcall *FnTest)(IUnknown* device);
    typedef int (__stdcall *FnSetRender)(IUnknown* device, int state, void* value);
    typedef int (__stdcall *FnBegin)(IUnknown* device);
    typedef int (__stdcall *FnGetBack)(IUnknown* swap, int a, int b, unsigned* out, int c);
    int hr;
    char buf[520];

    buf[0] = 0;
    hr = reinterpret_cast<FnTest>((*reinterpret_cast<void***>(g_d3dDevice))[3])(g_d3dDevice);
    if (hr == static_cast<int>(0x88760868)) {
        return 0;
    }
    if (hr == static_cast<int>(0x88760869)) {
        if (!DisplayD3dReset()) {
            DisplayD3dLogHresult(hr, 0);
            return 0;
        }
    }
    if (g_d3dInScene) {
        FormatBuffer(buf, sizeof(buf), kPercentS, kSceneAlreadyRendered);
        HostLogError(buf);
        return 0;
    }
    if (g_d3dLastMixer) {
        DisplayD3dSetBuffers(g_d3dLastMixer, 0, 0, 0);
    }
    reinterpret_cast<FnBegin>((*reinterpret_cast<void***>(g_d3dDevice))[0x32])(g_d3dDevice);
    if (g_d3dSwap) {
        reinterpret_cast<FnGetBack>((*reinterpret_cast<void***>(g_d3dSwap))[0xb])(
            g_d3dSwap, 0, 0, &g_d3dInScene, 0);
    }
    return 1;
}

// this = IDisplay. Called when Display28 arg is 0 or enabled at IDisplay+0x44.
// FUNCTION: VC1 0x00465e70
static int Display28Apply()
{
    int interval;

    if (g_d3dInScene || g_displayBusy) {
        return 0;
    }
    if (!g_d3dDevice) {
        g_displaySkipDevice = 0;
        return 0;
    }
    if (g_displaySkipDevice == 0) {
        g_displaySkipDevice = 0;
        return 0;
    }
    if (!g_displayReady || !g_displayModeReady) {
        return 0;
    }
    if (g_displaySkipDevice) {
        GetWindowRect(g_kdDisplayWindow, reinterpret_cast<LPRECT>(&g_displayWindowX));
    }
    g_displaySkipDevice = 0;
    interval = Display28Mode(g_displayMode ? static_cast<unsigned>(g_displayMode[2]) : 0);
    g_displayPresentInterval = ((3 < interval) - 1 & 0xfffffff0) + 0x20;
    DisplayD3dReset();
    return Display28Finish();
}

// MOV EAX, [0x004e8998]; RET. IDisplay vtable+0x18 body.
// FUNCTION: VC1 0x00466290
int KdDisplay::Display18()
{
    return g_displayHeight;
}

// MOV EAX, [0x004e8744]; RET. IDisplay vtable+0x14 body.
// FUNCTION: VC1 0x004662a0
int KdDisplay::Display14()
{
    return g_displayWidth;
}

// this = IDisplay (KdDisplay+0x14). mixer at IDisplay+0x18 = object+0x2c.
// FUNCTION: VC1 0x004662b0
void* IDisplay::GetMixer()
{
    return *reinterpret_cast<void**>(reinterpret_cast<char*>(this) + 0x18);
}

// MOV AL, [0x004e74b6]; RET. IDisplay vtable+0x24.
// FUNCTION: VC1 0x004662c0
unsigned char KdDisplay::Display24()
{
    return g_displaySkipDevice;
}

// Guarded static IUnknown*. IRenderTarget vtable+0x1c; IDisplay thunk SUB ECX, 0x10.
// FUNCTION: VC1 0x004662e0
void* KdDisplay::Display1c()
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
void KdDisplay::Display30(int unused)
{
    (void)unused;
}

// FUNCTION: VC1 0x00466320
void* DisplayBaseQueryInterface(KdDisplay* self, const char** iid)
{
    const char* name = *iid;
    if (name == g_iidScriptObject) {
        return self;
    }
    if (name == g_iidRenderTarget) {
        if (!self) {
            return 0;
        }
        return static_cast<IRenderTarget*>(self);
    }
    {
        void* found = DisplayFaceQueryInterface(static_cast<IDisplay*>(self), iid);
        if (found) {
            return found;
        }
        if (name == g_iidAbstract) {
            return static_cast<IAbstract*>(self);
        }
    }
    return 0;
}

// malloc 0x24, construct, QI IAbstract at +0x14. RET 4. IDisplay vtable+0x34.
// FUNCTION: VC1 0x004663b0
void** KdDisplay::Display34(void** out)
{
    void* object;
    IUnknown* queried;
    IUnknown* face;

    (void)this;
    object = malloc(0x24);
    if (!object) {
        queried = 0;
    }
    else {
        object = Display34Construct(object);
        if (!object) {
            queried = 0;
        }
        else {
            face = reinterpret_cast<IUnknown*>(reinterpret_cast<char*>(object) + 0x14);
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

// this = IShutdownable. Called at the start of Shutdown.
// FUNCTION: VC1 0x00466400
static int DisplayShutdownPrep(IShutdownable* self)
{
    void** mixerSlot;
    unsigned char* mixer;
    HWND hwnd;
    void* node;
    void* next;
    void* prev;

    DisplayReleaseGraphics(self);
    DisplayD3DTeardown();

    mixerSlot = reinterpret_cast<void**>(reinterpret_cast<char*>(self) + 0x14);
    mixer = static_cast<unsigned char*>(*mixerSlot);
    if (!mixer) {
        return 0;
    }

    node = g_heapPageList;
    while (node) {
        next = *reinterpret_cast<void**>(static_cast<unsigned char*>(node) + 0xc);
        if (*reinterpret_cast<int*>(static_cast<unsigned char*>(node) + 4) == 0
            && *reinterpret_cast<void**>(static_cast<unsigned char*>(node) + 8) == mixer) {
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

    hwnd = *reinterpret_cast<HWND*>(mixer + 4);
    KillTimer(hwnd, 1);
    SetWindowLongA(hwnd, GWL_USERDATA, 0);
    free(mixer);
    DestroyWindow(hwnd);
    *mixerSlot = 0;
    return 0;
}

// ESI=this. Reset vptrs, ShutdownPrep(IShutdownable), clear g_kdDisplay, Release [this+0xc].
// FUNCTION: VC1 0x004664a0
static void DisplayDtorBase(KdDisplay* self)
{
    IUnknown** slot;
    IUnknown* object;

    DisplayShutdownPrep(static_cast<IShutdownable*>(self));
    g_kdDisplay = 0;
    slot = reinterpret_cast<IUnknown**>(reinterpret_cast<char*>(self) + 0x0c);
    object = *slot;
    if (object) {
        object->Release();
    }
    *slot = 0;
}

// ESI=this, EDI=IRenderTarget. refs=1, mixer=0, returns this.
// FUNCTION: VC1 0x004665c0
static void ConstructDisplayBase(KdDisplay* self)
{
    RenderTargetInit(static_cast<IRenderTarget*>(self));
    self->refs = 1;
    g_displayPyro = 0;
    self->mixer = 0;
}

// RET 0xc. IDisplay vtable+0x38. Orig loads 3 stdcall args then cdecl-calls 0x0045bf40.
// STUB: VC1 0x00466660
void** KdDisplay::Display38(void** out, void* a, void* b)
{
    (void)this;
    (void)a;
    (void)b;
    Display38Body(out);
    return out;
}

// this=IDisplay. HostLog Init, CreateKdWindow into mixer at +0x18, ShowWindow.
// FUNCTION: VC1 0x00466690
static unsigned char DisplaySetSizeBody(IDisplay* self, unsigned width, unsigned height)
{
    void** mixerSlot;
    unsigned char* mixer;
    HWND hwnd;
    RECT client;
    typedef int (__thiscall *Fn18)(void* object);

    HostLog(kInitSize, width, height);
    mixerSlot = reinterpret_cast<void**>(reinterpret_cast<char*>(self) + 0x18);
    *mixerSlot = CreateKdWindow();
    mixer = static_cast<unsigned char*>(*mixerSlot);
    if (!mixer) {
        return 0;
    }

    hwnd = *reinterpret_cast<HWND*>(mixer + 4);
    if (GetKdEnv("KD_PARENT_WINDOW")) {
        if (GetClientRect(hwnd, &client)) {
            width = static_cast<unsigned>(client.right - client.left);
            height = static_cast<unsigned>(client.bottom - client.top);
        }
    }
    else {
        int frameY;
        int caption;
        int frameX;
        *reinterpret_cast<unsigned*>(mixer + 0x10) = width;
        *reinterpret_cast<unsigned*>(mixer + 0x14) = height;
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

    if (!GetKdEnv("KD_PARENT_WINDOW")) {
        void* object = reinterpret_cast<char*>(self) - 0x14;
        void** vtbl = *reinterpret_cast<void***>(object);
        reinterpret_cast<Fn18>(vtbl[6])(object);
        SetWindowPos(g_kdDisplayWindow, 0, g_displayWindowX, g_displayWindowY, 0, 0,
            SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    }

    if (!DisplayD3DInit()) {
        HostLogError(kCantInit);
        return 0;
    }
    return DisplayInitPyro(self, width, height);
}

// STUB: VC1 0x00466b00
static unsigned char Script0cBody(void* a, void* b)
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

    env = GetKdEnv("KD_PARENT_WINDOW");
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
    void* found = DisplayBaseQueryInterface(this, iid);
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
    unsigned* p = reinterpret_cast<unsigned*>(iface + 1);
    unsigned n;

    *p -= 1;
    n = *p;
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

// this = IShutdownable (KdDisplay+0x18). Releases target at +0x18, then QI g_iidRenderTarget.
// FUNCTION: VC1 0x004b1da0
int KdDisplay::Shutdown()
{
    IShutdownable* self = static_cast<IShutdownable*>(this);
    IUnknown** ptarget = reinterpret_cast<IUnknown**>(reinterpret_cast<char*>(self) + 0x18);
    IUnknown* object;
    IUnknown* queried;

    DisplayShutdownPrep(self);
    object = *ptarget;
    if (object) {
        object->Release();
    }
    if (!g_kdQueryUnknown) {
        *ptarget = 0;
        return 0;
    }
    queried = static_cast<IUnknown*>(g_kdQueryUnknown->QueryInterface(&g_iidRenderTarget));
    *ptarget = queried;
    if (!queried) {
        return 0;
    }
    return static_cast<int>(queried->AddRef());
}

// EAX=this. Calls FUN_004665c0 with this in ESI, then overwrites vptrs/floats.
// FUNCTION: VC1 0x004b1e40
KdDisplay* ConstructKdDisplay(KdDisplay* self)
{
    ConstructDisplayBase(self);
    self->KdDisplay::KdDisplay();
    return self;
}

// FUNCTION: VC1 0x004b1ea0
void** CreateKdDisplay(void** out)
{
    IUnknown* queried;
    KdDisplay* object;

    if (g_kdDisplay) {
        queried = static_cast<IUnknown*>(
            g_kdDisplay->QueryInterface(&g_iidAbstract));
        *out = queried;
        if (queried) {
            queried->AddRef();
        }
        return out;
    }

    object = static_cast<KdDisplay*>(malloc(sizeof(KdDisplay)));
    if (!object) {
        g_kdDisplay = 0;
        *out = 0;
        return out;
    }
    ConstructKdDisplay(object);
    g_kdDisplay = static_cast<IAbstract*>(object);
    queried = static_cast<IUnknown*>(object->QueryInterface(&g_iidAbstract));
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

// this=IRenderTarget (WBA from IDisplay). Draws via target then presents D3D.
// FUNCTION: VC1 0x004b1f50
void* KdDisplay::Display10()
{
    IRenderTarget* self = static_cast<IRenderTarget*>(this);
    IUnknown* target;
    IUnknown* camera;
    void* graphics;
    void* result;
    typedef void (__thiscall *Fn10)(IUnknown* object);
    typedef void (__thiscall *Fn44)(IUnknown* object, float* matrix);
    typedef void (__thiscall *Fn20)(IUnknown* object, int* rect);
    typedef void (__thiscall *Fn3c)(IUnknown* object, unsigned color);
    typedef void* (__thiscall *Fn1c)(IUnknown* object, float a, float b, float c, float d,
        float w, float h);
    typedef void (__thiscall *Fn58)(IUnknown* object, void* value);
    typedef void (__stdcall *DeviceEnd)(IUnknown* object);
    typedef void (__stdcall *DeviceClear)(IUnknown* object, unsigned a, unsigned b,
        unsigned c, unsigned d);

    target = *reinterpret_cast<IUnknown**>(reinterpret_cast<char*>(self) + 0x2c);
    if (g_displaySkipDevice == 0 && target) {
        Fn10 fn10 = reinterpret_cast<Fn10>((*reinterpret_cast<void***>(target))[4]);
        fn10(target);
        camera = 0;
        if (g_kdQueryUnknown) {
            camera = static_cast<IUnknown*>(g_kdQueryUnknown->QueryInterface(&g_iidCamera));
            if (camera) {
                camera->AddRef();
            }
        }
        graphics = 0;
        DisplayQueryGraphics(self, &graphics, &camera);
        if (camera) {
            camera->Release();
        }
        if (graphics) {
            int rect[4];
            Fn44 fn44 = reinterpret_cast<Fn44>((*reinterpret_cast<void***>(graphics))[0x11]);
            fn44(static_cast<IUnknown*>(graphics),
                reinterpret_cast<float*>(reinterpret_cast<char*>(self) + 0x30));
            rect[0] = g_displayViewLeft;
            rect[1] = g_displayViewTop;
            rect[2] = g_displayViewRight;
            rect[3] = g_displayViewBottom;
            Fn20 fn20 = reinterpret_cast<Fn20>((*reinterpret_cast<void***>(graphics))[8]);
            fn20(static_cast<IUnknown*>(graphics), rect);
            Fn3c fn3c = reinterpret_cast<Fn3c>((*reinterpret_cast<void***>(graphics))[0xf]);
            fn3c(static_cast<IUnknown*>(graphics), 0xff000000);
            Fn1c fn1c = reinterpret_cast<Fn1c>((*reinterpret_cast<void***>(target))[7]);
            result = fn1c(target, 0.0f, 0.0f, 0.0f, 0.0f,
                static_cast<float>(g_displayWidth), static_cast<float>(g_displayHeight));
            Fn58 fn58 = reinterpret_cast<Fn58>((*reinterpret_cast<void***>(graphics))[0x16]);
            fn58(static_cast<IUnknown*>(graphics), result);
            static_cast<IUnknown*>(graphics)->Release();
        }
        else {
            result = 0;
        }
    }
    else {
        result = 0;
    }

    Display10Scene(*reinterpret_cast<void**>(reinterpret_cast<char*>(self) + 8));
    Display10Present(1);
    if (g_d3dDevice) {
        DeviceEnd endScene = reinterpret_cast<DeviceEnd>(
            (*reinterpret_cast<void***>(g_d3dDevice))[0x23]);
        endScene(g_d3dDevice);
        if (g_d3dLastMixer == 0) {
            DeviceClear present = reinterpret_cast<DeviceClear>(
                (*reinterpret_cast<void***>(g_d3dDevice))[0xf]);
            present(g_d3dDevice, 0, 0, 0, 0);
        }
    }
    return result;
}

// this = IRenderTarget. [ECX+0x2c] is target (object+0x30). RET 8.
// FUNCTION: VC1 0x004b20c0
void* KdDisplay::Display0c(void* a, void* b)
{
    IUnknown* object;
    void** vtbl;
    typedef void* (__thiscall *Fn0c)(void* self, void* a, void* b);
    IRenderTarget* self = static_cast<IRenderTarget*>(this);

    object = *reinterpret_cast<IUnknown**>(reinterpret_cast<char*>(self) + 0x2c);
    if (g_displaySkipDevice == 0 && object) {
        vtbl = *reinterpret_cast<void***>(object);
        reinterpret_cast<Fn0c>(vtbl[3])(object, a, b);
        return a;
    }
    DisplayQueryGraphics(self, a, b);
    return a;
}

// this = IDisplay. arg in [ESP+8], enabled at IDisplay+0x44 (object+0x58). RET 4.
// STUB: VC1 0x004b2100
int IDisplay::Display28(unsigned char arg)
{
    if (arg == 0 || *(reinterpret_cast<unsigned char*>(this) + 0x44) != 0) {
        return Display28Apply();
    }
    return 0;
}

// this = IDisplay. Calls 0x00466690 then rebuilds the 3x3 at IDisplay+0x20.
// FUNCTION: VC1 0x004b2120
int KdDisplay::SetSize(unsigned width, unsigned height)
{
    IDisplay* self = static_cast<IDisplay*>(this);
    int viewW;
    int viewH;
    void* slot;
    float* matrix;

    if (!DisplaySetSizeBody(self, width, height)) {
        return 0;
    }
    if (GetKdEnv("KD_PARENT_WINDOW")) {
        return 1;
    }

    viewW = g_displayViewRight - g_displayViewLeft;
    viewH = g_displayViewBottom - g_displayViewTop;
    if (g_displayWidth == viewW && g_displayHeight == viewH) {
        return 1;
    }

    slot = 0;
    self->Display38(&slot, reinterpret_cast<void*>(g_displayWidth),
        reinterpret_cast<void*>(g_displayHeight));
    if (slot) {
        static_cast<IUnknown*>(slot)->Release();
    }

    matrix = reinterpret_cast<float*>(reinterpret_cast<char*>(self) + 0x20);
    if (!*reinterpret_cast<void**>(reinterpret_cast<char*>(self) + 0x1c)) {
        HostLogError(kCantCreateRt);
        return 0;
    }

    matrix[0] = 1.0f;
    matrix[1] = 0.0f;
    matrix[2] = 0.0f;
    matrix[3] = 0.0f;
    matrix[4] = 1.0f;
    matrix[5] = 0.0f;
    matrix[6] = 0.0f;
    matrix[7] = 0.0f;
    matrix[8] = 1.0f;
    (void)viewW;
    (void)viewH;
    return 1;
}

// STUB: VC1 0x004b22e0
unsigned char KdDisplay::Script0c(void* a, void* b)
{
    return Script0cBody(a, b) != 0;
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
