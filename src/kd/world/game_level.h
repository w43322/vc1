#ifndef VC1_KD_WORLD_GAME_LEVEL_H
#define VC1_KD_WORLD_GAME_LEVEL_H

#include "kd/scene/scene.h"

namespace vc1 {

struct IFunctionalGroup;
struct Vector2;

// Inferred name. One entry of KdLayerMap.
// SIZE 0x08
struct KdLayerEntry
{
    int layer;      // 0x00  ITileObject::GetLayer value
    int groupIndex; // 0x04  index into KdScene::groups
};

// Inferred name. Sorted layer -> scene group index table on a KdFactory
// buffer (no vftable of its own).
// SIZE 0x10
struct KdLayerMap
{
    KdFactory items; // 0x00

    KdLayerEntry* Begin() const { return reinterpret_cast<KdLayerEntry*>(items.begin); }
    KdLayerEntry* End() const
    {
        return reinterpret_cast<KdLayerEntry*>(
            reinterpret_cast<unsigned char*>(items.begin) + items.size);
    }
    KdLayerEntry* LowerBound(const int& key);
    int& operator[](const int& key);
    void Reserve(unsigned count);
};

// Factory "GameLevel": the game's Scene. Objects with an ITileObject face
// whose layer has a SetDynamicLayer group join that group instead of the
// render group. Scripts query objects by classificator mask and pick them
// under the cursor through the camera. The constructor overwrites all nine
// KdScene vftables (the base layout is unchanged).
// SIZE 0xac
// VTABLE: VC1 0x004d0de8 vc1::IScriptObject
// VTABLE: VC1 0x004d0e08 vc1::IScriptLoadable
// VTABLE: VC1 0x004d0e18 vc1::IUpdateable
// VTABLE: VC1 0x004d0e30 vc1::IRenderable
// VTABLE: VC1 0x004d0e48 vc1::IVisible
// VTABLE: VC1 0x004d0e5c vc1::IInputHandler
// VTABLE: VC1 0x004d0e70 vc1::IIdentifiable
// VTABLE: VC1 0x004d0e88 vc1::KdComponentGroup
// VTABLE: VC1 0x004d0eb0 vc1::IAbstract
class KdGameLevel : public KdScene
{
public:
    KdLayerMap layers;    // 0x90
    int renderGroupIndex; // 0xa0  render group's index in groups
    int clientWidth;      // 0xa4  display client rect at construction
    int clientHeight;     // 0xa8

    KdGameLevel() throw();
    ~KdGameLevel() throw();
    void* QueryInterface(const char** iid);
    unsigned AddRef();
    unsigned Release();
    unsigned char GetMember(const char* name, SquirrelObject* out);
    unsigned char SetMember(void* name, void* value);
    const char* GetTypeName();
    void AddObject(void** object);
    void RemoveObject(void** object);
    bool LoadFromScript(SquirrelObject& table);
    void Update(unsigned sliceBit, int dt);
    int Shutdown();

    void SetDynamicLayer(int layer, const KdPtr<IFunctionalGroup>& group);
    KdRefList FindObjectsByMask(unsigned include, unsigned exclude);
    KdPtr<IUnknown> GetObjectAt(const Vector2& point, unsigned include, unsigned exclude);
};

typedef char Vc1KdGameLevelSizeCheck[sizeof(KdGameLevel) == 0xac ? 1 : -1];

void** CreateKdGameLevel(void** out);

// SYNTHETIC: VC1 0x00403ae0
// vc1::KdGameLevel::`scalar deleting destructor'

// SYNTHETIC: VC1 0x00404180
// ?QueryInterface@KdGameLevel@vc1@@WM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00404190
// ?Release@KdGameLevel@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x004041a0
// ?QueryInterface@KdGameLevel@vc1@@WCM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x004041b0
// ?QueryInterface@KdGameLevel@vc1@@WBA@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x004041c0
// ?Release@KdGameLevel@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x004041d0
// ?AddRef@KdGameLevel@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x004041e0
// ?Release@KdGameLevel@vc1@@WCM@AEIXZ
// SYNTHETIC: VC1 0x004041f0
// ?QueryInterface@KdGameLevel@vc1@@WFA@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00404200
// ?QueryInterface@KdGameLevel@vc1@@WBE@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00404210
// ?Release@KdGameLevel@vc1@@WBA@AEIXZ
// SYNTHETIC: VC1 0x00404220
// ?AddRef@KdGameLevel@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x00404230
// ??_EKdGameLevel@vc1@@WFA@AEPAXI@Z
// SYNTHETIC: VC1 0x00404240
// ?Release@KdGameLevel@vc1@@WFA@AEIXZ
// SYNTHETIC: VC1 0x00404250
// ?QueryInterface@KdGameLevel@vc1@@WBI@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00404260
// ?Release@KdGameLevel@vc1@@WBE@AEIXZ
// SYNTHETIC: VC1 0x00404270
// ?AddRef@KdGameLevel@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x00404280
// ?AddRef@KdGameLevel@vc1@@WCM@AEIXZ
// SYNTHETIC: VC1 0x00404290
// ??_EKdGameLevel@vc1@@WBI@AEPAXI@Z
// SYNTHETIC: VC1 0x004042a0
// ?Release@KdGameLevel@vc1@@WBI@AEIXZ
// SYNTHETIC: VC1 0x004042b0
// ?AddRef@KdGameLevel@vc1@@WBA@AEIXZ
// SYNTHETIC: VC1 0x004042c0
// ?AddRef@KdGameLevel@vc1@@WFA@AEIXZ
// SYNTHETIC: VC1 0x004042d0
// ?AddRef@KdGameLevel@vc1@@WBE@AEIXZ
// SYNTHETIC: VC1 0x004042e0
// ?QueryInterface@KdGameLevel@vc1@@W3AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x004042f0
// ?AddRef@KdGameLevel@vc1@@WBI@AEIXZ
// SYNTHETIC: VC1 0x00404300
// ?QueryInterface@KdGameLevel@vc1@@W7AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00404310
// ?Release@KdGameLevel@vc1@@W3AEIXZ

}

#endif
