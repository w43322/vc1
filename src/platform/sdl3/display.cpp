// SDL3 implementation of the engine display interfaces. Original mappings
// remain in kd/display/display.cpp; this backend is not a byte-matching target.
#include "kd/display/display.h"
#include "kd/display/effect.h"
#include "kd/display/graphics.h"
#include "kd/display/pyro.h"
#include "kd/display/render_target.h"
#include "kd/display/texture.h"
#include "kd/factory.h"
#include "kd/iface/iid.h"
#include "host/platform.h"
#include "host/window.h"
#include "bindings/sqplus.h"
#include "bindings/component.h"
#include "platform/sdl3/render.h"
#include "mod/mods.h"
#include "kd/display/jpeg_stream.h"
#include <tga.h>
#include <vector>
#include <algorithm>
#include <cmath>
#include <new>
#include <cstdlib>
#include <cstring>
namespace vc1 {
IAbstract* g_kdDisplay;
int* g_displayMode;
int g_batchCount;
Matrix3 g_displayMatrixInv;
IUnknown* g_displayScene;
IUnknown* g_graphicsQuery;
PyroLibrary* g_displayPyro;
IUnknown* g_kdQueryUnknown;
IUnknown* g_graphics;
IUnknown* g_graphicsHost;
bool g_displaySkipDevice;
int g_displayWidth;
int g_displayHeight;
TrackedTexture* g_d3dChain2;
IUnknown* g_nullRenderTexture;
unsigned g_nullRenderTextureGuard;
extern const char kPercentS[] = "%s";
extern const char kReadBinary[] = "rb";
static const char kTexture[] = "Texture";
static const char kDisplay[] = "Display";
static const char kRenderTarget[] = "RenderTarget";
static const char kCantInitPyro[] = "Can't init Pyro";
const char* g_kdNameDisplay = kDisplay;
const char* g_kdNameTexture = kTexture;
const char* g_kdNameRenderTarget = kRenderTarget;
static void ReleaseNullRenderTexture();
static unsigned char DisplayQueryReady(KdRenderTargetNode* node);
static void FlushBatch(char final);
static int ApplyWindowedMode(bool windowed);
IPyroDevice::~IPyroDevice() {}
KdDisplayDevice::~KdDisplayDevice() { g_kdDisplay = 0; }
KdDisplayBase::~KdDisplayBase() { KdDisplayBase::Shutdown(); }
KdDisplay::~KdDisplay() { if (target) target->Release(); target = 0; }
KdDisplayDevice::KdDisplayDevice()
{
    g_displayPyro = 0;
}

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

float MinFloat(float a, float b)
{
    if (a < b) {
        return a;
    }
    return b;
}

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

const char* KdDisplayBase::GetTypeName()
{
    return kDisplay;
}

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

int IPyroDevice::DrawQuadBuffer(PyroDeviceBuffer* buffer, int first, int count, int flags)
{
    (void)buffer;
    (void)first;
    (void)count;
    (void)flags;
    return 1;
}

int IPyroDevice::DrawQuads(PyroVertexBuffer* buffer, int first, int count, int flags)
{
    return DrawQuadBuffer(static_cast<PyroVertexBufferAdapter*>(buffer)->buffer, first, count,
        flags);
}

int KdPyroBuffer::Upload(const void* source, int offset, int size, int discard)
{
    (void)discard;
    memcpy(data + offset, source, size);
    return 0;
}

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

void* KdPyroBuffer::GetData()
{
    return data;
}

int KdDisplayDevice::IsRgba()
{
    return 0;
}

int KdDisplayDevice::DrawQuads(PyroVertexBuffer* buffer, int first, int count, int flags)
{
    return DrawQuadBuffer(static_cast<KdPyroBuffer*>(buffer), first, count, flags);
}

int KdDisplayDevice::SetTexture(void* a, PyroDeviceObject* texture)
{
    (void)a;
    pyroTexture = texture;
    return 0;
}

int KdDisplayDevice::SetWorldMatrix(void* a)
{
    (void)a;
    return 0;
}

int KdDisplayDevice::Unused(void* a, void* b, void* c, void* d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return 1;
}

int KdDisplayDevice::SetBlendMode(unsigned mode, unsigned value)
{
    if (mode == 4 && value == 1) {
        mode18 = 0;
        return 0;
    }
    mode18 = 2;
    return 0;
}

int KdDisplayDevice::SetRenderState(int a, int b)
{
    (void)a;
    (void)b;
    return 0;
}

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
    const unsigned mode = static_cast<IGraphics*>(g_graphics)->GetMode();
    static_cast<IGraphics*>(g_graphics)->SetMode(mode18);
    static_cast<IGraphics*>(g_graphics)->DrawQuads(texture,
        static_cast<unsigned char*>(buffer->GetData()) + first * 0x60, count);
    static_cast<IGraphics*>(g_graphics)->SetMode(mode);
    return 0;
}

int KdDisplayDevice::CreateImage(PyroDeviceImage** out, void* pixels, int width, int height,
    int flags)
{
    *out = new PyroDeviceImage(pixels, width, height, flags);
    return 0;
}

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

unsigned KdDisplayDevice::Release()
{
    unsigned value = --refs;
    if (value == 0) {
        delete static_cast<IAbstract*>(this);
        return 0;
    }
    return value;
}

unsigned KdDisplayDevice::AddRef()
{
    unsigned value = refs;
    if (value != 0) {
        ++value;
        refs = value;
    }
    return value;
}

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

void ReleaseTrackedTexture(IUnknown* texture)
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
        D3dRelease(texture);
    }
}

int KdDisplayBase::GetHeight()
{
    return g_displayHeight;
}

int KdDisplayBase::GetWidth()
{
    return g_displayWidth;
}

void* KdDisplayBase::GetMixer()
{
    return mixer;
}

bool KdDisplayBase::IsWindowed()
{
    return g_displaySkipDevice;
}

unsigned char KdDisplayBase::SetMember(void* a, void* b)
{
    (void)a;
    (void)b;
    return 0;
}

void* KdDisplayBase::GetRenderTexture()
{
    if ((g_nullRenderTextureGuard & 1) == 0) {
        g_nullRenderTextureGuard |= 1;
        g_nullRenderTexture = 0;
        atexit(ReleaseNullRenderTexture);
    }
    return &g_nullRenderTexture;
}

void KdDisplayBase::ShowKeyboard(int unused)
{
    (void)unused;
}

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

unsigned KdDisplayBase::Release()
{
    unsigned value = --refs;
    if (value == 0) {
        delete static_cast<IAbstract*>(this);
        return 0;
    }
    return value;
}

unsigned KdDisplayBase::AddRef()
{
    unsigned value = refs;
    if (value != 0) {
        ++value;
        refs = value;
    }
    return value;
}

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

static void DisplayEventCallback(const KdEvent* event)
{
    KdEvent* posted;

    if (event->type == 0x2c) {

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

KdDisplayBase::KdDisplayBase()
{
    mixer = 0;
}

void* KdDisplayBase::BeginFrame(void* a, void* b)
{
    IRenderTarget::BeginFrame(a, b);
    return a;
}

KdPtr<IRenderTarget> KdDisplayBase::CreateRenderTarget(unsigned width, unsigned height)
{
    KdPtr<IRenderTarget> result;
    CreateRenderTargetBody(reinterpret_cast<void**>(&result.ptr), width, height);
    return result;
}

int KdDisplayBase::SetWindowed(bool arg)
{
    return ApplyWindowedMode(arg);
}

unsigned char KdDisplayBase::GetMember(const char* name, SquirrelObject* out)
{
    static ScriptMethodTable methods;

    if (methods.Empty()) {

        methods["CreateRenderTarget"] = CreateScriptMethod<KdDisplayBase>(&KdDisplayBase::CreateRenderTarget);
        methods[kScriptGetWidth] = CreateScriptMethod<KdDisplayBase>(&KdDisplayBase::GetWidth);
        methods[kScriptGetHeight] = CreateScriptMethod<KdDisplayBase>(&KdDisplayBase::GetWidth);

        methods["IsWindowed"] = CreateScriptMethod<KdDisplayBase>(&KdDisplayBase::IsWindowed);

        methods["SetWindowed"] = CreateScriptMethod<KdDisplayBase>(&KdDisplayBase::SetWindowed);

        methods["ShowKeyboard"] = CreateScriptMethod<KdDisplayBase>(&KdDisplayBase::ShowKeyboard);
    }
    return methods.Find(name, out);
}

unsigned char KdDisplay::SetMember(void* a, void* b)
{
    (void)a;
    (void)b;
    return 0;
}

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

unsigned KdDisplay::AddRef()
{
    unsigned n = refs;
    if (n != 0) {
        n += 1;
        refs = n;
    }
    return n;
}

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

KdDisplay::KdDisplay() throw()
    : target(0), enabled58(1)
{
}

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

int KdDisplay::SetWindowed(bool arg)
{
    if (arg == 0 || enabled58 != 0) {
        return ApplyWindowedMode(arg);
    }
    return 0;
}

unsigned char KdDisplay::GetMember(const char* name, SquirrelObject* out)
{
    return KdDisplayBase::GetMember(name, out) != 0;
}

static void ReleaseNullRenderTexture()
{
    IUnknown* object = g_nullRenderTexture;
    if (object) {
        object->Release();
    }
    g_nullRenderTexture = 0;
}

#include "platform/sdl3/render_impl.h"
}
