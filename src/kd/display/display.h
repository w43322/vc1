#ifndef VC1_KD_DISPLAY_H
#define VC1_KD_DISPLAY_H

#include "kd/iface/iscript_object.h"
#include "kd/iface/irender_target.h"
#include "kd/iface/ipyro_device.h"
#include "pyro/pyro_device.h"
#include "kd/iface/idisplay.h"
#include "kd/iface/ishutdownable.h"
#include "kd/iface/iabstract.h"
#include "kd/display/graphics.h"
#include "kd/display/render_target.h"
#include "kd/display/texture.h"
#include "bindings/matrix3.h"

namespace vc1 {

class KdDisplay;

extern IAbstract* g_kdDisplay;
extern IUnknown* g_d3dDevice;
extern IUnknown* g_displayScene;
extern int* g_displayMode;
extern const char* g_kdNameRenderTarget;

void DisplayD3dSetView(const Matrix3* matrix);

// FVF 0x142 (XYZ | DIFFUSE | TEX1).
// SIZE 0x18
struct BatchVertex
{
    float x;        // 0x00
    float y;        // 0x04
    float z;        // 0x08
    unsigned color; // 0x0c
    float u;        // 0x10
    float v;        // 0x14
};

// Only vertices are copied into the vertex buffer.
// SIZE 0x68
struct BatchQuad
{
    BatchVertex vertices[4]; // 0x00
    IUnknown* texture;       // 0x60  D3D texture, 0 for solid fills
    unsigned flags;          // 0x64  GraphicsState::mode
};

extern int g_batchCount; // display.cpp

void PushQuad(const BatchQuad* quad);
void* BeginQuadBatch(unsigned flags, IUnknown* texture, int* capacity);
void BatchLine(float x0, float y0, float x1, float y1, float z, unsigned color);
void DisplayD3dSetClip(int x, int y, int width, int height);

// Inferred name. A texture loaded from a file and its image size (before the
// power-of-two caps), linked on g_d3dChain2.
// SIZE 0x10
struct TrackedTexture
{
    IUnknown* texture;     // 0x00
    int width;             // 0x04
    int height;            // 0x08
    TrackedTexture* next;  // 0x0c
};

extern TrackedTexture* g_d3dChain2;

void ReleaseTrackedTexture(IUnknown* texture);

// D3D8 objects are COM: methods are __stdcall with this pushed, unlike the
// __thiscall vc1::IUnknown they are stored as.
inline unsigned long D3dRelease(IUnknown* object)
{
    typedef unsigned long (__stdcall *FnRelease)(IUnknown* object);
    return reinterpret_cast<FnRelease>((*reinterpret_cast<void***>(object))[2])(object);
}
IUnknown* LoadTextureFile(const char* name);
void* LockTextureRect(IUnknown* texture, int x, int y, bool readOnly, int width, int height);
IUnknown* CreateManagedTexture(unsigned width, unsigned height);

// Inferred name. Pyro device texture: holds the KD texture CreateImageTexture filled.
// SIZE 0x08
// VTABLE: VC1 0x004d6378
class KdPyroTexture : public PyroDeviceObject
{
public:
    KdRef<ITexture> texture; // 0x04

    explicit KdPyroTexture(KdPtr<ITexture> source);
};

// SYNTHETIC: VC1 0x0041ae70
// vc1::KdPyroTexture::`scalar deleting destructor'

// Inferred name. Pyro device vertex buffer (CreateBuffer hands out the
// PyroDeviceBuffer face, CreateVertexBuffer the PyroVertexBuffer face): a plain heap block
// of count * stride bytes the library uploads into and DrawQuadBuffer draws from.
// SIZE 0x14
// VTABLE: VC1 0x004d6350 vc1::PyroDeviceBuffer
// VTABLE: VC1 0x004d6368 vc1::PyroVertexBuffer
class KdPyroBuffer : public PyroDeviceBuffer, public PyroVertexBuffer
{
public:
    VC1_PYRO_HEAP

    unsigned char* data; // 0x08
    unsigned format;     // 0x0c  0x10009 from PyroLibrary::Init
    int stride;          // 0x10

    KdPyroBuffer(int count, int size, unsigned value)
    {
        data = static_cast<unsigned char*>(malloc(count * size));
        stride = size;
        format = value;
    }
    ~KdPyroBuffer()
    {
        free(data);
    }

    int Lock() { return 0; }
    int Unlock() { return 0; }
    void* GetData();
    int Upload(const void* source, int offset, int size, int discard);
};

// SYNTHETIC: VC1 0x0041ade0
// vc1::KdPyroBuffer::`scalar deleting destructor'
// SYNTHETIC: VC1 0x0041aec0
// ??_EKdPyroBuffer@vc1@@W3AEPAXI@Z

// Inferred name. The default IPyroDevice::CreateVertexBuffer wraps a device
// buffer from CreateBuffer in this when a device has no vertex buffer of its
// own; Upload copies into the device buffer between Lock and Unlock, and the
// wrapper owns (deletes) it.
// SIZE 0x08
// VTABLE: VC1 0x004d6278
class PyroVertexBufferAdapter : public PyroVertexBuffer
{
public:
    VC1_PYRO_HEAP

    PyroDeviceBuffer* buffer; // 0x04

    explicit PyroVertexBufferAdapter(PyroDeviceBuffer* inner) : buffer(inner) {}
    ~PyroVertexBufferAdapter()
    {
        if (buffer) {
            delete buffer;
        }
    }

    int Upload(const void* source, int offset, int size, int discard);
};

// SYNTHETIC: VC1 0x0041ac60
// vc1::PyroVertexBufferAdapter::`scalar deleting destructor'

// Inferred name. Implements the Pyro device plus the device-level IDisplay
// slots (SetSize builds g_displayPyro, CreateEffect) and Shutdown. Constructed
// inline by KdDisplayBase() (0x004665c0). QI/AddRef/Release take the IDisplay
// this (+0x04).
// SIZE 0x1c
// VTABLE: VC1 0x004d62bc vc1::IPyroDevice
// VTABLE: VC1 0x004d62f0 vc1::IDisplay
// VTABLE: VC1 0x004d6330 vc1::IShutdownable
// VTABLE: VC1 0x004d6340 vc1::IAbstract
class KdDisplayDevice : public IPyroDevice,
                        public IDisplay,
                        public IShutdownable,
                        public IAbstract
{
public:
    PyroDeviceObject* pyroTexture; // 0x14  SetTexture stores; Shutdown clears
    unsigned mode18;               // 0x18  SetBlendMode: 0 for (4, 1), else 2

    KdDisplayDevice();
    ~KdDisplayDevice();

    void* QueryInterface(const char** iid); // vtable+0x00
    unsigned AddRef();                      // vtable+0x04
    unsigned Release();                     // vtable+0x08

    int SetRenderState(int a, int b);
    int SetBlendMode(unsigned mode, unsigned value);
    int CreateBuffer(PyroDeviceBuffer** out, int count, int stride, unsigned format, int flags);
    int CreateVertexBuffer(PyroVertexBuffer** out, int count, int stride, unsigned format, int flags);
    int Unused(void* a, void* b, void* c, void* d);
    int CreateImageTexture(PyroDeviceObject** out, PyroDeviceImage* image, int flags);
    int CreateImage(PyroDeviceImage** out, void* pixels, int width, int height, int flags);
    int SetWorldMatrix(void* a);
    int SetTexture(void* a, PyroDeviceObject* texture);
    int DrawQuadBuffer(PyroDeviceBuffer* buffer, int first, int count, int flags);
    int DrawQuads(PyroVertexBuffer* buffer, int first, int count, int flags);
    int IsRgba();

    unsigned char SetSize(unsigned width, unsigned height); // IDisplay vtable+0x20
    void** CreateEffect(void** out);                           // IDisplay vtable+0x3c
    int Shutdown();                                         // IShutdownable vtable+0x0c
};

typedef char Vc1KdDisplayDeviceSizeCheck[sizeof(KdDisplayDevice) == 0x1c ? 1 : -1];

// SYNTHETIC: VC1 0x0041ad70
// vc1::KdDisplayDevice::`scalar deleting destructor'
// SYNTHETIC: VC1 0x0041aeb0
// ?AddRef@KdDisplayDevice@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x0041aed0
// ??_EKdDisplayDevice@vc1@@WM@AEPAXI@Z
// SYNTHETIC: VC1 0x0041aee0
// ?AddRef@KdDisplayDevice@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x0041aef0
// ?Release@KdDisplayDevice@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x0041af00
// ?QueryInterface@KdDisplayDevice@vc1@@W3AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0041af10
// ?Release@KdDisplayDevice@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x0041af20
// ?QueryInterface@KdDisplayDevice@vc1@@W7AEPAXPAPBD@Z

// Inferred name. Adds the script and render-target faces and the window
// (mixer). GetTypeName "Display". Constructor 0x004665c0 (ESI=this) is called by
// KdDisplay(); destructor body 0x004664a0.
// SIZE 0x30
// VTABLE: VC1 0x004d5e00 vc1::IScriptObject
// VTABLE: VC1 0x004d5e1c vc1::IRenderTarget
// VTABLE: VC1 0x004d5e3c vc1::IPyroDevice
// VTABLE: VC1 0x004d5e70 vc1::IDisplay
// VTABLE: VC1 0x004d5eb0 vc1::IShutdownable
// VTABLE: VC1 0x004d5ec0 vc1::IAbstract
class KdDisplayBase : public IScriptObject,
                      public IRenderTarget,
                      public KdDisplayDevice
{
public:
    void* mixer; // 0x2c  IDisplay+0x18

    KdDisplayBase();
    ~KdDisplayBase();

    void* QueryInterface(const char** iid); // vtable+0x00
    unsigned AddRef();                      // vtable+0x04
    unsigned Release();                     // vtable+0x08
    unsigned char GetMember(const char* name, SquirrelObject* out);
    unsigned char SetMember(void* a, void* b);
    const char* GetTypeName();
    void* BeginFrame(void* a, void* b);
    void* EndFrame();
    int GetWidth();
    int GetHeight();
    void* GetRenderTexture();
    unsigned char SetSize(unsigned width, unsigned height);
    bool IsWindowed();
    int SetWindowed(bool arg);
    void* GetMixer();
    void ShowKeyboard(int unused);
    void** CreateTexture(void** out);
    KdPtr<IRenderTarget> CreateRenderTarget(unsigned width, unsigned height);
    int Shutdown() throw();
    virtual int FitWindow(); // IScriptObject vtable+0x18
};

// SYNTHETIC: VC1 0x00402330
// ?GetWidth@KdDisplayBase@vc1@@WBA@AEHXZ
// SYNTHETIC: VC1 0x00402360
// ?GetHeight@KdDisplayBase@vc1@@WBA@AEHXZ
// SYNTHETIC: VC1 0x004023b0
// ?GetRenderTexture@KdDisplayBase@vc1@@WBA@AEPAXXZ
// SYNTHETIC: VC1 0x0041a660
// vc1::KdDisplayBase::`scalar deleting destructor'
// SYNTHETIC: VC1 0x0041a930
// ?BeginFrame@KdDisplayBase@vc1@@WBA@AEPAXPAX0@Z
// SYNTHETIC: VC1 0x0041a940
// ?Release@KdDisplayBase@vc1@@WBI@AEIXZ
// SYNTHETIC: VC1 0x0041a950
// ?QueryInterface@KdDisplayBase@vc1@@WBI@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0041a960
// ?Release@KdDisplayBase@vc1@@WBM@AEIXZ
// SYNTHETIC: VC1 0x0041a970
// ?QueryInterface@KdDisplayBase@vc1@@WBM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0041a980
// ?EndFrame@KdDisplayBase@vc1@@WBA@AEPAXXZ
// SYNTHETIC: VC1 0x0041a990
// ?AddRef@KdDisplayBase@vc1@@WBE@AEIXZ
// SYNTHETIC: VC1 0x0041a9a0
// ??_EKdDisplayBase@vc1@@WBM@AEPAXI@Z
// SYNTHETIC: VC1 0x0041a9b0
// ?AddRef@KdDisplayBase@vc1@@WBI@AEIXZ
// SYNTHETIC: VC1 0x0041a9c0
// ?Release@KdDisplayBase@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x0041a9d0
// ?QueryInterface@KdDisplayBase@vc1@@W3AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0041a9e0
// ?AddRef@KdDisplayBase@vc1@@WBM@AEIXZ
// SYNTHETIC: VC1 0x0041a9f0
// ?AddRef@KdDisplayBase@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x0041aa00
// ??_EKdDisplayBase@vc1@@WBA@AEPAXI@Z
// SYNTHETIC: VC1 0x0041aa10
// ?Release@KdDisplayBase@vc1@@WBE@AEIXZ
// SYNTHETIC: VC1 0x0041aa20
// ?QueryInterface@KdDisplayBase@vc1@@WBE@AEPAXPAPBD@Z

typedef char Vc1KdDisplayBaseSizeCheck[sizeof(KdDisplayBase) == 0x30 ? 1 : -1];

// SIZE 0x5c
// VTABLE: VC1 0x004d0b78 vc1::IScriptObject
// VTABLE: VC1 0x004d0b94 vc1::IRenderTarget
// VTABLE: VC1 0x004d0bb4 vc1::IPyroDevice
// VTABLE: VC1 0x004d0be8 vc1::IDisplay
// VTABLE: VC1 0x004d0c28 vc1::IShutdownable
// VTABLE: VC1 0x004d0c38 vc1::IAbstract
class KdDisplay : public KdDisplayBase
{
public:
    IUnknown* target;        // 0x30
    Matrix3 matrix;          // 0x34
    unsigned char enabled58; // 0x58

    KdDisplay() throw();
    ~KdDisplay();
    void* QueryInterface(const char** iid); // vtable+0x00
    unsigned AddRef();                      // vtable+0x04
    unsigned Release();                     // vtable+0x08
    unsigned char GetMember(const char* name, SquirrelObject* out); // vtable+0x0c
    unsigned char SetMember(void* a, void* b); // vtable+0x10
    void* BeginFrame(void* a, void* b);      // IRenderTarget vtable+0x0c
    void* EndFrame();                       // IRenderTarget vtable+0x10
    unsigned char SetSize(unsigned width, unsigned height); // IDisplay vtable+0x20
    int SetWindowed(bool arg);              // IDisplay vtable+0x28
    int Shutdown() throw();                 // IShutdownable vtable+0x0c
    int FitWindow();                        // IScriptObject vtable+0x18
};

typedef char Vc1KdDisplaySizeCheck[sizeof(KdDisplay) == 0x5c ? 1 : -1];

// SYNTHETIC: VC1 0x00402230
// vc1::KdDisplay::`scalar deleting destructor'
// SYNTHETIC: VC1 0x004022e0
// ?AddRef@KdDisplay@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x004022f0
// ?BeginFrame@KdDisplay@vc1@@WBA@AEPAXPAX0@Z
// SYNTHETIC: VC1 0x00402300
// ??_EKdDisplay@vc1@@WBA@AEPAXI@Z
// SYNTHETIC: VC1 0x00402310
// ?Release@KdDisplay@vc1@@WBE@AEIXZ
// SYNTHETIC: VC1 0x00402320
// ?QueryInterface@KdDisplay@vc1@@WBE@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00402340
// ?Release@KdDisplay@vc1@@WBI@AEIXZ
// SYNTHETIC: VC1 0x00402350
// ?QueryInterface@KdDisplay@vc1@@WBI@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00402370
// ?Release@KdDisplay@vc1@@WBM@AEIXZ
// SYNTHETIC: VC1 0x00402380
// ?QueryInterface@KdDisplay@vc1@@WBM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00402390
// ?EndFrame@KdDisplay@vc1@@WBA@AEPAXXZ
// SYNTHETIC: VC1 0x004023a0
// ?AddRef@KdDisplay@vc1@@WBE@AEIXZ
// SYNTHETIC: VC1 0x004023c0
// ??_EKdDisplay@vc1@@WBM@AEPAXI@Z
// SYNTHETIC: VC1 0x004023d0
// ?AddRef@KdDisplay@vc1@@WBI@AEIXZ
// SYNTHETIC: VC1 0x004023e0
// ?Release@KdDisplay@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x004023f0
// ?QueryInterface@KdDisplay@vc1@@W3AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00402400
// ?AddRef@KdDisplay@vc1@@WBM@AEIXZ

void** CreateKdDisplay(void** out);

}

#endif
