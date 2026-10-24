#ifndef VC1_KD_TILE_MANAGER_H
#define VC1_KD_TILE_MANAGER_H

#include "kd/factory.h"
#include "kd/script_instance.h"
#include "kd/iface/iabstract.h"
#include "kd/iface/icomponent_faces.h"
#include "kd/iface/igraphics.h"
#include "kd/iface/iloadable.h"
#include "kd/iface/irender_target.h"
#include "kd/iface/iscript_object.h"
#include "kd/iface/ishutdownable.h"
#include "kd/iface/itexture.h"
#include "bindings/rect.h"
#include "bindings/rectf.h"
#include "bindings/vector2.h"

namespace vc1 {

// Inferred names. One record of res/Tiles.bin: the tile's offset from its
// anchor and its size. The default constructor stores height, width, y, x
// (0x0041b8d0).
// SIZE 0x08
struct TileRect
{
    short x;      // 0x00
    short y;      // 0x02
    short width;  // 0x04
    short height; // 0x06

    TileRect() : x(0), y(0), width(0), height(0) {}
};

typedef char Vc1TileRectSizeCheck[sizeof(TileRect) == 0x08 ? 1 : -1];

// Inferred name. TileRect array in a KdFactory buffer (no vftable of its own).
// SIZE 0x10
struct TileRectList
{
    KdFactory items; // 0x00

    TileRect* Data() const { return reinterpret_cast<TileRect*>(items.begin); }
    void Reserve(unsigned count);
};

// Inferred name. A joint position written by GetJoint.
// SIZE 0x08
struct TilePoint
{
    int x; // 0x00
    int y; // 0x04
};

// Inferred names for the tile command flags: bits 0/1 mirror x/y, bit 2 hides
// the rest of the tile (cleared by a matching animation frame command). The
// game subclass adds 0x10000 (draw the highlight over each fragment) and
// 0x20000 (keep sub-pixel positions).
enum {
    kTileFlipX = 1,
    kTileFlipY = 2,
    kTileHidden = 4,
    kTileHighlight = 0x10000,
    kTileSubPixel = 0x20000
};

// Inferred name ("ITileManager" IID, the literal compared by QueryInterface
// 0x00460490). Every tile is a run of 16-bit commands in the code array: the
// low 6 bits are the opcode, opcode 0 keeps a 10-bit texture index in the
// upper bits.
// SIZE 0x04
struct ITileManager : IUnknown
{
    virtual Rect GetRect(int tile, unsigned flags) = 0;                         // vtable+0x0c
    virtual bool GetJoint(int tile, int joint, TilePoint* out, unsigned flags) = 0; // vtable+0x10
    virtual void Render(const KdPtr<IGraphics>& graphics, int tile, float x, float y,
        unsigned flags) = 0;                                                    // vtable+0x14
    virtual bool HitTest(int tile, int x, int y, unsigned flags) = 0;           // vtable+0x18
    virtual int GetTextureIndex(int tile) = 0;                                  // vtable+0x1c
    virtual KdArray<unsigned> GetFragments(int tile, unsigned flags) = 0;       // vtable+0x20
    virtual KdPtr<ITexture>& GetTexture(int index) = 0;                         // vtable+0x24
    virtual void SetTexture(int index, const KdPtr<ITexture>& texture) = 0;     // vtable+0x28
};

template <>
struct KdIid<ITileManager>
{
    static const char** Get() { return &g_iidTileManager; }
};

typedef char Vc1ITileManagerSizeCheck[sizeof(ITileManager) == 0x04 ? 1 : -1];

// Inferred name. Engine half of the "TileManager" component: the tile table
// from res/Tiles.bin (Load), up to 1024 textures that the script's
// "LoadTexture" member fills on first use, and a 1x1 render target for pixel
// exact hit tests. Constructor 0x00460ed0 (ESI=this), destructor 0x00460d70.
// SIZE 0x1068
// VTABLE: VC1 0x004d64ec vc1::ITileManager
// VTABLE: VC1 0x004d6518 vc1::IScriptObject
// VTABLE: VC1 0x004d6530 vc1::IScriptLoadable
// VTABLE: VC1 0x004d6540 vc1::IShutdownable
// VTABLE: VC1 0x004d6550 vc1::ILoadable
// VTABLE: VC1 0x004d6560 vc1::IAbstract
class KdTileManager : public ITileManager,
                      public IScriptObject,
                      public IScriptLoadable,
                      public IShutdownable,
                      public ILoadable,
                      public IAbstract
{
public:
    KdScriptInstance script;         // 0x1c
    KdRef<ITexture> textures[0x400]; // 0x34
    TileRectList rects;              // 0x1034
    KdArray<int> offsets;            // 0x1044  tile i is code[offsets[i], offsets[i + 1])
    KdArray<short> code;             // 0x1054
    KdRef<IRenderTarget> target;     // 0x1064

    KdTileManager();
    ~KdTileManager() throw();

    // The deleting destructors call free() directly.
    static void operator delete(void* p) { free(p); }

    void* QueryInterface(const char** iid);
    unsigned AddRef();
    unsigned Release();

    Rect GetRect(int tile, unsigned flags);
    bool GetJoint(int tile, int joint, TilePoint* out, unsigned flags);
    void Render(const KdPtr<IGraphics>& graphics, int tile, float x, float y, unsigned flags);
    bool HitTest(int tile, int x, int y, unsigned flags);
    int GetTextureIndex(int tile);
    KdArray<unsigned> GetFragments(int tile, unsigned flags);
    KdPtr<ITexture>& GetTexture(int index);
    void SetTexture(int index, const KdPtr<ITexture>& texture);

    unsigned char GetMember(const char* name, SquirrelObject* out);
    unsigned char SetMember(void* name, void* value);
    const char* GetTypeName();
    bool LoadFromScript(SquirrelObject& table);
    int Shutdown();
    unsigned char Load(const char* name);
};

typedef char Vc1KdTileManagerSizeCheck[sizeof(KdTileManager) == 0x1068 ? 1 : -1];

// SYNTHETIC: VC1 0x0041b500
// vc1::KdTileManager::`scalar deleting destructor'

// Inferred name. The game's TileManager (factory 0x004a1aa0, a singleton): adds
// a highlight drawn over every fragment of a flagged tile, an optional clip
// rectangle and the script helpers on top of KdTileManager.
// SIZE 0x1074
// VTABLE: VC1 0x004d1fa0 vc1::ITileManager
// VTABLE: VC1 0x004d1fcc vc1::IScriptObject
// VTABLE: VC1 0x004d1fe4 vc1::IScriptLoadable
// VTABLE: VC1 0x004d1ff4 vc1::IShutdownable
// VTABLE: VC1 0x004d2004 vc1::ILoadable
// VTABLE: VC1 0x004d2014 vc1::IAbstract
class VcTileManager : public KdTileManager
{
public:
    const RectF* clip;       // 0x1068  fragments outside are skipped
    unsigned highlightMode;  // 0x106c
    unsigned highlightColor; // 0x1070

    VcTileManager();
    ~VcTileManager() throw() {}

    void* QueryInterface(const char** iid);
    unsigned AddRef();
    unsigned Release();
    void Render(const KdPtr<IGraphics>& graphics, int tile, float x, float y, unsigned flags);
    unsigned char GetMember(const char* name, SquirrelObject* out);
    unsigned char SetMember(void* name, void* value);

    void ResetTextureUsage();
    void SetHighlightColor(unsigned color);
    void SetHighlightBlendMode(int mode); // int like IGraphics::SetMode (own dispatcher)
    Vector2 GetJointPosition(int tile, int joint);
};

typedef char Vc1VcTileManagerSizeCheck[sizeof(VcTileManager) == 0x1074 ? 1 : -1];

// SYNTHETIC: VC1 0x0040a430
// vc1::VcTileManager::`scalar deleting destructor'

}

#endif
