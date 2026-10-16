#ifndef VC1_KD_ROAD_PASS_MAP_H
#define VC1_KD_ROAD_PASS_MAP_H

#include "kd/factory.h"
#include "kd/script_instance.h"
#include "kd/iface/iabstract.h"
#include "kd/iface/icomponent_faces.h"
#include "kd/iface/igrid_map.h"
#include "kd/iface/iidentifiable.h"
#include "kd/iface/irenderable.h"
#include "kd/iface/iscript_object.h"
#include "kd/iface/ishutdownable.h"
#include "bindings/matrix3.h"

class SquirrelObject;

namespace vc1 {

// Inferred name. The cell kinds PassMap reads from its script table
// (CPassMap in Level.nut) by these member names; not initialized by the
// constructor.
// SIZE 0x34
struct KdPassMapCellValues
{
    int clean;              // 0x00 "CELL_VALUE_Clean"
    int occupied;           // 0x04 "CELL_VALUE_Occupied"
    int plant;              // 0x08 "CELL_VALUE_Plant"
    int roadBroken;         // 0x0c "CELL_VALUE_RoadBroken"
    int road;               // 0x10 "CELL_VALUE_Road"
    int roadIndestructible; // 0x14 "CELL_VALUE_RoadIndestructible"
    int roadConstruction;   // 0x18 "CELL_VALUE_RoadConstruction"
    int forestMin;          // 0x1c "CELL_VALUE_ForestMin"
    int forestMax;          // 0x20 "CELL_VALUE_ForestMax"
    int forestBurnt;        // 0x24 "CELL_VALUE_ForestBurnt"
    int forestCutted;       // 0x28 "CELL_VALUE_ForestCutted"
    int waste;              // 0x2c "CELL_VALUE_Waste"
    int roadInvisible;      // 0x30 "CELL_VALUE_RoadInvisible"
};

typedef char Vc1KdPassMapCellValuesSizeCheck[sizeof(KdPassMapCellValues) == 0x34 ? 1 : -1];

// Inferred name. Factory "PassMap": the byte grid of cell kinds of a level
// map (one cell per map tile, placed by the IPositionable matrix). Script
// "SetCellValue" calls the script member "OnCellChange"(self, x, y, value);
// the IGridRenderer layers (roads, grid rects) draw on top of it in Render.
// QI 0x004ac8e0 establishes the face offsets below.
// SIZE 0xd8
// VTABLE: VC1 0x004d14c4 vc1::IScriptObject
// VTABLE: VC1 0x004d14dc vc1::IScriptLoadable
// VTABLE: VC1 0x004d14ec vc1::IShutdownable
// VTABLE: VC1 0x004d14fc vc1::IReadable
// VTABLE: VC1 0x004d150c vc1::IWriteable
// VTABLE: VC1 0x004d151c vc1::IRenderable
// VTABLE: VC1 0x004d1534 vc1::IVisible
// VTABLE: VC1 0x004d1548 vc1::IGridMap
// VTABLE: VC1 0x004d1574 vc1::IIdentifiable
// VTABLE: VC1 0x004d158c vc1::IAbstract
class KdPassMap : public IScriptObject,
                  public IScriptLoadable,
                  public IShutdownable,
                  public IReadable,
                  public IWriteable,
                  public IRenderable,
                  public IVisible,
                  public IGridMap,
                  public KdPassMapCellValues,
                  public IIdentifiable,
                  public IAbstract
{
public:
    KdScriptInstance script;     // 0x70
    KdArray<unsigned char> cells; // 0x88 width*height, row-major
    short width;                 // 0x98
    short height;                // 0x9a
    Matrix3 position;            // 0x9c
    bool visible;                // 0xc0
    int renderLayer;             // 0xc4 root "LAYER_PassMap"
    KdRefList renderers;         // 0xc8 IGridRenderer faces

    KdPassMap() throw();
    ~KdPassMap() throw();
    void* QueryInterface(const char** iid);
    unsigned AddRef();
    unsigned Release();

    unsigned char GetMember(const char* name, SquirrelObject* out);
    unsigned char SetMember(void* name, void* value);
    const char* GetTypeName();

    bool LoadFromScript(SquirrelObject& table);
    int Shutdown();
    bool ReadFrom(const KdPtr<IInputStream>& stream);
    bool WriteTo(const KdPtr<IOutputStream>& stream);

    int Render(void* graphics);
    int GetRenderOrder();

    void SetVisible(bool value);
    bool IsVisible();

    void SetPosition(const Matrix3& value);
    const Matrix3& GetPosition();
    int GetWidth();
    int GetHeight();
    int GetCellValue(int x, int y);
    void SetCellValue(int x, int y, int value);
    void AddRenderer(const KdPtr<IGridRenderer>& renderer);
    void RemoveRenderer(const KdPtr<IGridRenderer>& renderer);
};

typedef char Vc1KdPassMapSizeCheck[sizeof(KdPassMap) == 0xd8 ? 1 : -1];

void** CreateKdPassMap(void** out);

// TEMPLATE: VC1 0x00405a20
// ?Resize@?$KdArray@E@vc1@@QAEXI@Z

// SYNTHETIC: VC1 0x00405a00
// vc1::KdPassMap::`scalar deleting destructor'

// SYNTHETIC: VC1 0x00405ea0
// ?Release@KdPassMap@vc1@@WGI@AEIXZ
// SYNTHETIC: VC1 0x00405eb0
// ?QueryInterface@KdPassMap@vc1@@WGI@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00405ec0
// ?Release@KdPassMap@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x00405ed0
// ?QueryInterface@KdPassMap@vc1@@WM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00405ee0
// ?AddRef@KdPassMap@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x00405ef0
// ??_EKdPassMap@vc1@@WGI@AEPAXI@Z
// SYNTHETIC: VC1 0x00405f00
// ?Release@KdPassMap@vc1@@WBA@AEIXZ
// SYNTHETIC: VC1 0x00405f10
// ?QueryInterface@KdPassMap@vc1@@WBA@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00405f20
// ?AddRef@KdPassMap@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x00405f50
// ?AddRef@KdPassMap@vc1@@WGI@AEIXZ
// SYNTHETIC: VC1 0x00405f60
// ?Release@KdPassMap@vc1@@WBE@AEIXZ
// SYNTHETIC: VC1 0x00405f70
// ?QueryInterface@KdPassMap@vc1@@WBE@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00405f80
// ?AddRef@KdPassMap@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x00405fa0
// ?Release@KdPassMap@vc1@@WFE@AEIXZ
// SYNTHETIC: VC1 0x00405fb0
// ?QueryInterface@KdPassMap@vc1@@WFE@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00405fc0
// ?Release@KdPassMap@vc1@@WBI@AEIXZ
// SYNTHETIC: VC1 0x00405fd0
// ?QueryInterface@KdPassMap@vc1@@WBI@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00405fe0
// ?AddRef@KdPassMap@vc1@@WBA@AEIXZ
// SYNTHETIC: VC1 0x00405ff0
// ??_EKdPassMap@vc1@@WFE@AEPAXI@Z
// SYNTHETIC: VC1 0x00406000
// ?Release@KdPassMap@vc1@@WBM@AEIXZ
// SYNTHETIC: VC1 0x00406010
// ?QueryInterface@KdPassMap@vc1@@WBM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00406020
// ?AddRef@KdPassMap@vc1@@WBE@AEIXZ
// SYNTHETIC: VC1 0x00406030
// ?AddRef@KdPassMap@vc1@@WFE@AEIXZ
// SYNTHETIC: VC1 0x00406040
// ?AddRef@KdPassMap@vc1@@WBI@AEIXZ
// SYNTHETIC: VC1 0x00406050
// ?Release@KdPassMap@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x00406060
// ?QueryInterface@KdPassMap@vc1@@W3AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00406070
// ?AddRef@KdPassMap@vc1@@WBM@AEIXZ
// SYNTHETIC: VC1 0x00406080
// ?Release@KdPassMap@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x00406090
// ?QueryInterface@KdPassMap@vc1@@W7AEPAXPAPBD@Z

}

#endif
