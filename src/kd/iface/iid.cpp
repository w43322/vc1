#include "kd/iface/iid.h"
#include "kd/scene/compound_object.h"

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

// STRING: VC1 0x004d4d18
static const char kIidScriptObject[] = "IScriptObject";
// GLOBAL: VC1 0x004d05f4
const char* g_iidScriptObject = kIidScriptObject;

// GLOBAL: VC1 0x004d058c
// STRING: VC1 0x004d4e7c
const char* g_iidOutputStream = "IOutputStream";
// GLOBAL: VC1 0x004d05ac
// STRING: VC1 0x004d4e10
const char* g_iidReadable = "IReadable";
// GLOBAL: VC1 0x004d05b0
// STRING: VC1 0x004d4e04
const char* g_iidWriteable = "IWriteable";
// GLOBAL: VC1 0x004d05b8
// STRING: VC1 0x004d4dec
const char* g_iidSaveable = "ISaveable";
// GLOBAL: VC1 0x004d05dc
// STRING: VC1 0x004d4d74
const char* g_iidWithLifeTime = "IWithLifeTime";
// GLOBAL: VC1 0x004d05e4
const char* g_iidClassifiable = kIidClassifiable;
// GLOBAL: VC1 0x004d05e8
// STRING: VC1 0x004d4d4c
const char* g_iidVisible = "IVisible";
// GLOBAL: VC1 0x004d05f0
// STRING: VC1 0x004d4d28
const char* g_iidManualControl = "IManualControl";
// GLOBAL: VC1 0x004d05f8
const char* g_iidScriptLoadable = kIidScriptLoadable;
// GLOBAL: VC1 0x004d05fc
// STRING: VC1 0x004d4cf8
const char* g_iidScriptSaveable = "IScriptSaveable";
// GLOBAL: VC1 0x004d0600
const char* g_iidPositionable = kIidPositionable;
// GLOBAL: VC1 0x004d0604
// STRING: VC1 0x004d4cd4
const char* g_iidPositionableUser = "IPositionableUser";
// GLOBAL: VC1 0x004d0608
// STRING: VC1 0x004d4cc4
const char* g_iidBoundingInfo = "IBoundingInfo";
// GLOBAL: VC1 0x004d0610
// STRING: VC1 0x004d4cac
const char* g_iidInputHandler = "IInputHandler";
// GLOBAL: VC1 0x004d0614
// STRING: VC1 0x004d4c9c
const char* g_iidObjectControl = "IObjectControl";
// GLOBAL: VC1 0x004d0620
// STRING: VC1 0x004d4c74
const char* g_iidAnimateable = "IAnimateable";

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
        return static_cast<ICloneable*>(this);
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

}
