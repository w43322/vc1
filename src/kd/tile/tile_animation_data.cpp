#include "kd/tile/tile_animation_data.h"

#include "bindings/sqplus.h"
#include "kd/factory.h"
#include "kd/iface/iid.h"

#include <new>
#include <stdlib.h>

namespace vc1 {

// STRING: VC1 0x004cfd9c
static const char kTileAnimationData[] = "TileAnimationData";
// GLOBAL: VC1 0x004d0af4
const char* g_kdNameTileAnimationData = kTileAnimationData;

// Inferred name. Script table at idx (always -1, folded by LTCG) as a
// name-sorted map of its values: sq_next over the table, each value
// AddRef'd into a SquirrelObject, the key read through the borrowed-string
// Get (0x00403990).
// FUNCTION: VC1 0x00409220
KdNameMap<SquirrelObject> Get(SqPlus::TypeWrapper<const KdNameMap<SquirrelObject>&>,
    HSQUIRRELVM v, int idx)
{
    KdNameMap<SquirrelObject> result;
    HSQOBJECT value;

    sq_pushnull(v);
    while (SQ_SUCCEEDED(sq_next(v, idx - 1))) {
        sq_getstackobj(v, -1, &value);
        SquirrelObject object(value);
        result[Get(SqPlus::TypeWrapper<const KdString&>(), v, sq_gettop(v) - 1)] = object;
        sq_pop(v, 2);
    }
    sq_pop(v, 1);
    return result;
}

// Inferred name. Script array of int arrays at idx: one element per slot,
// each read with the int-array Get (0x00403520). sq_get is inlined, so a
// missing slot raises "the index doesn't exist" and is skipped.
// FUNCTION: VC1 0x004093e0
KdObjArray<KdArray<int> > Get(SqPlus::TypeWrapper<const KdObjArray<KdArray<int> >&>,
    HSQUIRRELVM v, int idx)
{
    unsigned i;

    if (idx < 0) {
        idx = sq_gettop(v) + idx + 1;
    }
    KdObjArray<KdArray<int> > result(sq_getsize(v, idx));
    for (i = 0; i < result.Count(); ++i) {
        sq_pushinteger(v, i);
        if (SQ_SUCCEEDED(sq_get(v, idx))) {
            result[i].Assign(Get(SqPlus::TypeWrapper<const KdArray<int>&>(), v, sq_gettop(v)));
            sq_pop(v, 1);
        }
    }
    return result;
}

// LEA EAX, [ECX+0x20]; RET.
// FUNCTION: VC1 0x004a4540
const KdStringList& KdTileAnimationData::GetNames()
{
    return names;
}

// ADD ECX, 8; decrement refs at IAbstract+4. If 0, vtbl+0x0c(this, 1).
// FUNCTION: VC1 0x004a4590
unsigned KdTileAnimationData::Release()
{
    IUnknown* face;
    unsigned result;
    typedef void (__thiscall *Fn0c)(void* object, int arg);

    face = static_cast<IUnknown*>(static_cast<IAbstract*>(this));
    result = --refs;
    if (result == 0) {
        reinterpret_cast<Fn0c>((*reinterpret_cast<void***>(face))[3])(face, 1);
        return 0;
    }
    return result;
}

// MOV EAX, [ECX+0x0c]; TEST; INC; store. RET.
// FUNCTION: VC1 0x004a45b0
unsigned KdTileAnimationData::AddRef()
{
    unsigned value = refs;
    if (value != 0) {
        ++value;
        refs = value;
    }
    return value;
}

// RET 4. The animation's {channels, time} or null.
// FUNCTION: VC1 0x004a45c0
const TileAnimation* KdTileAnimationData::Find(const KdString& name)
{
    KdNameRecord<TileAnimation>* found = animations.Find(name);
    if (found == animations.End()) {
        return 0;
    }
    return &found->value;
}

// cdecl out. A new object returned through its IAbstract face.
// FUNCTION: VC1 0x004a4660
void** CreateKdTileAnimationData(void** out)
{
    KdTileAnimationData* object;
    IUnknown* ident;

    object = static_cast<KdTileAnimationData*>(malloc(sizeof(KdTileAnimationData)));
    if (object) {
        new (object) KdTileAnimationData();
    }
    if (!object) {
        ident = 0;
    }
    else {
        ident = static_cast<IUnknown*>(
            static_cast<IAbstract*>(object)->QueryInterface(&g_iidAbstract));
    }
    *out = ident;
    if (ident) {
        ident->AddRef();
    }
    static_cast<IUnknown*>(*out)->Release();
    return out;
}

// RET 4. this = IScriptLoadable. A table of { name = { Time, Channels } }:
// every name is appended to names, then its animation inserted unless the
// name is already present. A missing Time or Channels reads the record past
// the end, as in the original.
// FUNCTION: VC1 0x004a4720
bool KdTileAnimationData::LoadFromScript(SquirrelObject& table)
{
    KdNameRecord<SquirrelObject>* it;

    if (table.GetObjectHandle()._type != OT_TABLE) {
        return false;
    }
    KdNameMap<SquirrelObject> entries = GetScriptValue<KdNameMap<SquirrelObject> >(table);
    names.Reserve(entries.Count(), 1);
    animations.Reserve(entries.Count(), 1);
    for (it = entries.Begin(); it != entries.End(); ++it) {
        names.PushBack(it->key);
        KdNameMap<SquirrelObject> entry = GetScriptValue<KdNameMap<SquirrelObject> >(it->value);
        KdObjArray<KdArray<int> > channels;
        // STRING: VC1 0x004d1dc4
        int time = entry.Find(KdString("Time"))->value.ToInteger();
        channels = GetScriptValue<KdObjArray<KdArray<int> > >(
            // STRING: VC1 0x004d1dcc
            entry.Find(KdString("Channels"))->value);
        animations.InsertUnique(KdNameRecord<TileAnimation>(it->key, TileAnimation(channels, time)));
    }
    return true;
}

}
