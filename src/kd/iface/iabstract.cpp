#include "kd/iface/iabstract.h"
#include "kd/iface/iid.h"

namespace vc1 {

// Constructor writes vftable 0x004d1034 and refs=1; it is inlined into users.
IAbstract::IAbstract()
{
    refs = 1;
}

// ECX=face, ESI=result. Kept out of line: the original calls it from
// TerrainBuilder SetMode/OnPointer, BuildingBuilder SetEnabled/UpdateCursor
// and PassMap SetCellValue.
// FUNCTION: VC1 0x00401e90
__declspec(noinline) KdPtr<IUnknown> SelfObject(IAbstract* face)
{
    IUnknown* object = static_cast<IUnknown*>(face->QueryInterface(&g_iidAbstract));
    return KdPtr<IUnknown>(object
        ? static_cast<IUnknown*>(object->QueryInterface(&g_iidAbstract))
        : 0);
}

// RET 4. Returns this only for the canonical IAbstract IID pointer.
// FUNCTION: VC1 0x00401ec0
void* IAbstract::QueryInterface(const char** iid)
{
    return *iid == g_iidAbstract ? this : 0;
}

// Decrements refs; deleting destructor vtable+0x0c(this,1) on zero.
// FUNCTION: VC1 0x00401ee0
unsigned IAbstract::Release()
{
    unsigned value = --refs;
    if (value == 0) {
        delete this;
        return 0;
    }
    return value;
}

// Increment only while refs is nonzero.
// FUNCTION: VC1 0x00401f00
unsigned IAbstract::AddRef()
{
    unsigned value = refs;
    if (value != 0) {
        value += 1;
        refs = value;
    }
    return value;
}

IAbstract::~IAbstract()
{
}

}
