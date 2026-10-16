#include "kd/iface/iid.h"
#include "kd/scene/compound_object.h"
#include "kd/resource/resource_manager.h"
#include "kd/scene/scripted_object.h"
#include "kd/scene/scene.h"
#include "kd/world/game_level.h"
#include "kd/file/file.h"
#include "kd/render/object_watcher.h"
#include "kd/tile/tile_animation_data.h"
#include "kd/tile/tile_layer.h"
#include "kd/blob/blob.h"
#include "kd/road/grid_router.h"
#include "kd/road/pass_map.h"
#include "kd/road/road_renderer.h"
#include "kd/build/terrain_builder.h"
#include "kd/scene/camera.h"
#include "kd/render/classificated_renderer.h"
#include "kd/display/effect.h"

namespace vc1 {

// STRING: VC1 0x004d4ce8
static const char kIidPositionable[] = "IPositionable";
// STRING: VC1 0x004d4d08
static const char kIidScriptLoadable[] = "IScriptLoadable";
// STRING: VC1 0x004d4d58
static const char kIidClassifiable[] = "IClassifiable";

// STRING: VC1 0x004d4e9c
static const char kIidAbstract[] = "IAbstract";
// GLOBAL: VC1 0x004d0584
const char* g_iidAbstract = kIidAbstract;

// STRING: VC1 0x004d4e8c
static const char kIidInputStream[] = "IInputStream";
// GLOBAL: VC1 0x004d0588
const char* g_iidInputStream = kIidInputStream;

// GLOBAL: VC1 0x004d4e6c
extern const char kIidIdentifiable[] = "IIdentifiable";
// GLOBAL: VC1 0x004d0590
const char* g_iidIdentifiable = kIidIdentifiable;

// STRING: VC1 0x004d4e5c
static const char kIidShutdownable[] = "IShutdownable";
// GLOBAL: VC1 0x004d0594
const char* g_iidShutdownable = kIidShutdownable;

// STRING: VC1 0x004d4e50
static const char kIidDisplay[] = "IDisplay";
// GLOBAL: VC1 0x004d0598
const char* g_iidDisplay = kIidDisplay;

// STRING: VC1 0x004d4e40
static const char kIidRenderTarget[] = "IRenderTarget";
// GLOBAL: VC1 0x004d059c
const char* g_iidRenderTarget = kIidRenderTarget;

// STRING: VC1 0x004d4e34
static const char kIidGraphics[] = "IGraphics";
// GLOBAL: VC1 0x004d05a0
const char* g_iidGraphics = kIidGraphics;

// STRING: VC1 0x004d4e2c
static const char kIidEffect[] = "IEffect";
// GLOBAL: VC1 0x004d05a4
const char* g_iidEffect = kIidEffect;

// STRING: VC1 0x004d4e1c
static const char kIidEffectEmitter[] = "IEffectEmitter";
// GLOBAL: VC1 0x004d05a8
const char* g_iidEffectEmitter = kIidEffectEmitter;

// STRING: VC1 0x004d4df8
static const char kIidLoadable[] = "ILoadable";
// GLOBAL: VC1 0x004d05b4
const char* g_iidLoadable = kIidLoadable;

// STRING: VC1 0x004d4de0
static const char kIidCloneable[] = "ICloneable";
// GLOBAL: VC1 0x004d05bc
const char* g_iidCloneable = kIidCloneable;

// STRING: VC1 0x004d4dd4
static const char kIidTexture[] = "ITexture";
// GLOBAL: VC1 0x004d05c0
const char* g_iidTexture = kIidTexture;

// STRING: VC1 0x004d4dcc
static const char kIidFont[] = "IFont";
// GLOBAL: VC1 0x004d05c4
const char* g_iidFont = kIidFont;

// STRING: VC1 0x004d4dc4
static const char kIidSystem[] = "ISystem";
// GLOBAL: VC1 0x004d05c8
const char* g_iidSystem = kIidSystem;

// STRING: VC1 0x004d4db0
static const char kIidFunctionalGroup[] = "IFunctionalGroup";
// GLOBAL: VC1 0x004d05cc
const char* g_iidFunctionalGroup = kIidFunctionalGroup;

// STRING: VC1 0x004d4da0
static const char kIidComponentGroup[] = "IComponentGroup";
// GLOBAL: VC1 0x004d05d0
const char* g_iidComponentGroup = kIidComponentGroup;

// STRING: VC1 0x004d4d90
static const char kIidGroupMember[] = "IGroupMember";
// GLOBAL: VC1 0x004d05d4
const char* g_iidGroupMember = kIidGroupMember;

// STRING: VC1 0x004d4d84
static const char kIidUpdateable[] = "IUpdateable";
// GLOBAL: VC1 0x004d05d8
const char* g_iidUpdateable = kIidUpdateable;

// STRING: VC1 0x004d4d68
static const char kIidRenderable[] = "IRenderable";
// GLOBAL: VC1 0x004d05e0
const char* g_iidRenderable = kIidRenderable;

// STRING: VC1 0x004d4d38
static const char kIidResourceManager[] = "IResourceManager";
// GLOBAL: VC1 0x004d05ec
const char* g_iidResourceManager = kIidResourceManager;

// STRING: VC1 0x004d4cbc
static const char kIidCamera[] = "ICamera";
// GLOBAL: VC1 0x004d060c
const char* g_iidCamera = kIidCamera;

// STRING: VC1 0x004d4c8c
static const char kIidSoundDriver[] = "ISoundDriver";
// GLOBAL: VC1 0x004d0618
const char* g_iidSoundDriver = kIidSoundDriver;

// STRING: VC1 0x004d4c84
static const char kIidSound[] = "ISound";
// GLOBAL: VC1 0x004d061c
const char* g_iidSound = kIidSound;

// STRING: VC1 0x004d4d18
static const char kIidScriptObject[] = "IScriptObject";
// GLOBAL: VC1 0x004d05f4
const char* g_iidScriptObject = kIidScriptObject;

// STRING: VC1 0x004d4e7c
static const char kIidOutputStream[] = "IOutputStream";
// GLOBAL: VC1 0x004d058c
const char* g_iidOutputStream = kIidOutputStream;
// STRING: VC1 0x004d4e10
static const char kIidReadable[] = "IReadable";
// GLOBAL: VC1 0x004d05ac
const char* g_iidReadable = kIidReadable;
// STRING: VC1 0x004d4e04
static const char kIidWriteable[] = "IWriteable";
// GLOBAL: VC1 0x004d05b0
const char* g_iidWriteable = kIidWriteable;
// GLOBAL: VC1 0x004d05b8
// STRING: VC1 0x004d4dec
const char* g_iidSaveable = "ISaveable";
// STRING: VC1 0x004d4d74
static const char kIidWithLifeTime[] = "IWithLifeTime";
// GLOBAL: VC1 0x004d05dc
const char* g_iidWithLifeTime = kIidWithLifeTime;
// GLOBAL: VC1 0x004d05e4
const char* g_iidClassifiable = kIidClassifiable;
// STRING: VC1 0x004d4d4c
static const char kIidVisible[] = "IVisible";
// GLOBAL: VC1 0x004d05e8
const char* g_iidVisible = kIidVisible;
// GLOBAL: VC1 0x004d0628
// STRING: VC1 0x004d4c54
const char* g_iidTileManager = "ITileManager";
// GLOBAL: VC1 0x004d05f0
// STRING: VC1 0x004d4d28
const char* g_iidManualControl = "IManualControl";
// GLOBAL: VC1 0x004d05f8
const char* g_iidScriptLoadable = kIidScriptLoadable;
// STRING: VC1 0x004d4cf8
static const char kIidScriptSaveable[] = "IScriptSaveable";
// GLOBAL: VC1 0x004d05fc
const char* g_iidScriptSaveable = kIidScriptSaveable;
// GLOBAL: VC1 0x004d0600
const char* g_iidPositionable = kIidPositionable;
// STRING: VC1 0x004d4cd4
static const char kIidPositionableUser[] = "IPositionableUser";
// GLOBAL: VC1 0x004d0604
const char* g_iidPositionableUser = kIidPositionableUser;
// GLOBAL: VC1 0x004d0608
// STRING: VC1 0x004d4cc4
const char* g_iidBoundingInfo = "IBoundingInfo";
// STRING: VC1 0x004d4cac
static const char kIidInputHandler[] = "IInputHandler";
// GLOBAL: VC1 0x004d0610
const char* g_iidInputHandler = kIidInputHandler;
// GLOBAL: VC1 0x004d0614
// STRING: VC1 0x004d4c9c
const char* g_iidObjectControl = "IObjectControl";
// GLOBAL: VC1 0x004d0620
// STRING: VC1 0x004d4c74
const char* g_iidAnimateable = "IAnimateable";
// STRING: VC1 0x004d4c64
static const char kIidStateProvider[] = "IStateProvider";
// GLOBAL: VC1 0x004d0624
const char* g_iidStateProvider = kIidStateProvider;
// STRING: VC1 0x004cfe2c
static const char kIidTileAnimationData[] = "ITileAnimationData";
// GLOBAL: VC1 0x004d0ac8
const char* g_iidTileAnimationData = kIidTileAnimationData;
// GLOBAL: VC1 0x004d0acc
// STRING: VC1 0x004cfe20
const char* g_iidTileObject = "ITileObject";
// STRING: VC1 0x004cfe14
static const char kIidGridMap[] = "IGridMap";
// GLOBAL: VC1 0x004d0ad0
const char* g_iidGridMap = kIidGridMap;
// STRING: VC1 0x004cfe04
static const char kIidRoadRenderer[] = "IRoadRenderer";
// GLOBAL: VC1 0x004d0ad4
const char* g_iidRoadRenderer = kIidRoadRenderer;
// STRING: VC1 0x004cfdf8
static const char kIidGridRouter[] = "IGridRouter";
// GLOBAL: VC1 0x004d0ad8
const char* g_iidGridRouter = kIidGridRouter;
// STRING: VC1 0x004cfdec
static const char kIidTileLayer[] = "ITileLayer";
// GLOBAL: VC1 0x004d0adc
const char* g_iidTileLayer = kIidTileLayer;
// STRING: VC1 0x004cfddc
static const char kIidGridRenderer[] = "IGridRenderer";
// GLOBAL: VC1 0x004d0ae0
const char* g_iidGridRenderer = kIidGridRenderer;

// RET 4.
// FUNCTION: VC1 0x0045a9c0
void* KdEffectEmitter::QueryInterface(const char** iid)
{
    const char* name = *iid;
    if (name == kIidScriptObject) {
        if (!this) {
            return 0;
        }
        return static_cast<IScriptObject*>(this);
    }
    if (name == kIidWithLifeTime || name == kIidUpdateable) {
        if (!this) {
            return 0;
        }
        return static_cast<IWithLifeTime*>(this);
    }
    if (name == kIidRenderable || name == kIidEffectEmitter) {
        return static_cast<IEffectEmitter*>(this);
    }
    if (name == kIidPositionableUser) {
        if (!this) {
            return 0;
        }
        return static_cast<IPositionableUser*>(this);
    }
    if (name == kIidPositionable) {
        if (!this) {
            return 0;
        }
        return static_cast<IPositionable*>(this);
    }
    if (name == kIidVisible) {
        if (!this) {
            return 0;
        }
        return static_cast<IVisible*>(this);
    }
    if (name == kIidIdentifiable) {
        if (!this) {
            return 0;
        }
        return static_cast<IIdentifiable*>(this);
    }
    if (name == kIidShutdownable) {
        if (!this) {
            return 0;
        }
        return static_cast<IShutdownable*>(this);
    }
    if (name == kIidAbstract) {
        return static_cast<IAbstract*>(this);
    }
    return 0;
}

// this = IEffect, RET 4.
// FUNCTION: VC1 0x0045b650
void* KdEffect::QueryInterface(const char** iid)
{
    const char* name = *iid;
    if (name == kIidScriptObject) {
        if (!this) {
            return 0;
        }
        return static_cast<IScriptObject*>(this);
    }
    if (name == kIidEffect) {
        return static_cast<IEffect*>(this);
    }
    if (name == kIidLoadable) {
        if (!this) {
            return 0;
        }
        return static_cast<ILoadable*>(this);
    }
    if (name == kIidScriptLoadable) {
        if (!this) {
            return 0;
        }
        return static_cast<IScriptLoadable*>(this);
    }
    if (name == kIidAbstract) {
        return static_cast<IAbstract*>(this);
    }
    return 0;
}

// RET 4. The IAbstract face is returned without the null check.
// FUNCTION: VC1 0x0046a690
void* KdFile::QueryInterface(const char** iid)
{
    const char* name = *iid;
    if (name == kIidInputStream) {
        return static_cast<IInputStream*>(this);
    }
    if (name == kIidOutputStream) {
        if (!this) {
            return 0;
        }
        return static_cast<IOutputStream*>(this);
    }
    if (name == kIidScriptObject) {
        if (!this) {
            return 0;
        }
        return static_cast<IScriptObject*>(this);
    }
    if (name == kIidShutdownable) {
        if (!this) {
            return 0;
        }
        return static_cast<IShutdownable*>(this);
    }
    if (name == kIidAbstract) {
        return static_cast<IAbstract*>(this);
    }
    return 0;
}

// RET 4. The IInputStream face is the object itself.
// FUNCTION: VC1 0x0046c990
void* KdBlob::QueryInterface(const char** iid)
{
    const char* name = *iid;
    if (name == kIidScriptObject) {
        return static_cast<IScriptObject*>(this);
    }
    if (name == kIidInputStream) {
        return static_cast<IInputStream*>(this);
    }
    if (name == kIidOutputStream) {
        return static_cast<IOutputStream*>(this);
    }
    if (name == kIidReadable) {
        return static_cast<IReadable*>(this);
    }
    if (name == kIidWriteable) {
        return static_cast<IWriteable*>(this);
    }
    if (name == kIidScriptLoadable) {
        return static_cast<IScriptLoadable*>(this);
    }
    if (name == kIidScriptSaveable) {
        return static_cast<IScriptSaveable*>(this);
    }
    if (name == kIidAbstract) {
        return static_cast<IAbstract*>(this);
    }
    return 0;
}

// RET 4.
// FUNCTION: VC1 0x0046d520
void* KdResourceManager::QueryInterface(const char** iid)
{
    const char* name = *iid;
    if (name == kIidComponentGroup || name == kIidShutdownable) {
        return static_cast<KdComponentGroup*>(this);
    }
    if (name == kIidResourceManager) {
        return static_cast<IResourceManager*>(this);
    }
    if (name == kIidScriptObject) {
        return static_cast<IScriptObject*>(this);
    }
    if (name == kIidAbstract) {
        return static_cast<IAbstract*>(this);
    }
    return 0;
}

// RET 4.
// FUNCTION: VC1 0x0046e430
void* KdScene::QueryInterface(const char** iid)
{
    const char* name = *iid;
    if (name == kIidScriptObject) {
        return static_cast<IScriptObject*>(this);
    }
    if (name == kIidRenderable) {
        return static_cast<IRenderable*>(this);
    }
    if (name == kIidUpdateable) {
        return static_cast<IUpdateable*>(this);
    }
    if (name == kIidInputHandler) {
        return static_cast<IInputHandler*>(this);
    }
    if (name == kIidVisible) {
        return static_cast<IVisible*>(this);
    }
    if (name == kIidIdentifiable) {
        return static_cast<IIdentifiable*>(this);
    }
    if (name == kIidScriptLoadable) {
        return static_cast<IScriptLoadable*>(this);
    }
    if (name == kIidComponentGroup || name == kIidShutdownable) {
        return static_cast<KdComponentGroup*>(this);
    }
    if (name == kIidAbstract) {
        return static_cast<IAbstract*>(this);
    }
    return 0;
}

// RET 4. The ICamera face also answers IPositionable; no null check on the
// IAbstract face.
// FUNCTION: VC1 0x004701e0
void* KdCamera::QueryInterface(const char** iid)
{
    const char* name = *iid;
    if (name == kIidScriptObject) {
        return static_cast<IScriptObject*>(this);
    }
    if (name == kIidCamera || name == kIidPositionable) {
        return static_cast<ICamera*>(this);
    }
    if (name == kIidScriptLoadable) {
        return static_cast<IScriptLoadable*>(this);
    }
    if (name == kIidAbstract) {
        return static_cast<IAbstract*>(this);
    }
    return 0;
}

// RET 4. A face only when the script defines its member (the input face
// also when LoadFromScript cached a pointer handler), then the compound's.
// FUNCTION: VC1 0x00471040
void* KdScriptedObject::QueryInterface(const char** iid)
{
    void* face;

    if (*iid == kIidUpdateable && !script.GetValue(kScriptUpdate).IsNull()) {
        return static_cast<IUpdateable*>(this);
    }
    if (*iid == kIidRenderable && !script.GetValue(kScriptRender).IsNull()) {
        return static_cast<IRenderable*>(this);
    }
    if (*iid == kIidInputHandler
        && (!script.GetValue(kScriptOnInputChar).IsNull()
            || !script.GetValue(kScriptOnKeyPressed).IsNull()
            || !script.GetValue(kScriptOnKeyReleased).IsNull()
            || !pointerHandlers[0].IsNull()
            || !pointerHandlers[1].IsNull()
            || !pointerHandlers[2].IsNull()
            || !pointerHandlers[3].IsNull())) {
        return static_cast<IInputHandler*>(this);
    }
    if (*iid == kIidStateProvider && !script.GetValue(kScriptGetStateWeight).IsNull()) {
        return static_cast<IStateProvider*>(this);
    }
    face = KdCompoundObject::QueryInterface(iid);
    if (face) {
        return face;
    }
    if (*iid == kIidAbstract) {
        return static_cast<IAbstract*>(this);
    }
    return 0;
}

// Kept in this translation unit so the recovered function directly compares
// the canonical IID string objects, as the original does.
// FUNCTION: VC1 0x00472f00
void* KdCompoundObject::QueryInterface(const char** iid)
{
    const char* name = *iid;
    if (name == kIidScriptObject) {
        return static_cast<IScriptObject*>(this);
    }
    if (name == kIidIdentifiable) {
        return static_cast<IIdentifiable*>(this);
    }
    if (name == kIidClassifiable) {
        return static_cast<IClassifiable*>(this);
    }
    if (name == kIidScriptLoadable) {
        return static_cast<IScriptLoadable*>(this);
    }
    if (name == kIidComponentGroup || name == kIidShutdownable) {
        return static_cast<KdComponentGroup*>(this);
    }
    if (name == kIidPositionable) {
        return static_cast<IPositionable*>(this);
    }
    if (name == kIidAbstract) {
        return static_cast<IAbstract*>(this);
    }
    return 0;
}

// RET 4.
// FUNCTION: VC1 0x004a2750
void* KdTileLayer::QueryInterface(const char** iid)
{
    const char* name = *iid;
    if (name == kIidTileLayer) {
        return static_cast<ITileLayer*>(this);
    }
    if (name == kIidScriptObject) {
        return static_cast<IScriptObject*>(this);
    }
    if (name == kIidIdentifiable) {
        return static_cast<IIdentifiable*>(this);
    }
    if (name == kIidClassifiable) {
        return static_cast<IClassifiable*>(this);
    }
    if (name == kIidReadable) {
        return static_cast<IReadable*>(this);
    }
    if (name == kIidWriteable) {
        return static_cast<IWriteable*>(this);
    }
    if (name == kIidRenderable) {
        return static_cast<IRenderable*>(this);
    }
    if (name == kIidFunctionalGroup) {
        return static_cast<IFunctionalGroup*>(this);
    }
    if (name == kIidAbstract) {
        return static_cast<IAbstract*>(this);
    }
    return 0;
}

// RET 4. this = ITileAnimationData.
// FUNCTION: VC1 0x004a4550
void* KdTileAnimationData::QueryInterface(const char** iid)
{
    const char* name = *iid;
    if (name == kIidTileAnimationData) {
        return this;
    }
    if (name == kIidScriptLoadable) {
        return static_cast<IScriptLoadable*>(this);
    }
    if (name == kIidAbstract) {
        return static_cast<IAbstract*>(this);
    }
    return 0;
}

// RET 4. The compound's faces first.
// FUNCTION: VC1 0x004a5ca0
void* KdTerrainBuilder::QueryInterface(const char** iid)
{
    void* face = KdCompoundObject::QueryInterface(iid);
    if (face) {
        return face;
    }
    if (*iid == kIidInputHandler) {
        return static_cast<IInputHandler*>(this);
    }
    if (*iid == kIidGridRenderer) {
        return static_cast<IGridRenderer*>(this);
    }
    if (*iid == kIidAbstract) {
        return static_cast<IAbstract*>(this);
    }
    return 0;
}

// RET 4. IRenderable shares the IRoadRenderer face.
// FUNCTION: VC1 0x004aa3a0
void* KdRoadRenderer::QueryInterface(const char** iid)
{
    const char* name = *iid;
    if (name == kIidRoadRenderer || name == kIidRenderable) {
        return this;
    }
    if (name == kIidScriptObject) {
        return static_cast<IScriptObject*>(this);
    }
    if (name == kIidIdentifiable) {
        return static_cast<IIdentifiable*>(this);
    }
    if (name == kIidClassifiable) {
        return static_cast<IClassifiable*>(this);
    }
    if (name == kIidWriteable) {
        return static_cast<IWriteable*>(this);
    }
    if (name == kIidReadable) {
        return static_cast<IReadable*>(this);
    }
    if (name == kIidAbstract) {
        return static_cast<IAbstract*>(this);
    }
    return 0;
}

// RET 4. IPositionable shares the IGridMap face.
// FUNCTION: VC1 0x004ac8e0
void* KdPassMap::QueryInterface(const char** iid)
{
    const char* name = *iid;
    if (name == kIidGridMap) {
        return static_cast<IGridMap*>(this);
    }
    if (name == kIidScriptObject) {
        return static_cast<IScriptObject*>(this);
    }
    if (name == kIidScriptLoadable) {
        return static_cast<IScriptLoadable*>(this);
    }
    if (name == kIidShutdownable) {
        return static_cast<IShutdownable*>(this);
    }
    if (name == kIidIdentifiable) {
        return static_cast<IIdentifiable*>(this);
    }
    if (name == kIidReadable) {
        return static_cast<IReadable*>(this);
    }
    if (name == kIidWriteable) {
        return static_cast<IWriteable*>(this);
    }
    if (name == kIidRenderable) {
        return static_cast<IRenderable*>(this);
    }
    if (name == kIidPositionable) {
        return static_cast<IGridMap*>(this);
    }
    if (name == kIidVisible) {
        return static_cast<IVisible*>(this);
    }
    if (name == kIidAbstract) {
        return static_cast<IAbstract*>(this);
    }
    return 0;
}

// RET 4.
// FUNCTION: VC1 0x004adb00
void* KdObjectWatcher::QueryInterface(const char** iid)
{
    const char* name = *iid;
    if (name == kIidScriptObject) {
        return static_cast<IScriptObject*>(this);
    }
    if (name == kIidUpdateable) {
        return static_cast<IUpdateable*>(this);
    }
    if (name == kIidIdentifiable) {
        return static_cast<IIdentifiable*>(this);
    }
    if (name == kIidShutdownable) {
        return static_cast<IShutdownable*>(this);
    }
    if (name == kIidAbstract) {
        return static_cast<IAbstract*>(this);
    }
    return 0;
}

// RET 4.
// FUNCTION: VC1 0x004aef00
void* KdGridRouter::QueryInterface(const char** iid)
{
    const char* name = *iid;
    if (name == kIidGridRouter) {
        return static_cast<IGridRouter*>(this);
    }
    if (name == kIidScriptObject) {
        return static_cast<IScriptObject*>(this);
    }
    if (name == kIidShutdownable) {
        return static_cast<IShutdownable*>(this);
    }
    if (name == kIidAbstract) {
        return static_cast<IAbstract*>(this);
    }
    return 0;
}

// RET 4. KdScene's faces first (ECX kept in EDX across the call); the
// IAbstract fallback after it cannot be reached, KdScene answers it.
// FUNCTION: VC1 0x004b0c80
void* KdGameLevel::QueryInterface(const char** iid)
{
    void* face = KdScene::QueryInterface(iid);
    if (face) {
        return face;
    }
    if (*iid == kIidAbstract) {
        return static_cast<IAbstract*>(this);
    }
    return 0;
}

// RET 4. KdScriptedObject's faces first; the IAbstract fallback after it is
// unreachable the same way as in KdGameLevel's.
// FUNCTION: VC1 0x004b2340
void* KdClassificatedRenderer::QueryInterface(const char** iid)
{
    void* face = KdScriptedObject::QueryInterface(iid);
    if (face) {
        return face;
    }
    if (*iid == kIidAbstract) {
        return static_cast<IAbstract*>(this);
    }
    return 0;
}

}
