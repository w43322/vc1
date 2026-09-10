#include "kd/scene/compound_object.h"

#include "kd/iface/iunknown.h"
#include "kd/iface/iid.h"
#include "kd/factory.h"
#include "host/runtime.h"

#include <new>
#include <stdlib.h>
#include <string.h>

namespace vc1 {

// STRING: VC1 0x004d4b90
static const char kCompoundObject[] = "CompoundObject";
// GLOBAL: VC1 0x004d0674
const char* g_kdNameCompoundObject = kCompoundObject;

IdentifiableName::IdentifiableName()
    : begin(&g_kdLogTag), size(0), capacity(0)
{
    AssignCountedString(this, g_emptyCountedBegin, g_emptyCountedSize);
}

void* CompoundComponentGroup::QueryInterface(const char** iid)
{
    if (*iid == g_iidComponentGroup || *iid == g_iidShutdownable) {
        return this;
    }
    return 0;
}

unsigned CompoundComponentGroup::AddRef()
{
    return 0;
}

unsigned CompoundComponentGroup::Release()
{
    return 0;
}

int CompoundComponentGroup::Shutdown()
{
    return Unmapped24();
}

int CompoundComponentGroup::Unmapped10(void* key, void** out)
{
    (void)key;
    *out = 0;
    return 0;
}

void** CompoundComponentGroup::Unmapped14(void** out, void* key)
{
    (void)key;
    *out = 0;
    return out;
}

void** CompoundComponentGroup::Unmapped18(void** out, void* key)
{
    (void)key;
    *out = 0;
    return out;
}

int CompoundComponentGroup::Unmapped1c(void* key)
{
    (void)key;
    return 0;
}

void* CompoundComponentGroup::Unmapped20()
{
    return &extra;
}

int CompoundComponentGroup::Unmapped24()
{
    return 0;
}

void* CompoundTailValue::QueryInterface(const char** iid)
{
    if (*iid == g_iidCloneable) {
        return static_cast<ICloneable*>(this);
    }
    return 0;
}

unsigned CompoundTailValue::AddRef()
{
    return 0;
}

unsigned CompoundTailValue::Release()
{
    return 0;
}

int CompoundTailValue::Tail0c(void* arg)
{
    (void)arg;
    return 0;
}

int CompoundTailValue::Tail10(void* arg)
{
    (void)arg;
    return 0;
}

int CompoundTailValue::Tail14()
{
    return 0;
}

void** CompoundTailValue::Clone(void** out)
{
    *out = 0;
    return out;
}

// FUNCTION: VC1 0x00472f90
unsigned KdCompoundObject::Release()
{
    return IAbstract::Release();
}

// FUNCTION: VC1 0x00472fb0
unsigned KdCompoundObject::AddRef()
{
    return IAbstract::AddRef();
}

// FUNCTION: VC1 0x00473800
KdCompoundObject::KdCompoundObject() throw()
{
    scriptValue.obj._unVal.pUserPointer = 0;
    scriptValue.obj._type = OT_NULL;
    tail6c = 0;
}

unsigned char KdCompoundObject::Script0c(void* a, void* b)
{
    (void)a;
    (void)b;
    return 0;
}

unsigned char KdCompoundObject::Script10(void* a, void* b)
{
    (void)a;
    (void)b;
    return 0;
}

const char* KdCompoundObject::Script14()
{
    return kCompoundObject;
}

void** KdCompoundObject::Clone(void** out)
{
    *out = 0;
    return out;
}

void* KdCompoundObject::Position0c(void* out)
{
    return out;
}

void KdCompoundObject::Position10(const void* valueArg)
{
    (void)valueArg;
}

const char* KdCompoundObject::GetName()
{
    return static_cast<const char*>(name.begin);
}

void KdCompoundObject::SetName(const char* valueArg)
{
    (void)valueArg;
}

int KdCompoundObject::Classify0c(void* arg)
{
    (void)arg;
    return 0;
}

int KdCompoundObject::Classify10(void* arg)
{
    (void)arg;
    return 0;
}

int KdCompoundObject::Classify14(void* arg)
{
    (void)arg;
    return 0;
}

int KdCompoundObject::Classify18()
{
    return 0;
}

int KdCompoundObject::Shutdown()
{
    return 0;
}

int KdCompoundObject::Unmapped10(void* key, void** out)
{
    return CompoundComponentGroup::Unmapped10(key, out);
}

void** KdCompoundObject::Unmapped14(void** out, void* key)
{
    return CompoundComponentGroup::Unmapped14(out, key);
}

void** KdCompoundObject::Unmapped18(void** out, void* key)
{
    return CompoundComponentGroup::Unmapped18(out, key);
}

// FUNCTION: VC1 0x004738e0
void** CreateKdCompoundObject(void** out)
{
    KdCompoundObject* memory = static_cast<KdCompoundObject*>(malloc(sizeof(KdCompoundObject)));
    IUnknown* queried;

    if (!memory) {
        *out = 0;
        return out;
    }
    new (memory) KdCompoundObject();
    queried = static_cast<IUnknown*>(
        static_cast<IAbstract*>(memory)->QueryInterface(&g_iidAbstract));
    *out = queried;
    if (queried) {
        queried->AddRef();
    }
    KdRelease(static_cast<IUnknown*>(*out));
    return out;
}

}
