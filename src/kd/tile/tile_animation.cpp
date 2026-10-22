#include "kd/tile/tile_animation.h"

#include "kd/iface/iid.h"
#include "bindings/component.h"
#include "bindings/matrix3.h"
#include "bindings/sqplus.h"
#include "host/platform.h"

#include <math.h>
#include <new>
#include <stdlib.h>

namespace vc1 {

float RoundToNearest(float value);

// STRING: VC1 0x004cfd40
static const char kTileAnimation[] = "TileAnimation";
// GLOBAL: VC1 0x004d0b10
const char* g_kdNameTileAnimation = kTileAnimation;

// STRING: VC1 0x004d1cd8
static const char kScriptSetAnimationData[] = "SetAnimationData";
// STRING: VC1 0x004d1cec
static const char kScriptGetAnimationName[] = "GetAnimationName";
// STRING: VC1 0x004d1d00
static const char kScriptGetAnimationTime[] = "GetAnimationTime";

// Inferred name. This file's own never-assigned null object.
// GLOBAL: VC1 0x004ed69c
KdRef<IUnknown> g_kdTileAnimationNullObject;

static const double kTwoPi = 6.283185307179586;

// MOV EAX, "TileAnimation"; RET. IScriptObject vtable+0x14.
// FUNCTION: VC1 0x004087e0
const char* KdTileAnimation::GetTypeName()
{
    return kTileAnimation;
}

// RET 4. Script "SetAnimationData".
// FUNCTION: VC1 0x004a4bb0
void KdTileAnimation::SetAnimationData(const KdPtr<ITileAnimationData>& value)
{
    data.Assign(value);
}

// RET 4. IUpdateable vtable+0x10: ignored.
// FUNCTION: VC1 0x004a4bf0
int KdTileAnimation::SetLayer(unsigned)
{
    return 0;
}

// IUpdateable vtable+0x0c.
// FUNCTION: VC1 0x004a4c00
int KdTileAnimation::GetUpdateLayer()
{
    return 0x20;
}

// IScriptObject vtable+0x10. RET 8.
// FUNCTION: VC1 0x004a4c10
unsigned char KdTileAnimation::SetMember(void*, void*)
{
    return 0;
}

// RET 4. Script "GetAnimationTime": the named animation's length, or -1.
// FUNCTION: VC1 0x004a4c20
int KdTileAnimation::GetAnimationTime(const KdString& name)
{
    const TileAnimation* animation;

    if (!data.ptr) {
        return -1;
    }
    animation = data->Find(name);
    if (!animation) {
        return -1;
    }
    return animation->time;
}

// RET 4. IObjectControl vtable+0x0c: the slave's ITileObject face.
// FUNCTION: VC1 0x004a4c50
void KdTileAnimation::SetSlaveObject(const KdPtr<IAbstract>& object)
{
    if (slave.ptr) {
        slave.ptr->Release();
    }
    if (!object.ptr) {
        slave.ptr = 0;
        return;
    }
    slave.ptr = static_cast<ITileObject*>(object->QueryInterface(&g_iidTileObject));
    if (slave.ptr) {
        slave.ptr->AddRef();
    }
}

// IAnimateable vtable+0x10 (tail call).
// FUNCTION: VC1 0x004a4ca0
const KdStringList& KdTileAnimation::GetAnimationNames()
{
    return data->GetNames();
}

// RET 4. IPositionableUser vtable+0x0c.
// FUNCTION: VC1 0x004a4cb0
void KdTileAnimation::SetPositionable(const KdPtr<IPositionable>& source)
{
    positionable.Assign(source);
}

// ESI=this.
// FUNCTION: VC1 0x004a4cf0
KdTileAnimation::~KdTileAnimation() throw()
{
}

// ADD ECX, 0x18; deleting destructor through IAbstract on zero.
// FUNCTION: VC1 0x004a4de0
unsigned KdTileAnimation::Release()
{
    IAbstract* const self = static_cast<IAbstract*>(this);
    unsigned value = --self->refs;
    if (!value) {
        delete self;
        return 0;
    }
    return value;
}

// FUNCTION: VC1 0x004a4e00
unsigned KdTileAnimation::AddRef()
{
    unsigned value = refs;
    if (value != 0) {
        ++value;
        refs = value;
    }
    return value;
}

// RET 8. IAnimateable vtable+0x0c. An unknown name logs a warning and clears
// the slave's tile; the phase scales the animation length into the start
// time.
// FUNCTION: VC1 0x004a4e10
bool KdTileAnimation::SetAnimation(const KdString& name, float phase)
{
    if (!data.ptr) {
        return false;
    }
    current = data->Find(name);
    if (!current) {
        // STRING: VC1 0x004d1d14
        HostLogWarning(&g_kdNameTileAnimation, "Animation \"%s\" is not found", name.c_str());
        if (slave.ptr) {
            slave->SetTile(-1);
        }
        return false;
    }
    elapsed = static_cast<int>(current->time * phase);
    return true;
}

// Inlined KdRef reset to this file's null object.
template <class T>
static __forceinline void ResetToNull(KdRef<T>& slot, const char** iid)
{
    if (slot.ptr) {
        slot.ptr->Release();
    }
    if (!g_kdTileAnimationNullObject.ptr) {
        slot.ptr = 0;
        return;
    }
    slot.ptr = static_cast<T*>(g_kdTileAnimationNullObject->QueryInterface(iid));
    if (slot.ptr) {
        slot.ptr->AddRef();
    }
}

// IShutdownable vtable+0x0c (ECX=this+4).
// FUNCTION: VC1 0x004a4e80
int KdTileAnimation::Shutdown()
{
    ResetToNull(data, &g_iidTileAnimationData);
    ResetToNull(slave, &g_iidTileObject);
    ResetToNull(positionable, &g_iidPositionable);
    current = 0;
    return 0;
}

// EAX=this.
// FUNCTION: VC1 0x004a4f40
KdTileAnimation::KdTileAnimation() throw()
    : current(0)
{
}

// FUNCTION: VC1 0x004a4fb0
void** CreateKdTileAnimation(void** out)
{
    void* const memory = malloc(sizeof(KdTileAnimation));
    IAbstract* object = 0;
    IUnknown* queried;

    if (memory) {
        object = new (memory) KdTileAnimation();
    }
    queried = object ? static_cast<IUnknown*>(object->QueryInterface(&g_iidAbstract)) : 0;
    *out = queried;
    if (queried) {
        queried->AddRef();
    }
    static_cast<IUnknown*>(*out)->Release();
    return out;
}

// Script "GetAnimationName": the name whose animation is the current one,
// or the shared empty string.
// FUNCTION: VC1 0x004a5000
const char* KdTileAnimation::GetAnimationName()
{
    if (data.ptr && current) {
        const KdStringList& names = data->GetNames();
        unsigned i;
        for (i = 0; i < names.items.size >> 4; ++i) {
            const KdString& name = reinterpret_cast<const KdString*>(names.items.begin)[i];
            if (data->Find(name) == current) {
                return name.c_str();
            }
        }
    }
    return reinterpret_cast<const char*>(&g_kdLogTag);
}

// RET 8. IUpdateable vtable+0x14. Channel = the X axis angle in [0, 2pi)
// scaled to the channel count (rounded, wrapped); frame = elapsed * frames
// / time, wrapped.
// FUNCTION: VC1 0x004a5080
void KdTileAnimation::Update(unsigned, int dt)
{
    elapsed += dt;
    if (current && slave.ptr && positionable.ptr) {
        const Matrix3& position = positionable->GetPosition();
        const unsigned count = current->channels.Count();
        const float angle = static_cast<float>(fmod(
            static_cast<float>(atan2(position._12, position._11)) + kTwoPi, kTwoPi));
        const unsigned channel = static_cast<unsigned>(
            RoundToNearest(static_cast<float>(angle * count / kTwoPi))) % count;
        const KdArray<int>& frames = current->channels[channel];
        const unsigned frameCount = frames.Count();
        slave->SetTile(frames[static_cast<unsigned>(elapsed) * frameCount
            / static_cast<unsigned>(current->time) % frameCount]);
    }
}

// RET 8. No script instance; the own table only.
// FUNCTION: VC1 0x004a5380
unsigned char KdTileAnimation::GetMember(const char* name, SquirrelObject* out)
{
    static ScriptMethodTable methods;

    if (methods.Empty()) {
        methods[kScriptSetAnimationData] =
            CreateScriptMethod<KdTileAnimation>(&KdTileAnimation::SetAnimationData);
        methods[kScriptGetAnimationName] =
            CreateScriptMethod<KdTileAnimation>(&KdTileAnimation::GetAnimationName);
        methods[kScriptGetAnimationTime] =
            CreateScriptMethod<KdTileAnimation>(&KdTileAnimation::GetAnimationTime);
    }
    return methods.Find(name, out);
}

}
