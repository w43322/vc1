#ifndef VC1_KD_ITILE_LAYER_H
#define VC1_KD_ITILE_LAYER_H

#include "kd/factory.h"
#include "kd/iface/iid.h"
#include "kd/iface/iunknown.h"

namespace vc1 {

// Inferred names. One placed tile of a TileLayer: Write (0x004a2860) and
// ReadTiles (0x004a3db0) stream the fields in the order id, x, y, and
// BuildGrid (0x004a3a30) offsets the tile manager rect of id by (x, y).
// SIZE 0x06
struct Tile
{
    short x;  // 0x00
    short y;  // 0x02
    short id; // 0x04
};

typedef char Vc1TileSizeCheck[sizeof(Tile) == 0x06 ? 1 : -1];

// Inferred names (iid "ITileLayer", 0x004cfdec). Every mutator rebuilds the
// layer's spatial grid afterwards. Vtable 0x004d1e80 in TileLayer.
// SIZE 0x04
struct ITileLayer : IUnknown
{
    virtual KdArray<Tile>& GetTiles() = 0;          // vtable+0x0c
    virtual void InsertTile(const Tile& tile) = 0;  // vtable+0x10
    virtual void AppendTile(const Tile& tile) = 0;  // vtable+0x14
    virtual void RemoveTile(int index) = 0;         // vtable+0x18
    virtual void SetTileId(int index, short id) = 0; // vtable+0x1c
};

typedef char Vc1ITileLayerSizeCheck[sizeof(ITileLayer) == 0x04 ? 1 : -1];

template <>
struct KdIid<ITileLayer>
{
    static const char** Get() { return &g_iidTileLayer; }
};

}

#endif
