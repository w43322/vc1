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

void* KdQueryInterface(IUnknown* object, const char** iid);
void KdAddRef(IUnknown* object);
void KdRelease(IUnknown* object);

// Inferred name. Owning interface pointer member: the original destructors
// inline "if (p) p->Release(); p = 0;" per member in reverse declaration order.
// Assignment does not touch the reference count.
template <class T>
class KdRef
{
public:
    T* ptr;

    KdRef() : ptr(0) {}
    ~KdRef()
    {
        if (ptr) {
            ptr->Release();
        }
        ptr = 0;
    }
    KdRef& operator=(T* value)
    {
        ptr = value;
        return *this;
    }
    operator T*() const { return ptr; }
    T* operator->() const { return ptr; }

private:
    KdRef(const KdRef&);
    KdRef& operator=(const KdRef&);
};

}

#endif
