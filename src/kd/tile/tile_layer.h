#ifndef VC1_KD_TILE_LAYER_H
#define VC1_KD_TILE_LAYER_H

#include "kd/factory.h"
#include "kd/name_map.h"
#include "kd/iface/iabstract.h"
#include "kd/iface/iclassifiable.h"
#include "kd/iface/icomponent_faces.h"
#include "kd/iface/ifunctional_group.h"
#include "kd/iface/iidentifiable.h"
#include "kd/iface/irenderable.h"
#include "kd/iface/iscript_object.h"
#include "kd/iface/itile_layer.h"
#include "bindings/matrix4.h"
#include "bindings/rect.h"

class SquirrelObject;

namespace vc1 {

// Inferred name. Insert result of a KdArray<int> used as a sorted set.
// SIZE 0x08
struct KdIntSetInsert
{
    int* position;          // 0x00
    unsigned char inserted; // 0x04
};

// Inferred names. TileLayer's spatial index: the union of all tile rects,
// cut into 128x128 cells that each hold the addresses of the overlapping
// tiles (as a sorted set), plus the result set of the last Query.
// Destructor 0x00409a90 (EAX=this).
// SIZE 0x38
struct KdTileGrid
{
    Rect bounds;                     // 0x00
    int columns;                     // 0x10
    int rows;                        // 0x14
    KdObjArray<KdArray<int> > cells; // 0x18 columns*rows, row-major
    KdArray<int> visible;            // 0x28 Query result, sorted Tile*

    KdTileGrid() : columns(0), rows(0) {}
    ~KdTileGrid() throw();
    void Build(const KdArray<Tile>& tiles);
    KdArray<int>* Query(int left, int top, int right, int bottom);
};

typedef char Vc1KdTileGridSizeCheck[sizeof(KdTileGrid) == 0x38 ? 1 : -1];

// Inferred name. Factory "TileLayer": one layer of placed tiles of a level
// map (Level.nut LoadMap fills it with "ReadTiles"), drawn through the
// "TileManager" component with an optional parallax back tile, and the
// tile objects of the same render layer drawn in between the tiles by
// depth. QI 0x004a2750 establishes the face offsets below.
// SIZE 0xe0
// VTABLE: VC1 0x004d1e80 vc1::ITileLayer
// VTABLE: VC1 0x004d1ea0 vc1::IReadable
// VTABLE: VC1 0x004d1eb0 vc1::IWriteable
// VTABLE: VC1 0x004d1ec0 vc1::IRenderable
// VTABLE: VC1 0x004d1ed8 vc1::IFunctionalGroup
// VTABLE: VC1 0x004d1ef0 vc1::IScriptObject
// VTABLE: VC1 0x004d1f08 vc1::IIdentifiable
// VTABLE: VC1 0x004d1f20 vc1::IClassifiable
// VTABLE: VC1 0x004d1f3c vc1::IAbstract
class KdTileLayer : public ITileLayer,
                    public IReadable,
                    public IWriteable,
                    public IRenderable,
                    public IFunctionalGroup,
                    public IScriptObject,
                    public IIdentifiable,
                    public IClassifiable,
                    public IAbstract
{
public:
    KdArray<Tile> tiles;   // 0x3c
    Matrix4 transform;     // 0x4c identity; not read by the recovered code
    int renderLayer;       // 0x8c
    KdRefList objects;     // 0x90 ITileObject faces
    short backTile;        // 0xa0 -1: none
    int backParallax;      // 0xa4 right shift of the view origin
    KdTileGrid grid;       // 0xa8

    KdTileLayer() throw();
    ~KdTileLayer() throw();
    void* QueryInterface(const char** iid);
    unsigned AddRef();
    unsigned Release();

    KdArray<Tile>& GetTiles();
    void InsertTile(const Tile& tile);
    void AppendTile(const Tile& tile);
    void RemoveTile(int index);
    void SetTileId(int index, short id);

    bool ReadFrom(const KdPtr<IInputStream>& stream);
    bool WriteTo(const KdPtr<IOutputStream>& stream);

    int RenderMasked(void* graphics, void* mask);
    int Render(void* graphics);
    int GetRenderOrder();

    unsigned char Add(const KdPtr<IAbstract>& object);
    void Remove(const KdPtr<IAbstract>& object);
    void Clear();

    unsigned char GetMember(const char* name, SquirrelObject* out);
    unsigned char SetMember(void* name, void* value);
    const char* GetTypeName();

    bool ReadTiles(const KdPtr<IAbstract>& stream, int count);
    void SetRenderLayer(int layer);
    void SetBackTile(int tile); // int: shares the void(int) closure with SetRenderLayer
    void SetBackParallax(int shift);
};

typedef char Vc1KdTileLayerSizeCheck[sizeof(KdTileLayer) == 0xe0 ? 1 : -1];

void** CreateKdTileLayer(void** out);

// TEMPLATE: VC1 0x004047e0
// ?Reserve@?$KdObjArray@URect@vc1@@@vc1@@QAEXIE@Z

// SYNTHETIC: VC1 0x00409a70
// vc1::KdTileLayer::`scalar deleting destructor'

// SYNTHETIC: VC1 0x0040a1b0
// ?Release@KdTileLayer@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x0040a1c0
// ?QueryInterface@KdTileLayer@vc1@@W7AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0040a1d0
// ?Release@KdTileLayer@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x0040a1e0
// ?QueryInterface@KdTileLayer@vc1@@WM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0040a1f0
// ?AddRef@KdTileLayer@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x0040a200
// ?Release@KdTileLayer@vc1@@WCM@AEIXZ
// SYNTHETIC: VC1 0x0040a210
// ?QueryInterface@KdTileLayer@vc1@@WCM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0040a220
// ??_EKdTileLayer@vc1@@WCM@AEPAXI@Z
// SYNTHETIC: VC1 0x0040a230
// ?Release@KdTileLayer@vc1@@WBA@AEIXZ
// SYNTHETIC: VC1 0x0040a240
// ?QueryInterface@KdTileLayer@vc1@@WBA@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0040a250
// ?AddRef@KdTileLayer@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x0040a260
// ?Release@KdTileLayer@vc1@@WBE@AEIXZ
// SYNTHETIC: VC1 0x0040a270
// ?QueryInterface@KdTileLayer@vc1@@WBE@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0040a280
// ?AddRef@KdTileLayer@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x0040a290
// ?Release@KdTileLayer@vc1@@WDE@AEIXZ
// SYNTHETIC: VC1 0x0040a2a0
// ?QueryInterface@KdTileLayer@vc1@@WDE@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0040a2b0
// ?AddRef@KdTileLayer@vc1@@WCM@AEIXZ
// SYNTHETIC: VC1 0x0040a2c0
// ??_EKdTileLayer@vc1@@WDE@AEPAXI@Z
// SYNTHETIC: VC1 0x0040a2d0
// ?Release@KdTileLayer@vc1@@WBI@AEIXZ
// SYNTHETIC: VC1 0x0040a2e0
// ?QueryInterface@KdTileLayer@vc1@@WBI@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0040a2f0
// ?AddRef@KdTileLayer@vc1@@WBA@AEIXZ
// SYNTHETIC: VC1 0x0040a300
// ??_EKdTileLayer@vc1@@WBI@AEPAXI@Z
// SYNTHETIC: VC1 0x0040a310
// ?AddRef@KdTileLayer@vc1@@WBE@AEIXZ
// SYNTHETIC: VC1 0x0040a320
// ?AddRef@KdTileLayer@vc1@@WDE@AEIXZ
// SYNTHETIC: VC1 0x0040a330
// ?AddRef@KdTileLayer@vc1@@WBI@AEIXZ
// SYNTHETIC: VC1 0x0040a340
// ?Release@KdTileLayer@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x0040a350
// ?QueryInterface@KdTileLayer@vc1@@W3AEPAXPAPBD@Z

}

#endif
