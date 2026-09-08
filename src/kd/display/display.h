#ifndef VC1_KD_DISPLAY_H
#define VC1_KD_DISPLAY_H

#include "kd/iface/iscript_object.h"
#include "kd/iface/irender_target.h"
#include "kd/iface/idisplay.h"
#include "kd/iface/ishutdownable.h"
#include "kd/iface/iabstract.h"
#include "kd/display/render_target.h"
#include "kd/display/texture.h"

namespace vc1 {

class KdDisplay;

extern IAbstract* g_kdDisplay;

// VTABLE: VC1 0x004d0b78 vc1::IScriptObject
// VTABLE: VC1 0x004d0b94 vc1::IRenderTarget
// VTABLE: VC1 0x004d0be8 vc1::IDisplay
// VTABLE: VC1 0x004d0c28 vc1::IShutdownable
// VTABLE: VC1 0x004d0c38 vc1::IAbstract
// SIZE 0x5c
class KdDisplay : public IScriptObject,
                  public IRenderTarget,
                  public IDisplay,
                  public IShutdownable,
                  public IAbstract
{
public:
    unsigned refs;           // 0x20
    unsigned pad24;          // 0x24
    unsigned pad28;          // 0x28
    void* mixer;             // 0x2c  IDisplay+0x18
    IUnknown* target;        // 0x30
    float matrix[9];         // 0x34
    unsigned char enabled58; // 0x58

    KdDisplay();
    ~KdDisplay();
    void* QueryInterface(const char** iid); // vtable+0x00
    unsigned AddRef();                      // vtable+0x04
    unsigned Release();                     // vtable+0x08
    unsigned char Script0c(void* a, void* b); // vtable+0x0c
    unsigned char Script10(void* a, void* b); // vtable+0x10
    const char* Script14();                 // vtable+0x14
    void* Display0c(void* a, void* b);      // vtable+0x0c
    void* Display10();                       // vtable+0x10
    int GetWidth();                         // vtable+0x14
    int GetHeight();                        // vtable+0x18
    void* Display1c();                      // vtable+0x1c
    int SetSize(unsigned width, unsigned height); // vtable+0x20
    unsigned char IsWindowed();            // vtable+0x24
    void Display30(int unused);              // vtable+0x30
    void** CreateTexture(void** out);       // vtable+0x34
    void** CreateRenderTarget(void** out, unsigned width, unsigned height); // vtable+0x38
    int Shutdown();                         // vtable+0x0c
};

typedef char Vc1KdDisplaySizeCheck[sizeof(KdDisplay) == 0x5c ? 1 : -1];

// SYNTHETIC: VC1 0x00402230
// vc1::KdDisplay::`scalar deleting destructor'
// SYNTHETIC: VC1 0x004022e0
// ?AddRef@KdDisplay@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x004022f0
// ?Display0c@KdDisplay@vc1@@WBA@AEPAXPAX0@Z
// SYNTHETIC: VC1 0x00402300
// ??_EKdDisplay@vc1@@W3AEPAXI@Z
// SYNTHETIC: VC1 0x00402310
// ?Release@KdDisplay@vc1@@WBE@AEIXZ
// SYNTHETIC: VC1 0x00402320
// ?QueryInterface@KdDisplay@vc1@@WBE@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00402330
// ?GetWidth@KdDisplay@vc1@@WBA@AEHXZ
// SYNTHETIC: VC1 0x00402340
// ?Release@KdDisplay@vc1@@WBI@AEIXZ
// SYNTHETIC: VC1 0x00402350
// ?QueryInterface@KdDisplay@vc1@@WBI@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00402360
// ?GetHeight@KdDisplay@vc1@@WBA@AEHXZ
// SYNTHETIC: VC1 0x00402370
// ?Release@KdDisplay@vc1@@WBM@AEIXZ
// SYNTHETIC: VC1 0x00402380
// ?QueryInterface@KdDisplay@vc1@@WBM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00402390
// ?Display10@KdDisplay@vc1@@WBA@AEPAXXZ
// SYNTHETIC: VC1 0x004023a0
// ?AddRef@KdDisplay@vc1@@WBE@AEIXZ
// SYNTHETIC: VC1 0x004023b0
// ?Display1c@KdDisplay@vc1@@WBA@AEPAXXZ
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

KdDisplay* ConstructKdDisplay(KdDisplay* self);
void** CreateKdDisplay(void** out);
void* DisplayFaceQueryInterface(IDisplay* self, const char** iid);
void* DisplayBaseQueryInterface(KdDisplay* self, const char** iid);

}

#endif
