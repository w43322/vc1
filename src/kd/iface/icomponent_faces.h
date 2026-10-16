#ifndef VC1_KD_ICOMPONENT_FACES_H
#define VC1_KD_ICOMPONENT_FACES_H

#include "kd/iface/iunknown.h"
#include "kd/iface/iid.h"
#include "kd/iface/iabstract.h"
#include "kd/iface/icamera.h"
#include "kd/iface/iclassifiable.h"
#include "kd/iface/icloneable.h"
#include "kd/iface/iidentifiable.h"
#include "kd/iface/iloadable.h"
#include "kd/iface/ipositionable.h"
#include "kd/iface/irenderable.h"
#include "kd/iface/irender_target.h"
#include "kd/iface/ishutdownable.h"
#include "kd/iface/isystem.h"
#include "kd/iface/itexture.h"
#include "kd/iface/iupdateable.h"

class SquirrelObject;

namespace vc1 {

struct RectF;

// Component faces used only through the g5 script natives (InitHostServices,
// 0x004b61b0). Slot positions come from the natives' vtable calls; names are
// the script names and are inferred. Slots no native calls are placeholders.

// SIZE 0x04
struct IInputStream : IUnknown
{
    virtual unsigned Read(void* buffer, unsigned size) = 0; // vtable+0x0c
};

// SIZE 0x04
struct IOutputStream : IUnknown
{
    virtual unsigned Write(const void* buffer, unsigned size) = 0; // vtable+0x0c
};

// readComponent (0x004b53d0).
// SIZE 0x04
struct IReadable : IUnknown
{
    virtual bool ReadFrom(const KdPtr<IInputStream>& stream) = 0; // vtable+0x0c
};

// writeComponent (0x004b52c0).
// SIZE 0x04
struct IWriteable : IUnknown
{
    virtual bool WriteTo(const KdPtr<IOutputStream>& stream) = 0; // vtable+0x0c
};

// saveToFile (0x004b5db0).
// SIZE 0x04
struct ISaveable : IUnknown
{
    virtual bool Save(const char* path) = 0; // vtable+0x0c
};

// getLifeTime (0x004b4d70) and isExpired (0x004b4cf0).
// SIZE 0x04
struct IWithLifeTime : IUnknown
{
    virtual void LifeTime0c() = 0;    // vtable+0x0c
    virtual void LifeTime10() = 0;    // vtable+0x10
    virtual void LifeTime14() = 0;    // vtable+0x14
    virtual int GetLifeTime() = 0;    // vtable+0x18
    virtual bool IsExpired() = 0;     // vtable+0x1c
};

// setVisible (0x004b54e0) and isVisible (0x004b5560).
// SIZE 0x04
struct IVisible : IUnknown
{
    virtual void SetVisible(bool visible) = 0; // vtable+0x0c
    virtual bool IsVisible() = 0;              // vtable+0x10
};

// enableControl (0x004b5c90) and isControlEnabled (0x004b5c10).
// SIZE 0x04
struct IManualControl : IUnknown
{
    virtual void EnableControl(bool enable) = 0; // vtable+0x0c
    virtual bool IsControlEnabled() = 0;         // vtable+0x10
};

// loadFromScript (0x004b5b10).
// SIZE 0x04
struct IScriptLoadable : IUnknown
{
    virtual bool LoadFromScript(SquirrelObject& table) = 0; // vtable+0x0c
};

// saveToScript (0x004b5a70).
// SIZE 0x04
struct IScriptSaveable : IUnknown
{
    virtual SquirrelObject SaveToScript() = 0; // vtable+0x0c
};

// setPositionable (0x004b5670).
// SIZE 0x04
struct IPositionableUser : IUnknown
{
    virtual void SetPositionable(const KdPtr<IPositionable>& source) = 0; // vtable+0x0c
};

// getBoundingRect (0x004b4df0) passes the never-written 0x004ed6b0.
// SIZE 0x04
struct IBoundingInfo : IUnknown
{
    virtual const RectF& GetBoundingRect(const RenderMask& mask) = 0; // vtable+0x0c
};

// setSlaveObject (0x004b5b90).
// SIZE 0x04
struct IObjectControl : IUnknown
{
    virtual void SetSlaveObject(const KdPtr<IAbstract>& slave) = 0; // vtable+0x0c
};

// setAnimation (0x004b55e0).
// SIZE 0x04
struct IAnimateable : IUnknown
{
    virtual bool SetAnimation(const KdString& name, float speed) = 0; // vtable+0x0c
};

#define VC1_KD_IID(T, slot) \
    template <> \
    struct KdIid<T> \
    { \
        static const char** Get() { return &slot; } \
    };

VC1_KD_IID(IAbstract, g_iidAbstract)
VC1_KD_IID(IInputStream, g_iidInputStream)
VC1_KD_IID(IOutputStream, g_iidOutputStream)
VC1_KD_IID(IIdentifiable, g_iidIdentifiable)
VC1_KD_IID(IShutdownable, g_iidShutdownable)
VC1_KD_IID(IRenderTarget, g_iidRenderTarget)
VC1_KD_IID(IReadable, g_iidReadable)
VC1_KD_IID(IWriteable, g_iidWriteable)
VC1_KD_IID(ILoadable, g_iidLoadable)
VC1_KD_IID(ISaveable, g_iidSaveable)
VC1_KD_IID(ICloneable, g_iidCloneable)
VC1_KD_IID(ITexture, g_iidTexture)
VC1_KD_IID(ISystem, g_iidSystem)
VC1_KD_IID(IUpdateable, g_iidUpdateable)
VC1_KD_IID(IWithLifeTime, g_iidWithLifeTime)
VC1_KD_IID(IRenderable, g_iidRenderable)
VC1_KD_IID(IClassifiable, g_iidClassifiable)
VC1_KD_IID(IVisible, g_iidVisible)
VC1_KD_IID(IManualControl, g_iidManualControl)
VC1_KD_IID(IScriptLoadable, g_iidScriptLoadable)
VC1_KD_IID(IScriptSaveable, g_iidScriptSaveable)
VC1_KD_IID(IPositionable, g_iidPositionable)
VC1_KD_IID(IPositionableUser, g_iidPositionableUser)
VC1_KD_IID(IBoundingInfo, g_iidBoundingInfo)
VC1_KD_IID(ICamera, g_iidCamera)
VC1_KD_IID(IObjectControl, g_iidObjectControl)
VC1_KD_IID(IAnimateable, g_iidAnimateable)

#undef VC1_KD_IID

}

#endif
