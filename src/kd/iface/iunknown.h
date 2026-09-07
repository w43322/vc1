#ifndef VC1_KD_IUNKNOWN_H
#define VC1_KD_IUNKNOWN_H

namespace vc1 {

// SIZE 0x04
struct IUnknown
{
    virtual void* QueryInterface(const char** iid) = 0; // vtable+0x00
    virtual unsigned AddRef() = 0;                      // vtable+0x04
    virtual unsigned Release() = 0;                     // vtable+0x08
};

typedef char Vc1IUnknownSizeCheck[sizeof(IUnknown) == 0x04 ? 1 : -1];

// SIZE 0x04
struct Unknown : IUnknown
{
    void* QueryInterface(const char** iid); // vtable+0x00
    unsigned AddRef();                      // vtable+0x04
    unsigned Release();                     // vtable+0x08
};

typedef char Vc1UnknownSizeCheck[sizeof(Unknown) == 0x04 ? 1 : -1];

void* KdQueryInterface(IUnknown* object, const char** iid);
void KdAddRef(IUnknown* object);
void KdRelease(IUnknown* object);

}

#endif
