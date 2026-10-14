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

// Inferred name. Interface IID lookup for KdPtr conversions; specialised next
// to each interface with a static Get() returning its g_iid* slot.
template <class T>
struct KdIid;

// Inferred name. Copyable counted interface pointer, the by-value form of
// KdRef: construction from a raw pointer and copies AddRef. Script bindings
// pass it through SqPlus Push/Get (0x0040c970, 0x0041b830).
template <class T>
class KdPtr
{
public:
    T* ptr;

    KdPtr() : ptr(0) {}
    explicit KdPtr(T* value) : ptr(value)
    {
        if (ptr) {
            ptr->AddRef();
        }
    }
    KdPtr(const KdPtr& source) : ptr(source.ptr)
    {
        if (ptr) {
            ptr->AddRef();
        }
    }
    // Counted T face of another interface, QI'd through KdIid<T>.
    template<class U>
    KdPtr(const KdPtr<U>& source)
        : ptr(source.ptr ? static_cast<T*>(source.ptr->QueryInterface(KdIid<T>::Get())) : 0)
    {
        if (ptr) {
            ptr->AddRef();
        }
    }
    ~KdPtr()
    {
        if (ptr) {
            ptr->Release();
        }
        ptr = 0;
    }
    T* operator->() const { return ptr; }

private:
    KdPtr& operator=(const KdPtr&);
};

}

#endif
