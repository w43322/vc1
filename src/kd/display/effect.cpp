#include "kd/display/effect.h"
#include "kd/display/pyro.h"
#include "kd/stream/file_reader.h"
#include "pyro/pyro_emitter.h"
#include "host/runtime.h"

#include "bindings/component.h"
#include "bindings/sqplus.h"

#include <errno.h>
#include <stdlib.h>

namespace SqPlus {
// A created emitter goes out as a counted IUnknown.
inline void Push(HSQUIRRELVM v, vc1::KdPtr<vc1::IEffectEmitter>& value)
{
    vc1::KdPtr<vc1::IUnknown> object(value.ptr);
    Push(v, object);
}
}

namespace vc1 {

extern PyroLibrary* g_displayPyro; // display.cpp
extern const char kReadBinary[];   // display.cpp

// STRING: VC1 0x004d4c18
static const char kEffect[] = "Effect";
// GLOBAL: VC1 0x004d0644
const char* g_kdNameEffect = kEffect;

// What CreateEmitter hands out for an unknown emitter name. Only read
// (and released by its atexit destructor 0x004cb150), so always null.
// GLOBAL: VC1 0x004ed708
KdPtr<IAbstract> g_effectNoEmitter;

// RET 8. Null handle: errno 0x11, 0. Otherwise driver write (+0x124).
// FUNCTION: VC1 0x0040eda0 FOLDED
int KdPyroStream::Write(const void* buffer, int size)
{
    if (!file) {
        errno = 0x11;
        return 0;
    }
    return file->driver->write(file->driver, buffer, 1, size, file->native);
}

// MOV EAX, "Effect"; RET.
// FUNCTION: VC1 0x0041d630
const char* KdEffect::Script14()
{
    return kEffect;
}

// RET 4. this = IEffect. The script "CreateEmitter" native: vtable+0x0c
// with a null name.
// FUNCTION: VC1 0x0041d640
KdPtr<IEffectEmitter> KdEffect::CreateFirstEmitter()
{
    return CreateEmitter(0);
}

// RET 8. Null handle: errno 0x11, 0. Otherwise driver read (+0x120).
// FUNCTION: VC1 0x0041d680 FOLDED
int KdPyroStream::Read(void* buffer, int size)
{
    if (!file) {
        errno = 0x11;
        return 0;
    }
    return file->driver->read(file->driver, buffer, 1, size, file->native);
}

// RET 8. Null handle: errno 0x11, -1. Otherwise the driver's 64-bit seek
// (+0x140) with the offset sign-extended.
// FUNCTION: VC1 0x0041d6c0
int KdPyroStream::Seek(int offset, int origin)
{
    if (!file) {
        errno = 0x11;
        return -1;
    }
    return file->driver->seek(file->driver, file->native, offset, origin);
}

// Null handle: errno 0x11, -1. Otherwise the low half of the driver's tell
// (+0x144).
// FUNCTION: VC1 0x0041d710 FOLDED
int KdPyroStream::Tell()
{
    if (!file) {
        errno = 0x11;
        return -1;
    }
    return static_cast<int>(file->driver->tell(file->driver, file->native));
}

// EDI = result. The emitter wrapper is not recovered yet.
// STUB: VC1 0x0045b590
KdPtr<IAbstract> CreateEffectEmitter(IPyroEmitter* emitter, IEffect* effect)
{
    (void)emitter;
    (void)effect;
    return KdPtr<IAbstract>();
}

// XOR AL,AL; RET 8.
// FUNCTION: VC1 0x0045b5f0
unsigned char KdEffect::Script10(void* a, void* b)
{
    (void)a;
    (void)b;
    return 0;
}

inline void KdEffect::FreeFile()
{
    if (file) {
        file->DestroyTextures();
        delete file;
        file = 0;
    }
}

// ESI = this.
// FUNCTION: VC1 0x0045b600
KdEffect::~KdEffect()
{
    FreeFile();
}

// ADD ECX, 0x10; deleting destructor (IAbstract vtable+0x0c) on zero refs.
// FUNCTION: VC1 0x0045b6b0
unsigned KdEffect::Release()
{
    IUnknown* face;
    unsigned result;
    typedef void (__thiscall *Fn0c)(void* object, int arg);

    face = static_cast<IUnknown*>(static_cast<IAbstract*>(this));
    result = --refs;
    if (result == 0) {
        reinterpret_cast<Fn0c>((*reinterpret_cast<void***>(face))[3])(face, 1);
        return 0;
    }
    return result;
}

// refs at +0x14; only counts up from nonzero.
// FUNCTION: VC1 0x0045b6d0
unsigned KdEffect::AddRef()
{
    unsigned value = refs;
    if (value != 0) {
        ++value;
        refs = value;
    }
    return value;
}

// EAX = this.
// FUNCTION: VC1 0x0045b6e0
KdEffect::KdEffect()
    : bounds(0.0f, 0.0f, 1.0f, 1.0f)
{
    file = 0;
}

// this = ILoadable, RET 4. Drops the previous file, then reads the .pyro
// through the display's Pyro library and creates its textures.
// FUNCTION: VC1 0x0045b790
unsigned char KdEffect::Load(const char* name)
{
    KdFileHandle* handle;

    FreeFile();
    handle = static_cast<KdFileHandle*>(OpenKdFile(name, kReadBinary));
    if (!handle) {
        return 0;
    }
    KdPyroStream stream(handle);
    file = g_displayPyro->LoadFile(&stream);
    if (!file) {
        handle->driver->close(handle->driver, handle->native);
        free(handle);
        return 0;
    }
    handle->driver->close(handle->driver, handle->native);
    free(handle);
    file->CreateTextures();
    // STRING: VC1 0x004d6924
    HostLog(&g_kdNameEffect, "Loaded %s (emitters: %i)", name, file->GetEmitterCount());
    return 1;
}

// this = IScriptLoadable, RET 4. The effect's PyroEffects.nut table.
// FUNCTION: VC1 0x0045b860
bool KdEffect::LoadFromScript(SquirrelObject& table)
{
    if (table.GetObjectHandle()._type != OT_TABLE) {
        return false;
    }
    // STRING: VC1 0x004d6940
    bounds = GetScriptValue<RectF>(table.GetValue("BoundingRect"));
    return true;
}

// RET 8 (hidden result, name).
// FUNCTION: VC1 0x0045b990
KdPtr<IEffectEmitter> KdEffect::CreateEmitter(const char* name)
{
    PyroEmitter* emitter;

    if (!name) {
        name = file->GetEmitter(0)->GetName();
    }
    emitter = file->CreateEmitter(name);
    if (!emitter) {
        return KdPtr<IEffectEmitter>(g_effectNoEmitter);
    }
    return KdPtr<IEffectEmitter>(CreateEffectEmitter(emitter, this));
}

// RET 8. Method table 0x004ee574 (guard bit 0x004ee584, atexit destructor
// 0x004cb0f0).
// FUNCTION: VC1 0x0045ba60
unsigned char KdEffect::Script0c(const char* name, SquirrelObject* out)
{
    static ScriptMethodTable methods;

    if (methods.Empty()) {
        // STRING: VC1 0x004d6914
        methods["CreateEmitter"] = CreateScriptMethod<KdEffect>(&KdEffect::CreateFirstEmitter);
    }
    return methods.Find(name, out);
}

}
