#ifndef VC1_KD_TILE_OBJECT_H
#define VC1_KD_TILE_OBJECT_H

#include "kd/factory.h"
#include "kd/iface/iabstract.h"
#include "kd/iface/iclassifiable.h"
#include "kd/iface/icomponent_faces.h"
#include "kd/iface/iidentifiable.h"
#include "kd/iface/ipositionable.h"
#include "kd/iface/iscript_object.h"
#include "kd/iface/ishutdownable.h"
#include "kd/iface/itile_object.h"
#include "bindings/matrix3.h"
#include "bindings/rect.h"
#include "bindings/rectf.h"

class SquirrelObject;

namespace vc1 {

struct ICamera;
struct Vector2;
class KdTileObject;

// Inferred name. One lazily created joint follower of a KdTileObject.
// SIZE 0x08
struct KdTileJoint
{
    int joint;        // 0x00 TileManager joint index
    IUnknown* object; // 0x04 counted KdTileJointObject (IAbstract face)
};

// Inferred name. Result of KdTileJointMap::Insert.
struct KdTileJointInsert
{
    KdTileJoint* slot;
    bool inserted;
};

// Inferred name. Joint -> follower table sorted by joint on a KdFactory
// buffer. LowerBound and Reserve fold with KdLayerMap's (same 8-byte
// records keyed by the first int).
// SIZE 0x10
struct KdTileJointMap
{
    KdFactory items; // 0x00

    ~KdTileJointMap() throw()
    {
        if (items.capacity) {
            DestroyRange(Begin(), Begin() + items.capacity / sizeof(KdTileJoint));
        }
    }
    KdTileJoint* Begin() const { return reinterpret_cast<KdTileJoint*>(items.begin); }
    KdTileJoint* End() const
    {
        return reinterpret_cast<KdTileJoint*>(
            reinterpret_cast<unsigned char*>(items.begin) + items.size);
    }
    unsigned Count() const { return items.size / sizeof(KdTileJoint); }
    KdTileJoint* Find(const int& key)
    {
        KdTileJoint* const entry = LowerBound(key);
        if (entry < End() && !(key < entry->joint)) {
            return entry;
        }
        return End();
    }
    KdTileJoint* LowerBound(const int& key);
    void Reserve(unsigned count);
    KdTileJoint* InsertAt(KdTileJoint* pos, const KdTileJoint& entry);
    KdTileJointInsert Insert(const KdTileJoint& entry);
    static void DestroyRange(KdTileJoint* first, KdTileJoint* last);
};

// Inferred name. Factory "TileObject": one TileManager tile drawn at the
// translation of an IPositionable (SetPositionable), in its render layer of
// a TileLayer / GameLevel, tinted and optionally clipped to a rect. GetJoint
// hands out IPositionable followers of the tile's joint points. QI
// 0x004a0300 establishes the face offsets below.
// SIZE 0x74
// VTABLE: VC1 0x004d20b0 vc1::ITileObject
// VTABLE: VC1 0x004d20e8 vc1::IPositionableUser
// VTABLE: VC1 0x004d20f8 vc1::IScriptObject
// VTABLE: VC1 0x004d2110 vc1::IVisible
// VTABLE: VC1 0x004d2124 vc1::IBoundingInfo
// VTABLE: VC1 0x004d2134 vc1::IShutdownable
// VTABLE: VC1 0x004d2144 vc1::IIdentifiable
// VTABLE: VC1 0x004d215c vc1::IClassifiable
// VTABLE: VC1 0x004d2178 vc1::IAbstract
class KdTileObject : public ITileObject,
                     public IPositionableUser,
                     public IScriptObject,
                     public IVisible,
                     public IBoundingInfo,
                     public IShutdownable,
                     public IIdentifiable,
                     public IClassifiable,
                     public IAbstract
{
public:
    KdRef<IPositionable> positionable; // 0x3c
    KdTileJointMap joints;             // 0x40
    int tile;                          // 0x50 -1: none
    int layer;                         // 0x54 render layer
    int color;                         // 0x58 ARGB
    unsigned flags;                    // 0x5c TileManager render flags
    bool visible;                      // 0x60
    Rect clip;                         // 0x64 UI rect; empty: no clipping

    KdTileObject() throw();
    ~KdTileObject() throw();
    void* QueryInterface(const char** iid);
    unsigned AddRef();
    unsigned Release();

    int RenderMasked(void* graphics, void* mask);
    int Render(void* graphics);
    int GetRenderOrder();
    int GetTile();
    void SetTile(int value);
    void SetRenderLayer(int value);
    void SetColor(int value);
    void SetHighlighted(bool value);
    void EnableFloatCoords(bool value);
    float GetDepth(IUnknown** camera);
    KdPtr<IUnknown> GetJoint(int joint);

    void SetPositionable(const KdPtr<IPositionable>& source);

    unsigned char GetMember(const char* name, SquirrelObject* out);
    unsigned char SetMember(void* name, void* value);
    const char* GetTypeName();

    void SetVisible(bool value);
    bool IsVisible();

    const RectF& GetBoundingRect(const RenderMask& mask);

    int Shutdown();

    void SetClip(const Rect& rect);
    bool HitTest(const Vector2& point, const KdPtr<ICamera>& camera, bool precise);
};

typedef char Vc1KdTileObjectSizeCheck[sizeof(KdTileObject) == 0x74 ? 1 : -1];

// Inferred name. GetJoint's follower: an IPositionable whose translation is
// the owner's position moved by the owner tile's joint point; SetPosition is
// ignored. QI 0x004a0250 establishes the face offsets below.
// SIZE 0x3c
// VTABLE: VC1 0x004d2188 vc1::IPositionable
// VTABLE: VC1 0x004d219c vc1::IShutdownable
// VTABLE: VC1 0x004d21ac vc1::IAbstract
class KdTileJointObject : public IPositionable, public IShutdownable, public IAbstract
{
public:
    Matrix3 position;          // 0x10
    KdRef<KdTileObject> owner; // 0x34
    int joint;                 // 0x38

    KdTileJointObject(KdTileObject* source, int index) throw();
    ~KdTileJointObject() throw();
    void* QueryInterface(const char** iid);
    unsigned AddRef();
    unsigned Release();

    void SetPosition(const Matrix3& value);
    const Matrix3& GetPosition();
    int Shutdown();
};

typedef char Vc1KdTileJointObjectSizeCheck[sizeof(KdTileJointObject) == 0x3c ? 1 : -1];

void** CreateKdTileObject(void** out);

// SYNTHETIC: VC1 0x0040a9c0
// vc1::KdTileObject::`scalar deleting destructor'
// SYNTHETIC: VC1 0x0040a9e0
// vc1::KdTileJointObject::`scalar deleting destructor'

// TEMPLATE: VC1 0x0040abb0
// ??$CreateScriptMethod@VKdTileObject@vc1@@P812@AEHXZ@vc1@@YA?AVSquirrelObject@@P8KdTileObject@0@AEHXZ@Z
// TEMPLATE: VC1 0x0040ac20
// ??$CreateScriptMethod@VKdTileObject@vc1@@P812@AEXH@Z@vc1@@YA?AVSquirrelObject@@P8KdTileObject@0@AEXH@Z@Z
// TEMPLATE: VC1 0x0040ac90
// ??$CreateScriptMethod@VKdTileObject@vc1@@P812@AEX_N@Z@vc1@@YA?AVSquirrelObject@@P8KdTileObject@0@AEX_N@Z@Z
// TEMPLATE: VC1 0x0040ad00
// ??$CreateScriptMethod@VKdTileObject@vc1@@P812@AE?AV?$KdPtr@UIUnknown@vc1@@@2@H@Z@vc1@@YA?AVSquirrelObject@@P8KdTileObject@0@AE?AV?$KdPtr@UIUnknown@vc1@@@0@H@Z@Z
// TEMPLATE: VC1 0x0040ad70
// ??$CreateScriptMethod@VKdTileObject@vc1@@P812@AEXABURect@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdTileObject@0@AEXABURect@0@@Z@Z
// TEMPLATE: VC1 0x0040ade0
// ??$CreateScriptMethod@VKdTileObject@vc1@@P812@AE_NABUVector2@2@ABV?$KdPtr@UICamera@vc1@@@2@_N@Z@vc1@@YA?AVSquirrelObject@@P8KdTileObject@0@AE_NABUVector2@0@ABV?$KdPtr@UICamera@vc1@@@0@_N@Z@Z
// TEMPLATE: VC1 0x0040aee0
// ??$Call@VKdTileObject@vc1@@H@?$ReturnSpecialization@V?$KdPtr@UIUnknown@vc1@@@vc1@@@SqPlus@@SAHAAVKdTileObject@vc1@@P823@AE?AV?$KdPtr@UIUnknown@vc1@@@3@H@ZPAUSQVM@@H@Z
// TEMPLATE: VC1 0x0040af60
// ??$Call@VKdTileObject@vc1@@ABUVector2@2@ABV?$KdPtr@UICamera@vc1@@@2@_N@?$ReturnSpecialization@_N@SqPlus@@SAHAAVKdTileObject@vc1@@P823@AE_NABUVector2@3@ABV?$KdPtr@UICamera@vc1@@@3@_N@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x004a0760
// ?Dispatch@?$ScriptMethodCall@VKdTileObject@vc1@@P812@AE_NABUVector2@2@ABV?$KdPtr@UICamera@vc1@@@2@_N@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004a0800
// ?Dispatch@?$ScriptMethodCall@VKdTileObject@vc1@@P812@AEXABURect@2@@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004a08c0
// ?Dispatch@?$ScriptMethodCall@VKdTileObject@vc1@@P812@AEX_N@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004a0990
// ?Dispatch@?$ScriptMethodCall@VKdTileObject@vc1@@P812@AEXH@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004a0aa0
// ?Dispatch@?$ScriptMethodCall@VKdTileObject@vc1@@P812@AEHXZ@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004a0b70
// ?Dispatch@?$ScriptMethodCall@VKdTileObject@vc1@@P812@AE?AV?$KdPtr@UIUnknown@vc1@@@2@H@Z@vc1@@SAHPAUSQVM@@@Z

// SYNTHETIC: VC1 0x0040b0d0
// ?QueryInterface@KdTileObject@vc1@@WBA@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0040b0e0
// ?Release@KdTileObject@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x0040b0f0
// ?AddRef@KdTileObject@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x0040b100
// ?Release@KdTileObject@vc1@@WCM@AEIXZ
// SYNTHETIC: VC1 0x0040b110
// ??_EKdTileObject@vc1@@WCM@AEPAXI@Z
// SYNTHETIC: VC1 0x0040b120
// ?QueryInterface@KdTileObject@vc1@@WBE@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0040b130
// ?Release@KdTileObject@vc1@@WBA@AEIXZ
// SYNTHETIC: VC1 0x0040b140
// ?AddRef@KdTileObject@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x0040b150
// ?QueryInterface@KdTileObject@vc1@@WDE@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0040b160
// ?QueryInterface@KdTileObject@vc1@@WBI@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0040b170
// ?Release@KdTileObject@vc1@@WBE@AEIXZ
// SYNTHETIC: VC1 0x0040b180
// ?AddRef@KdTileObject@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x0040b190
// ?Release@KdTileObject@vc1@@WDE@AEIXZ
// SYNTHETIC: VC1 0x0040b1a0
// ?AddRef@KdTileObject@vc1@@WCM@AEIXZ
// SYNTHETIC: VC1 0x0040b1b0
// ?QueryInterface@KdTileJointObject@vc1@@W3AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0040b1c0
// ??_EKdTileObject@vc1@@WDE@AEPAXI@Z
// SYNTHETIC: VC1 0x0040b1d0
// ?Release@KdTileObject@vc1@@WBI@AEIXZ
// SYNTHETIC: VC1 0x0040b1e0
// ?AddRef@KdTileObject@vc1@@WBA@AEIXZ
// SYNTHETIC: VC1 0x0040b1f0
// ?QueryInterface@KdTileJointObject@vc1@@W7AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0040b200
// ?Release@KdTileJointObject@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x0040b210
// ??_EKdTileObject@vc1@@WBI@AEPAXI@Z
// SYNTHETIC: VC1 0x0040b220
// ?AddRef@KdTileObject@vc1@@WBE@AEIXZ
// SYNTHETIC: VC1 0x0040b230
// ?AddRef@KdTileObject@vc1@@WDE@AEIXZ
// SYNTHETIC: VC1 0x0040b240
// ?Release@KdTileJointObject@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x0040b250
// ?QueryInterface@KdTileObject@vc1@@W3AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0040b260
// ?AddRef@KdTileObject@vc1@@WBI@AEIXZ
// SYNTHETIC: VC1 0x0040b270
// ??_EKdTileJointObject@vc1@@W7AEPAXI@Z
// SYNTHETIC: VC1 0x0040b280
// ?AddRef@KdTileJointObject@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x0040b290
// ?QueryInterface@KdTileObject@vc1@@W7AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0040b2a0
// ?Release@KdTileObject@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x0040b2b0
// ?AddRef@KdTileJointObject@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x0040b2c0
// ?QueryInterface@KdTileObject@vc1@@WM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0040b2d0
// ?Release@KdTileObject@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x0040b2e0
// ?QueryInterface@KdTileObject@vc1@@WCM@AEPAXPAPBD@Z

}

#endif
