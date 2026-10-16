#ifndef VC1_KD_SCENE_H
#define VC1_KD_SCENE_H

#include "kd/iface/iabstract.h"
#include "kd/iface/icomponent_faces.h"
#include "kd/iface/iidentifiable.h"
#include "kd/iface/iinput_handler.h"
#include "kd/iface/irenderable.h"
#include "kd/iface/iscript_object.h"
#include "kd/iface/iupdateable.h"
#include "kd/component_group.h"
#include "kd/factory.h"
#include "kd/script_instance.h"
#include "bindings/squirrel_object.h"

namespace vc1 {

// Inferred names. Never assigned; the "no object" values the scene compares
// group and identifier faces against (by IAbstract identity).
extern KdRef<IUnknown> g_kdNullSceneObject;
extern KdRef<IUnknown> g_kdNullSceneIdentifier;

extern const char kSystemObjectExists[];
extern const char kSystemObjectNotFound[];

// Factory "Scene" and the base of the game's GameLevel: a named component
// group with a script instance whose objects also join the scene's own
// functional groups (initially the system's UpdateGroup, RenderGroup and
// InputGroup; scripts append, remove and toggle groups by name). The update,
// render, visibility and input faces forward to those three groups.
// QI 0x0046e430 establishes the face offsets below; IComponentGroup also
// answers IShutdownable.
// SIZE 0x90
// VTABLE: VC1 0x004d5464 vc1::IScriptObject
// VTABLE: VC1 0x004d5484 vc1::IScriptLoadable
// VTABLE: VC1 0x004d5494 vc1::IUpdateable
// VTABLE: VC1 0x004d54ac vc1::IRenderable
// VTABLE: VC1 0x004d54c4 vc1::IVisible
// VTABLE: VC1 0x004d54d8 vc1::IInputHandler
// VTABLE: VC1 0x004d54ec vc1::IIdentifiable
// VTABLE: VC1 0x004d5504 vc1::KdComponentGroup
// VTABLE: VC1 0x004d552c vc1::IAbstract
class KdScene : public IScriptObject,
                public IScriptLoadable,
                public IUpdateable,
                public IRenderable,
                public IVisible,
                public IInputHandler,
                public IIdentifiable,
                public KdComponentGroup,
                public IAbstract
{
public:
    KdScriptInstance script;          // 0x58
    KdRef<IUpdateable> updateGroup;   // 0x70
    KdRef<IRenderable> renderGroup;   // 0x74
    KdRef<IInputHandler> inputGroup;  // 0x78
    KdRefList groups;                 // 0x7c  IFunctionalGroup faces
    unsigned groupMask;               // 0x8c  bit i: objects join groups[i]

    KdScene() throw();
    ~KdScene() throw();
    void* QueryInterface(const char** iid);
    unsigned AddRef();
    unsigned Release();
    unsigned char GetMember(const char* name, SquirrelObject* out);
    unsigned char SetMember(void* name, void* value);
    const char* GetTypeName();
    // Inferred names. Primary vftable +0x18 / +0x1c: the object and, through
    // its component group, every child joins / leaves the enabled groups.
    virtual void AddObject(void** object);
    virtual void RemoveObject(void** object);
    bool LoadFromScript(SquirrelObject& table);

    int GetUpdateLayer();
    int SetLayer(unsigned layer);
    void Update(unsigned sliceBit, int dt);
    int RenderMasked(void* graphics, void* mask);
    int Render(void* graphics);
    int GetRenderOrder();
    void SetVisible(bool visible);
    bool IsVisible();
    unsigned char OnKey(const unsigned* data);
    unsigned char OnPointer(const unsigned* data);

    int Shutdown();
    void RegisterObject(const KdString& name, const KdPtr<IAbstract>& object);
    KdPtr<IUnknown> UnregisterObject(const KdString& name);
    KdPtr<IUnknown> FindObject(const KdString& name);

    void EnableGroup(const KdString& name, bool enable);
    void AppendGroup(const KdPtr<IAbstract>& group);
    void RemoveGroup(const KdPtr<IAbstract>& group);
};

typedef char Vc1KdSceneSizeCheck[sizeof(KdScene) == 0x90 ? 1 : -1];

int FindSceneGroup(KdScene* scene, const KdString& name);
void** CreateKdScene(void** out);

// SYNTHETIC: VC1 0x00417450
// vc1::KdScene::`scalar deleting destructor'

}

#endif
