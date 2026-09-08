#include "kd/display/display.h"

#include "kd/iface/iunknown.h"
#include "kd/iface/iid.h"

#include <stdlib.h>

namespace vc1 {

// GLOBAL: VC1 0x004ed54c
IAbstract* g_kdDisplay;
// GLOBAL: VC1 0x004ed65c
IUnknown* g_kdQueryUnknown;

// STRING: VC1 0x004d4c4c
static const char kDisplay[] = "Display";
// GLOBAL: VC1 0x004d0630
const char* g_kdNameDisplay = kDisplay;

// GLOBAL: VC1 0x004ee284
static IUnknown* g_display1cObject;
// GLOBAL: VC1 0x004ee288
static unsigned g_display1cGuard;

static void DisplayDtorBase(KdDisplay* self);
static void ConstructDisplayBase(KdDisplay* self);
static unsigned char Script0cBody(void* a, void* b);
static void Display1cAtexit();
static void* DisplayQueryGraphics(void* self, void* a, void* b);
static void* Display34Construct(void* self);
static void** Display38Body(void** out);
static int Display28Apply();
static int DisplayShutdownPrep(IShutdownable* self);

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

// this = IRenderTarget (KdDisplay+0x04). IDisplay thunk SUB ECX, 0x10.
// STUB: VC1 0x0041a500
static void* DisplayQueryGraphics(void* self, void* a, void* b)
{
    (void)self;
    (void)b;
    return a;
}

// MOV EAX, 0x004d4c4c; RET. IScriptObject vtable+0x14.
// FUNCTION: VC1 0x0041a650
const char* KdDisplay::Script14()
{
    return kDisplay;
}

// EAX unused. LTCG passes extra args in EBX/EDI. RET 4.
// STUB: VC1 0x0045bf40
static void** Display38Body(void** out)
{
    return out;
}

// EAX=malloc(0x24) block. Sets vptrs; returns this.
// STUB: VC1 0x0045c0f0
static void* Display34Construct(void* self)
{
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

// this = IDisplay. Called when Display28 arg is 0 or enabled at IDisplay+0x44.
// STUB: VC1 0x00465e70
static int Display28Apply()
{
    return 0;
}

// GLOBAL: VC1 0x004e74b6
unsigned char g_displaySkipDevice;
// GLOBAL: VC1 0x004e8744
int g_displayWidth;
// GLOBAL: VC1 0x004e8998
int g_displayHeight;

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
// STUB: VC1 0x00466400
static int DisplayShutdownPrep(IShutdownable* self)
{
    (void)self;
    return 0;
}

// STUB: VC1 0x004664a0
static void DisplayDtorBase(KdDisplay* self)
{
    (void)self;
}

// STUB: VC1 0x004665c0
static void ConstructDisplayBase(KdDisplay* self)
{
    (void)self;
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

// STUB: VC1 0x00466b00
static unsigned char Script0cBody(void* a, void* b)
{
    (void)a;
    (void)b;
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

// STUB: VC1 0x004b1f50
void* KdDisplay::Display10()
{
    return 0;
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
// STUB: VC1 0x004b2120
int KdDisplay::SetSize(unsigned width, unsigned height)
{
    (void)width;
    (void)height;
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
