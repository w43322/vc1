#include "kd/display/effect.h"
#include "kd/display/pyro.h"
#include "kd/stream/file_reader.h"
#include "pyro/pyro_emitter.h"
#include "host/runtime.h"

#include "bindings/component.h"
#include "bindings/sqplus.h"

#include "kd/iface/igraphics.h"
#include "kd/iface/iid.h"
#include "bindings/rect.h"

#include <errno.h>
#include <new>
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

extern PyroLibrary* g_displayPyro;      // display.cpp
extern IUnknown* g_graphics;            // display.cpp
extern const char kReadBinary[];        // display.cpp
extern const char kScriptSetEnabled[];  // object_watcher.cpp
bool IsRectVisible(const KdPtr<IGraphics>& graphics, const Rect& rect); // tile_manager.cpp

// STRING: VC1 0x004d4c18
static const char kEffect[] = "Effect";
// GLOBAL: VC1 0x004d0644
const char* g_kdNameEffect = kEffect;

// What CreateEmitter hands out for an unknown emitter name. Only read
// (and released by its atexit destructor 0x004cb150), so always null.
// GLOBAL: VC1 0x004ed708
KdPtr<IAbstract> g_effectNoEmitter;

// What an emitter falls back to for its graphics after drawing and for its
// positionable on Shutdown. Like g_effectNoEmitter only read (atexit
// destructor 0x004cb4b0), so always null.
// GLOBAL: VC1 0x004ed710
KdPtr<IUnknown> g_effectNoTarget;

// Pyro's SetMatrix input in local mode: a 3x4 row-major transform whose
// rotation and translation entries Update overwrites.
// GLOBAL: VC1 0x004e8938
float g_effectLocalTransform[3][4] = {
    { 1.0f, 0.0f, 0.0f, 0.0f },
    { 0.0f, 1.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 1.0f, 0.0f },
};

// STRING: VC1 0x004d4c08
static const char kEffectEmitter[] = "EffectEmitter";

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

// Inferred name. ESI = slot, RET 4. slot = source's IGraphics face, counted;
// only caller KdEffectEmitter::Render.
// FUNCTION: VC1 0x0041a680
__declspec(noinline) IUnknown** AssignGraphics(IUnknown** slot, const KdPtr<IUnknown>& source)
{
    IUnknown* queried;

    if (*slot) {
        (*slot)->Release();
    }
    if (!source.ptr) {
        *slot = 0;
        return slot;
    }
    queried = static_cast<IUnknown*>(source.ptr->QueryInterface(&g_iidGraphics));
    *slot = queried;
    if (queried) {
        queried->AddRef();
    }
    return slot;
}

// MOV EAX, "Effect"; RET.
// FUNCTION: VC1 0x0041d630
const char* KdEffect::GetTypeName()
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

// MOV EAX, "EffectEmitter"; RET.
// FUNCTION: VC1 0x0042dfa0
const char* KdEffectEmitter::GetTypeName()
{
    return kEffectEmitter;
}

// Inferred name. ESI = graphics; returns &g_graphics. Pyro draws through
// g_graphics (KdDisplayDevice); only caller KdEffectEmitter::Render.
// FUNCTION: VC1 0x0042dfd0
__declspec(noinline) IUnknown** SetGraphics(const KdPtr<IGraphics>& graphics)
{
    if (graphics.ptr) {
        graphics.ptr->AddRef();
    }
    if (g_graphics) {
        g_graphics->Release();
    }
    g_graphics = graphics.ptr;
    return &g_graphics;
}

// RET 4. Nine dwords into localMatrix.
// FUNCTION: VC1 0x0045a820
void KdEffectEmitter::SetLocalMatrix(const Matrix3& matrix)
{
    localMatrix = matrix;
}

// FUNCTION: VC1 0x0045a840
void KdEffectEmitter::SetLocalMode(bool local)
{
    localMode = local;
}

// FUNCTION: VC1 0x0045a850
void KdEffectEmitter::SetEnabled(bool value)
{
    enabled = value;
}

// FUNCTION: VC1 0x0045a860
void KdEffectEmitter::SetRenderLayer(int layer)
{
    renderLayer = layer;
}

// Restarts the emitter and the lifetime.
// FUNCTION: VC1 0x0045a870
void KdEffectEmitter::Reset()
{
    emitter->Reset();
    lifeTime = 0;
    enabled = true;
    started = false;
}

// this = IVisible.
// FUNCTION: VC1 0x0045a890
bool KdEffectEmitter::IsVisible()
{
    return visible;
}

// FUNCTION: VC1 0x0045a8a0
void KdEffectEmitter::SetVisible(bool value)
{
    visible = value;
}

// The render layer.
// FUNCTION: VC1 0x0045a8b0
int KdEffectEmitter::GetRenderOrder()
{
    return renderLayer;
}

// RET 4. this = IWithLifeTime; the layer is ignored.
// FUNCTION: VC1 0x0045a8c0
int KdEffectEmitter::SetLayer(unsigned layer)
{
    (void)layer;
    return 0;
}

// MOV EAX, 0x40; RET.
// FUNCTION: VC1 0x0045a8d0
int KdEffectEmitter::GetUpdateLayer()
{
    return 0x40;
}

// Expired once it has emitted and every particle is gone.
// FUNCTION: VC1 0x0045a8e0
bool KdEffectEmitter::IsExpired()
{
    return started && !emitter->HasParticles();
}

// FUNCTION: VC1 0x0045a900
unsigned KdEffectEmitter::GetLifeTime()
{
    return lifeTime;
}

// XOR AL,AL; RET 8.
// FUNCTION: VC1 0x0045a910
unsigned char KdEffectEmitter::SetMember(void* a, void* b)
{
    (void)a;
    (void)b;
    return 0;
}

// this = IPositionable. The followed positionable's, else its own.
// FUNCTION: VC1 0x0045a920
const Matrix3& KdEffectEmitter::GetPosition()
{
    if (positionable) {
        return positionable->GetPosition();
    }
    return position;
}

// RET 4. Ignored while following a positionable.
// FUNCTION: VC1 0x0045a940
void KdEffectEmitter::SetPosition(const Matrix3& value)
{
    if (!positionable) {
        position = value;
    }
}

// RET 4. this = IPositionableUser. Keeps the current position, then follows
// source (counted).
// FUNCTION: VC1 0x0045a960
void KdEffectEmitter::SetPositionable(const KdPtr<IPositionable>& source)
{
    position = GetPosition();
    if (source.ptr) {
        source.ptr->AddRef();
    }
    if (positionable) {
        positionable->Release();
    }
    positionable = source.ptr;
}

// ADD ECX, 0x30; deleting destructor (IAbstract vtable+0x0c) on zero refs.
// FUNCTION: VC1 0x0045aa80
unsigned KdEffectEmitter::Release()
{
    IUnknown* face;
    unsigned result;
    typedef void (__thiscall *FnDeletingDestructor)(void* object, int arg);

    face = static_cast<IUnknown*>(static_cast<IAbstract*>(this));
    result = --refs;
    if (result == 0) {
        reinterpret_cast<FnDeletingDestructor>((*reinterpret_cast<void***>(face))[3])(face, 1);
        return 0;
    }
    return result;
}

// refs at +0x34; only counts up from nonzero.
// FUNCTION: VC1 0x0045aaa0
unsigned KdEffectEmitter::AddRef()
{
    unsigned value = refs;
    if (value != 0) {
        ++value;
        refs = value;
    }
    return value;
}

// RET 8. this = IWithLifeTime. Advances the emitter to the new lifetime at
// the current position * localMatrix. In local mode the emitter gets the
// transform (and its creation position rotated by it) and moves at the
// origin; otherwise it just moves to the transformed origin.
// FUNCTION: VC1 0x0045aab0
void KdEffectEmitter::Update(unsigned sliceBit, int dt)
{
    (void)sliceBit;
    lifeTime += dt;
    float time = lifeTime * 0.001f;
    float step = dt * 0.001f;
    Matrix3 at = GetPosition() * localMatrix;
    if (localMode) {
        g_effectLocalTransform[0][0] = at._11;
        g_effectLocalTransform[1][0] = at._12;
        g_effectLocalTransform[0][1] = at._21;
        g_effectLocalTransform[1][1] = at._22;
        g_effectLocalTransform[0][3] = at._31;
        g_effectLocalTransform[1][3] = at._32;
        emitter->SetMatrix(&g_effectLocalTransform[0][0]);
        emitter->SetPosition(emitterX * at._11 + emitterY * at._12,
            emitterX * at._21 + emitterY * at._22, 0.0f);
        emitter->Move(time, step, 0.0f, 0.0f, 0.0f, enabled);
    }
    else {
        emitter->Move(time, step, at._31, at._32, 0.0f, enabled);
    }
    emitter->Prepare(time, step);
    if (!started && emitter->HasParticles()) {
        started = true;
    }
}

// this = IShutdownable. Stops following its positionable.
// FUNCTION: VC1 0x0045ac60
int KdEffectEmitter::Shutdown()
{
    IPositionable* queried;

    if (positionable) {
        positionable->Release();
    }
    if (!g_effectNoTarget.ptr) {
        positionable = 0;
        return 0;
    }
    queried = static_cast<IPositionable*>(g_effectNoTarget.ptr->QueryInterface(&g_iidPositionable));
    positionable = queried;
    if (queried) {
        queried->AddRef();
    }
    return 0;
}

// RET 4. graphics is a KdPtr<IGraphics>*. Draws through g_graphics when the
// effect's BoundingRect at the current origin is on screen.
// FUNCTION: VC1 0x0045aca0
int KdEffectEmitter::Render(void* graphics)
{
    const KdPtr<IGraphics>& target = *static_cast<const KdPtr<IGraphics>*>(graphics);

    if (visible) {
        const Matrix3& at = GetPosition();
        const RectF& bounds = static_cast<KdEffect*>(effect)->bounds;
        Rect rect(static_cast<SQInteger>(bounds.x + at._31),
            static_cast<SQInteger>(bounds.y + at._32),
            static_cast<SQInteger>(bounds.width),
            static_cast<SQInteger>(bounds.height));
        if (IsRectVisible(target, rect)) {
            SetGraphics(target);
            emitter->Render();
            AssignGraphics(&g_graphics, g_effectNoTarget);
        }
    }
    return 0;
}

// EAX = this. The emitter is deleted, the effect released, then the
// positionable member.
// FUNCTION: VC1 0x0045ad40
KdEffectEmitter::~KdEffectEmitter()
{
    if (emitter) {
        delete emitter;
        emitter = 0;
    }
    if (effect) {
        effect->Release();
        effect = 0;
    }
}

// RET 8. Method table 0x004ee3f4 (guard bit 0x004ee404, atexit destructor
// 0x004cb450); every closure calls through the vtable.
// FUNCTION: VC1 0x0045b150
unsigned char KdEffectEmitter::GetMember(const char* name, SquirrelObject* out)
{
    static ScriptMethodTable methods;

    if (methods.Empty()) {
        // STRING: VC1 0x004d7a34
        methods["Reset"] = CreateScriptMethod<KdEffectEmitter>(&KdEffectEmitter::Reset);
        methods[kScriptSetRenderLayer] = CreateScriptMethod<KdEffectEmitter>(&KdEffectEmitter::SetRenderLayer);
        methods[kScriptSetEnabled] = CreateScriptMethod<KdEffectEmitter>(&KdEffectEmitter::SetEnabled);
        // STRING: VC1 0x004d7a3c
        methods["SetLocalMode"] = CreateScriptMethod<KdEffectEmitter>(&KdEffectEmitter::SetLocalMode);
        // STRING: VC1 0x004d7a4c
        methods["SetLocalMatrix"] = CreateScriptMethod<KdEffectEmitter>(&KdEffectEmitter::SetLocalMatrix);
    }
    return methods.Find(name, out);
}

// ESI = this, RET 8. Takes ownership of source, keeps owner counted and
// remembers the emitter's own position.
// FUNCTION: VC1 0x0045b410
KdEffectEmitter::KdEffectEmitter(IPyroEmitter* source, IEffect* owner)
{
    float z;

    visible = true;
    enabled = true;
    localMode = true;
    started = false;
    emitterX = 0.0f;
    emitterY = 0.0f;
    effect = owner;
    lifeTime = 0;
    emitter = source;
    renderLayer = 200;
    emitter->SetUserData(this);
    effect->AddRef();
    emitter->GetPosition(&emitterX, &emitterY, &z);
}

// EDI = result.
// FUNCTION: VC1 0x0045b590
KdPtr<IAbstract> CreateEffectEmitter(IPyroEmitter* emitter, IEffect* effect)
{
    KdEffectEmitter* object;
    IAbstract* face;
    IAbstract* queried;

    object = static_cast<KdEffectEmitter*>(malloc(sizeof(KdEffectEmitter)));
    if (!object) {
        queried = 0;
    }
    else {
        new (object) KdEffectEmitter(emitter, effect);
        face = static_cast<IAbstract*>(object);
        if (!face) {
            queried = 0;
        }
        else {
            queried = static_cast<IAbstract*>(face->QueryInterface(&g_iidAbstract));
        }
    }
    KdPtr<IAbstract> result(queried);
    result->Release();
    return result;
}

// XOR AL,AL; RET 8.
// FUNCTION: VC1 0x0045b5f0
unsigned char KdEffect::SetMember(void* a, void* b)
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
    typedef void (__thiscall *FnDeletingDestructor)(void* object, int arg);

    face = static_cast<IUnknown*>(static_cast<IAbstract*>(this));
    result = --refs;
    if (result == 0) {
        reinterpret_cast<FnDeletingDestructor>((*reinterpret_cast<void***>(face))[3])(face, 1);
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
unsigned char KdEffect::GetMember(const char* name, SquirrelObject* out)
{
    static ScriptMethodTable methods;

    if (methods.Empty()) {
        // STRING: VC1 0x004d6914
        methods["CreateEmitter"] = CreateScriptMethod<KdEffect>(&KdEffect::CreateFirstEmitter);
    }
    return methods.Find(name, out);
}

}
