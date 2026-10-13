#include "kd/factory.h"
#include "kd/iface/iid.h"
#include "kd/iface/ifont.h"

namespace vc1 {

class KdFont : public IUnknown
{
};

// ESI=self, RET 4. Releases the old font, then holds source QI'd to IFont.
// FUNCTION: VC1 0x00418090
KdRef<IFont>* AssignFont(KdRef<IFont>* self, IUnknown* const* source)
{
    IFont* queried;

    if (self->ptr) {
        self->ptr->Release();
    }
    if (!*source) {
        self->ptr = 0;
        return self;
    }
    queried = static_cast<IFont*>((*source)->QueryInterface(&g_iidFont));
    self->ptr = queried;
    if (queried) {
        queried->AddRef();
    }
    return self;
}

// STRING: VC1 0x004d4bd4
static const char kFont[] = "Font";
// GLOBAL: VC1 0x004d0660
const char* g_kdNameFont = kFont;

// STUB: VC1 0x00466f50
void** CreateKdFont(void** out)
{
    (void)sizeof(KdFont*);
    if (out) {
        *out = 0;
    }
    return out;
}

}
