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
    unsigned refs;           // 0x20
    unsigned pad24;          // 0x24
    unsigned pad28;          // 0x28
    void* device;            // 0x2c
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
    int Script14();                            // vtable+0x14
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
};

typedef char Vc1KdDisplaySizeCheck[sizeof(KdDisplay) == 0x5c ? 1 : -1];

// FUNCTION: VC1 0x00402230
// vc1::KdDisplay::`scalar deleting destructor'

KdDisplay* ConstructKdDisplay(KdDisplay* self);
void** CreateKdDisplay(void** out);
void* DisplayFaceQueryInterface(IDisplay* self, const char** iid);
void* DisplayBaseQueryInterface(KdDisplay* self, const char** iid);

}

#endif
