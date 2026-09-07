#ifndef VC1_KD_DISPLAY_H
#define VC1_KD_DISPLAY_H

#include "kd/iface/iscript_object.h"
#include "kd/iface/irender_target.h"
#include "kd/iface/idisplay.h"
#include "kd/iface/ishutdownable.h"
#include "kd/iface/iabstract.h"

namespace vc1 {

class KdDisplay;

extern IAbstract* g_kdDisplay;

// Extra original vftables: 0x004d0b94, 0x004d0be8, 0x004d0c28, 0x004d0c38.
// VTABLE: VC1 0x004d0b78
// SIZE 0x5c
class KdDisplay : public IScriptObject,
                  public IRenderTarget,
                  public IDisplay,
                  public IShutdownable,
                  public IAbstract
{
public:
    unsigned pad20[4];       // 0x20
    float zero30;            // 0x30
    float one34;             // 0x34
    float zero38;            // 0x38
    float zero3c;            // 0x3c
    float zero40;            // 0x40
    float one44;             // 0x44
    float zero48;            // 0x48
    float zero4c;            // 0x4c
    float zero50;            // 0x50
    float one54;             // 0x54
    unsigned char enabled58; // 0x58

    KdDisplay();
    void* QueryInterface(const char** iid); // vtable+0x00
    unsigned AddRef();                      // vtable+0x04
    unsigned Release();                     // vtable+0x08
    int Display0c();                        // vtable+0x0c
    int Display10();                        // vtable+0x10
    int Display14();                        // vtable+0x14
    int Display18();                        // vtable+0x18
    int Display1c();                        // vtable+0x1c
    int SetSize(unsigned width, unsigned height); // vtable+0x20
    int Display24();                        // vtable+0x24
    int Display28();                        // vtable+0x28
    void* GetMixer();                       // vtable+0x2c
    int Display30();                        // vtable+0x30
    int Display34();                        // vtable+0x34
    int Display38();                        // vtable+0x38
    int Shutdown();                         // vtable+0x0c
    int Unmapped0c();                       // vtable+0x0c
};

typedef char Vc1KdDisplaySizeCheck[sizeof(KdDisplay) == 0x5c ? 1 : -1];

KdDisplay* ConstructKdDisplay(KdDisplay* self);
void** CreateKdDisplay(void** out);

}

#endif
