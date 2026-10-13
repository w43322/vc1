#ifndef VC1_KD_DISPLAY_H
#define VC1_KD_DISPLAY_H

#include "kd/iface/iscript_object.h"
#include "kd/iface/irender_target.h"
#include "kd/iface/ipyro_device.h"
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

void DisplayD3dSetView(const Matrix3* matrix);
void DisplayD3dSetClip(int x, int y, int width, int height);

// Inferred name. Implements the Pyro device plus the device-level IDisplay
// slots (SetSize builds g_displayPyro, Display3c) and Shutdown. Constructed
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
    unsigned value14; // 0x14  Pyro20 stores; Shutdown clears
    unsigned mode18;  // 0x18  Pyro04: 0 for (4, 1), else 2

    KdDisplayDevice();
    ~KdDisplayDevice();

    void* QueryInterface(const char** iid); // vtable+0x00
    unsigned AddRef();                      // vtable+0x04
    unsigned Release();                     // vtable+0x08

    int Pyro00(void* a, void* b);
    int Pyro04(unsigned mode, unsigned value);
    int Pyro08(void** out, void* b, void* c, void* d, void* e);
    int Pyro0c(void** out, void* b, void* c, void* d, void* e);
    int Pyro10(void* a, void* b, void* c, void* d);
    int Pyro14(void* a, void* b, void* c);
    int Pyro18(void** out, void* b, void* c, void* d, void* e);
    int Pyro1c(void* a);
    int Pyro20(void* a, unsigned value);
    int Pyro24(void* a, void* b, void* c, void* d);
    int Pyro28(void* a, void* b, void* c, void* d);
    int Pyro2c();

    unsigned char SetSize(unsigned width, unsigned height); // IDisplay vtable+0x20
    void** Display3c(void** out);                           // IDisplay vtable+0x3c
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
// (mixer). Script14 "Display". Constructor 0x004665c0 (ESI=this) is called by
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
    unsigned char Script0c(void* a, void* b);
    unsigned char Script10(void* a, void* b);
    const char* Script14();
    void* Display0c(void* a, void* b);
    void* Display10();
    int GetWidth();
    int GetHeight();
    void* Display1c();
    unsigned char SetSize(unsigned width, unsigned height);
    unsigned char IsWindowed();
    int SetWindowed(unsigned char arg);
    void* GetMixer();
    void Display30(int unused);
    void** CreateTexture(void** out);
    void** CreateRenderTarget(void** out, unsigned width, unsigned height);
    int Shutdown() throw();
    virtual int FitWindow(); // IScriptObject vtable+0x18
};

// SYNTHETIC: VC1 0x00402330
// ?GetWidth@KdDisplayBase@vc1@@WBA@AEHXZ
// SYNTHETIC: VC1 0x00402360
// ?GetHeight@KdDisplayBase@vc1@@WBA@AEHXZ
// SYNTHETIC: VC1 0x004023b0
// ?Display1c@KdDisplayBase@vc1@@WBA@AEPAXXZ
// SYNTHETIC: VC1 0x0041a660
// vc1::KdDisplayBase::`scalar deleting destructor'
// SYNTHETIC: VC1 0x0041a930
// ?Display0c@KdDisplayBase@vc1@@WBA@AEPAXPAX0@Z
// SYNTHETIC: VC1 0x0041a940
// ?Release@KdDisplayBase@vc1@@WBI@AEIXZ
// SYNTHETIC: VC1 0x0041a950
// ?QueryInterface@KdDisplayBase@vc1@@WBI@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0041a960
// ?Release@KdDisplayBase@vc1@@WBM@AEIXZ
// SYNTHETIC: VC1 0x0041a970
// ?QueryInterface@KdDisplayBase@vc1@@WBM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0041a980
// ?Display10@KdDisplayBase@vc1@@WBA@AEPAXXZ
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
    unsigned char Script0c(void* a, void* b); // vtable+0x0c
    unsigned char Script10(void* a, void* b); // vtable+0x10
    void* Display0c(void* a, void* b);      // IRenderTarget vtable+0x0c
    void* Display10();                       // IRenderTarget vtable+0x10
    unsigned char SetSize(unsigned width, unsigned height); // IDisplay vtable+0x20
    int SetWindowed(unsigned char arg);     // IDisplay vtable+0x28
    int Shutdown();                         // IShutdownable vtable+0x0c
    int FitWindow();                        // IScriptObject vtable+0x18
};

typedef char Vc1KdDisplaySizeCheck[sizeof(KdDisplay) == 0x5c ? 1 : -1];

// SYNTHETIC: VC1 0x00402230
// vc1::KdDisplay::`scalar deleting destructor'
// SYNTHETIC: VC1 0x004022e0
// ?AddRef@KdDisplay@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x004022f0
// ?Display0c@KdDisplay@vc1@@WBA@AEPAXPAX0@Z
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
// ?Display10@KdDisplay@vc1@@WBA@AEPAXXZ
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
