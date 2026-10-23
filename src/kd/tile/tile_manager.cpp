#include "kd/tile/tile_manager.h"

#include "app/host_api.h"
#include "kdcore/platform.h"
#include "kd/iface/icamera.h"
#include "kd/iface/idisplay.h"
#include "kd/iface/iid.h"
#include "kd/iface/isystem.h"
#include "kd/scene/compound_object.h"
#include "kd/scene/scripted_object.h"
#include "kd/stream/file_reader.h"
#include "bindings/component.h"
#include "bindings/matrix3.h"
#include "bindings/sqplus.h"
#include "sqplus/SquirrelVM.h"

#include <math.h>
#include <new>
#include <stdlib.h>

namespace vc1 {

extern const char kScriptGetTexture[]; // resource_manager.cpp
extern const char kReadBinary[];       // display.cpp

// STRING: VC1 0x004d4b5c
static const char kTileManager[] = "TileManager";
// GLOBAL: VC1 0x004d068c
const char* g_kdNameTileManager = kTileManager;

// The live TileManager's IAbstract face; set by the factory, cleared by the
// destructor.
// GLOBAL: VC1 0x004ed624
IAbstract* g_kdTileManager;

// GLOBAL: VC1 0x004d64ac
extern const char kScriptGetRect[] = "GetRect";
// GLOBAL: VC1 0x004d64b4
extern const char kScriptGetTextureIndex[] = "GetTextureIndex";
// GLOBAL: VC1 0x004d64c4
extern const char kScriptSetTexture[] = "SetTexture";
// GLOBAL: VC1 0x004d64d0
extern const char kScriptGetFragments[] = "GetFragments";
// GLOBAL: VC1 0x004d64e0
extern const char kScriptLoadTexture[] = "LoadTexture";
// GLOBAL: VC1 0x004d0de0
extern const char kScriptHitTest[] = "HitTest";
// GLOBAL: VC1 0x004d1f4c
extern const char kScriptSetHighlightBlendMode[] = "SetHighlightBlendMode";
// GLOBAL: VC1 0x004d1f64
extern const char kScriptSetHighlightColor[] = "SetHighlightColor";
// GLOBAL: VC1 0x004d1f78
extern const char kScriptGetJointPosition[] = "GetJointPosition";
// GLOBAL: VC1 0x004d1f8c
extern const char kScriptResetTextureUsage[] = "ResetTextureUsage";

// Release the slot, then take the iid face of source (AddRef'd), or null.
template <class T>
static inline void AssignFace(KdRef<T>& slot, IUnknown* source, const char** iid)
{
    if (slot.ptr) {
        slot.ptr->Release();
    }
    if (!source) {
        slot.ptr = 0;
        return;
    }
    slot.ptr = static_cast<T*>(source->QueryInterface(iid));
    if (slot.ptr) {
        slot.ptr->AddRef();
    }
}

// The object as a script value: the IAbstract face QI'd twice, pushed through
// SquirrelObject::Set<KdPtr<IUnknown> > (0x004014a0).
static __forceinline SquirrelObject ScriptSelf(IAbstract* face)
{
    IUnknown* object = static_cast<IUnknown*>(face->QueryInterface(&g_iidAbstract));
    return SquirrelObject((KdPtr<IUnknown>(object
        ? static_cast<IUnknown*>(object->QueryInterface(&g_iidAbstract))
        : 0)));
}

// Texture index of an opcode 0 command: bits 6-7 are index bits 8-9, the high
// byte its low 8 bits.
static inline int FragmentTexture(unsigned short command)
{
    return static_cast<short>((command & 0xc0) << 2) | static_cast<short>(command >> 8);
}

// Inferred name. EDI=graphics. False only when the graphics transform is a
// plain translation and rect misses the clip rectangle moved into the same
// space; any rotation or scale is assumed visible.
// FUNCTION: VC1 0x0040a360
bool IsRectVisible(const KdPtr<IGraphics>& graphics, const Rect& rect)
{
    const Matrix3& transform = graphics->GetTransform();
    if (transform._11 == 1.0f && transform._12 == 0.0f && transform._21 == 0.0f
        && transform._22 == 1.0f) {
        Rect clip = graphics->GetClipRect();
        clip.x = static_cast<SQInteger>(clip.x - transform._31);
        clip.y = static_cast<SQInteger>(clip.y - transform._32);
        return clip.Overlaps(rect);
    }
    return true;
}

// MOV EAX, "TileManager"; RET. IScriptObject vtable+0x14 of both classes.
// FUNCTION: VC1 0x0041b4f0
const char* KdTileManager::GetTypeName()
{
    return kTileManager;
}

// ECX=this, EAX=count. Grows to count records and default-constructs the new
// ones.
// FUNCTION: VC1 0x0041b8d0
void TileRectList::Reserve(unsigned count)
{
    unsigned old = items.capacity >> 3;
    GrowCountedBuffer(&items, static_cast<int>(count * sizeof(TileRect)), 1);
    unsigned now = items.capacity >> 3;
    if (now > old) {
        TileRect* p = Data() + old;
        TileRect* last = Data() + now;
        for (; p < last; ++p) {
            new (p) TileRect();
        }
    }
}

// IScriptObject vtable+0x10: the script instance's SetMember (tail call).
// FUNCTION: VC1 0x00460430
unsigned char KdTileManager::SetMember(void* name, void* value)
{
    IScriptInstance* instance = &script;
    return instance->SetMember(static_cast<const char*>(name),
        *static_cast<const SquirrelObject*>(value));
}

// RET 8. ITileManager vtable+0x28, script "SetTexture".
// FUNCTION: VC1 0x00460440
void KdTileManager::SetTexture(int index, const KdPtr<ITexture>& texture)
{
    ITexture* value = texture.ptr;
    if (value) {
        value->AddRef();
    }
    if (textures[index].ptr) {
        textures[index].ptr->Release();
    }
    textures[index].ptr = value;
}

// RET 4.
// FUNCTION: VC1 0x00460490
void* KdTileManager::QueryInterface(const char** iid)
{
    const char* name = *iid;
    if (name == g_iidTileManager) {
        return this;
    }
    if (name == g_iidScriptObject) {
        return static_cast<IScriptObject*>(this);
    }
    if (name == g_iidScriptLoadable) {
        return static_cast<IScriptLoadable*>(this);
    }
    if (name == g_iidShutdownable) {
        return static_cast<IShutdownable*>(this);
    }
    if (name == g_iidLoadable) {
        return static_cast<ILoadable*>(this);
    }
    if (name == g_iidAbstract) {
        return static_cast<IAbstract*>(this);
    }
    return 0;
}

// The count lives in the IAbstract face (this+0x14).
// FUNCTION: VC1 0x00460500
unsigned KdTileManager::Release()
{
    IAbstract* iface = static_cast<IAbstract*>(this);
    unsigned n;

    iface->refs -= 1;
    n = iface->refs;
    if (n == 0) {
        delete iface;
        return 0;
    }
    return n;
}

// FUNCTION: VC1 0x00460520
unsigned KdTileManager::AddRef()
{
    unsigned n = refs;
    if (n != 0) {
        n += 1;
        refs = n;
    }
    return n;
}

// RET 0x10. ITileManager vtable+0x18, script "HitTest": (x, y) inside the
// tile's rectangle and, with the 1x1 target, not on a magenta pixel after the
// tile is drawn over magenta with (x, y) at the origin.
// FUNCTION: VC1 0x00460530
bool KdTileManager::HitTest(int tile, int x, int y, unsigned flags)
{
    if (tile == -1) {
        return false;
    }
    Rect rect = GetRect(tile, flags);
    if (!(rect.x <= x && x < rect.x + rect.width && rect.y <= y && y < rect.y + rect.height)) {
        return false;
    }
    bool hit = true;
    if (target) {
        KdPtr<IGraphics> graphics;
        {
            KdPtr<ICamera> camera(g_kdNullObject.ptr
                ? static_cast<ICamera*>(g_kdNullObject.ptr->QueryInterface(&g_iidCamera)) : 0);
            target->BeginFrame(&graphics.ptr, &camera);
        }
        if (graphics.ptr) {
            graphics->Clear(0xffff00ff);
            Render(graphics, tile, static_cast<float>(-x), static_cast<float>(-y), flags);
            target->EndFrame();
            KdPtr<ITexture> texture(*static_cast<KdPtr<ITexture>*>(target->GetRenderTexture()));
            if (texture.ptr) {
                Rect region(0, 0, 1, 1);
                const unsigned* pixel = static_cast<const unsigned*>(texture->Lock(&region, 1));
                if (pixel) {
                    hit = (*pixel & 0xffffff) != 0xff00ff;
                    texture->Unlock();
                }
            }
        }
    }
    return hit;
}

// RET 0xc (hidden return). ITileManager vtable+0x0c, script "GetRect": the
// tile's rectangle relative to its anchor, mirrored by the flip flags.
// FUNCTION: VC1 0x004606e0
Rect KdTileManager::GetRect(int tile, unsigned flags)
{
    if (tile == -1) {
        return Rect();
    }
    const TileRect& r = rects.Data()[tile];
    int y = (flags & kTileFlipY) ? 1 - r.height - r.y : r.y;
    int x = (flags & kTileFlipX) ? 1 - r.width - r.x : r.x;
    return Rect(x, y, r.width, r.height);
}

// ECX=IShutdownable (this+0xc). Points the target and every texture at the
// null object, then resets the script instance.
// FUNCTION: VC1 0x00460760
int KdTileManager::Shutdown()
{
    IScriptInstanceReset* reset;

    AssignFace(target, g_kdNullObject.ptr, &g_iidRenderTarget);
    for (int i = 0; i < 0x400; ++i) {
        AssignFace(textures[i], g_kdNullObject.ptr, &g_iidTexture);
    }
    reset = &script;
    reset->Reset();
    return 0;
}

// RET 4. ITileManager vtable+0x1c, script "GetTextureIndex": the texture of
// the tile's first fragment, following sub-tiles, or -1.
// FUNCTION: VC1 0x00460810
int KdTileManager::GetTextureIndex(int tile)
{
    if (tile == -1) {
        return -1;
    }
    int i = offsets[tile];
    const int end = offsets[tile + 1];
    while (i != end) {
        const unsigned short command = code[i++];
        switch (command & 0x3f) {
        case 0:
            return FragmentTexture(command);
        case 4:
            return GetTextureIndex(code[i]);
        case 3:
        case 6:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
            i += 2;
            break;
        case 5:
            i += 3;
            break;
        case 15:
        case 16:
            i += 1;
            break;
        }
    }
    return -1;
}

// RET 0x10. ITileManager vtable+0x10: true when the tile (or a sub-tile)
// carries joint; out is the last pen position set before it.
// FUNCTION: VC1 0x004608e0
bool KdTileManager::GetJoint(int tile, int joint, TilePoint* out, unsigned flags)
{
    if (tile == -1) {
        return false;
    }
    int i = offsets[tile];
    const int end = offsets[tile + 1];
    while (i != end) {
        switch (code[i++] & 0x3f) {
        case 0:
        case 6:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
            i += 2;
            break;
        case 1:
            flags ^= kTileFlipX;
            break;
        case 2:
            flags ^= kTileFlipY;
            break;
        case 3: {
            int x = code[i];
            if (flags & kTileFlipX) {
                x = -x;
            }
            ++i;
            out->x = x;
            if (flags & kTileFlipY) {
                out->y = -code[i++];
            } else {
                out->y = code[i++];
            }
            break;
        }
        case 4: {
            const int sub = code[i];
            const bool found = GetJoint(sub, joint, out, flags);
            ++i;
            if (found) {
                return true;
            }
            break;
        }
        case 5:
            i += 3;
            break;
        case 15:
            i += 1;
            break;
        case 16:
            if (joint == code[i++]) {
                return true;
            }
            break;
        }
    }
    return false;
}

// RET 4. ECX=ILoadable (this+0x10). res/Tiles.bin: short count, count records
// {short x, y, width, height; int end}, then the shorts of every tile's
// commands (offsets[i + 1] is record i's end).
// FUNCTION: VC1 0x00460a20
unsigned char KdTileManager::Load(const char* name)
{
    KdFileHandle* file = static_cast<KdFileHandle*>(OpenKdFile(name, kReadBinary));
    unsigned size;
    unsigned char* data;

    if (!file) {
        return 0;
    }
    data = static_cast<unsigned char*>(KdMapFile(file, &size));
    if (data) {
        const int count = *reinterpret_cast<short*>(data);
        unsigned pos = 2;
        unsigned bytes = count * 4 + 4;
        if (offsets.items.capacity < bytes) {
            GrowCountedBuffer(&offsets.items, static_cast<int>(bytes), 1);
        }
        offsets.items.size = count * 4 + 4;
        if (rects.items.capacity < static_cast<unsigned>(count * 8)) {
            rects.Reserve(count);
        }
        rects.items.size = count * 8;
        offsets[0] = 0;
        for (int i = 0; i < count; ++i) {
            TileRect& r = rects.Data()[i];
            r.x = *reinterpret_cast<short*>(data + pos);
            pos += 2;
            r.y = *reinterpret_cast<short*>(data + pos);
            pos += 2;
            r.width = *reinterpret_cast<short*>(data + pos);
            pos += 2;
            r.height = *reinterpret_cast<short*>(data + pos);
            pos += 2;
            offsets[i + 1] = *reinterpret_cast<int*>(data + pos);
            pos += 4;
        }
        bytes = offsets.Data()[offsets.items.size / 4 - 1] * 2;
        if (code.items.capacity < bytes) {
            GrowCountedBuffer(&code.items, static_cast<int>(bytes), 1);
        }
        code.items.size = bytes;
        const short* source = reinterpret_cast<short*>(data + pos);
        for (unsigned i = 0; i < code.items.size >> 1; ++i) {
            code[i] = source[i];
        }
    }
    if (file->driver->unmap) {
        file->driver->unmap(file->driver, file->native, data);
    } else if (data) {
        free(data);
    }
    file->driver->close(file->driver, file->native);
    free(file);
    return 1;
}

// RET 0xc (hidden return). ITileManager vtable+0x20, script "GetFragments":
// (sub-tile, pen x, pen y) for every sub-tile command of the tile.
// FUNCTION: VC1 0x00460be0
KdArray<unsigned> KdTileManager::GetFragments(int tile, unsigned flags)
{
    KdArray<unsigned> result;
    int x = 0;
    int y = 0;
    int i = offsets[tile];
    const int end = offsets[tile + 1];

    while (i != end) {
        switch (code[i++] & 0x3f) {
        case 0:
        case 6:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
            i += 2;
            break;
        case 1:
            flags ^= kTileFlipX;
            break;
        case 2:
            flags ^= kTileFlipY;
            break;
        case 3:
            x = code[i];
            if (flags & kTileFlipX) {
                x = -x;
            }
            ++i;
            y = (flags & kTileFlipY) ? -code[i] : code[i];
            ++i;
            break;
        case 4: {
            const int sub = code[i++];
            result.PushBack(sub);
            result.PushBack(x);
            result.PushBack(y);
            break;
        }
        case 5:
            i += 3;
            break;
        case 15:
        case 16:
            i += 1;
            break;
        }
    }
    return result;
}

// ECX=this. Resets the script instance and the singleton; the members release
// the target and textures and free the tables.
// FUNCTION: VC1 0x00460d70
KdTileManager::~KdTileManager() throw()
{
    IScriptInstanceReset* reset = &script;
    reset->Reset();
    g_kdTileManager = 0;
}

// ESI=this. refs=1, every texture null, and a 1x1 render target from the
// system's display (ISystem vtable+0x40, IDisplay vtable+0x38) for HitTest.
// FUNCTION: VC1 0x00460ed0
KdTileManager::KdTileManager()
{
    KdPtr<ISystem> system(GetKdSystem());
    KdPtr<IUnknown> display;
    IDisplay* face = *reinterpret_cast<IDisplay**>(
        system->GetDisplay(reinterpret_cast<void**>(&display.ptr)));
    KdPtr<IRenderTarget> created(face->CreateRenderTarget(1, 1));
    AssignFace(target, created.ptr, &g_iidRenderTarget);
}

// RET 4. ITileManager vtable+0x24, script "GetTexture": texture slot index,
// filled on first use from the script member "LoadTexture"(index).
// FUNCTION: VC1 0x00461460
KdPtr<ITexture>& KdTileManager::GetTexture(int index)
{
    KdRef<ITexture>& slot = textures[index];
    if (!slot) {
        IScriptInstance* instance = &script;
        SquirrelObject member;
        instance->GetMember(kScriptLoadTexture, member);
        SquirrelObject self = ScriptSelf(this);
        SquirrelVM::BeginCall(member, self);
        SquirrelVM::PushParam(index);
        KdPtr<IAbstract> loaded = SquirrelVM::EndCall().Get<KdPtr<IAbstract> >();
        AssignFace(slot, loaded.ptr, &g_iidTexture);
    }
    return reinterpret_cast<KdPtr<ITexture>&>(slot);
}

// RET 4. IScriptLoadable vtable+0x0c, owner = the IScriptObject face.
// FUNCTION: VC1 0x004616c0
bool KdTileManager::LoadFromScript(SquirrelObject& table)
{
    return script.Load(table, static_cast<IScriptObject*>(this)) != 0;
}

// RET 8. The script instance's members first, then the method table (table
// 0x004ee63c, guard bit 0x004ee64c, atexit destructor 0x004cae70).
// FUNCTION: VC1 0x00461700
unsigned char KdTileManager::GetMember(const char* name, SquirrelObject* out)
{
    IScriptInstance* instance = &script;

    if (instance->GetMember(name, *out)) {
        return 1;
    }
    static ScriptMethodTable methods;
    if (methods.Empty()) {
        methods[kScriptGetRect] = CreateScriptMethod<KdTileManager>(&KdTileManager::GetRect);
        methods[kScriptRender] = CreateScriptMethod<KdTileManager>(&KdTileManager::Render);
        methods[kScriptHitTest] = CreateScriptMethod<KdTileManager>(&KdTileManager::HitTest);
        methods[kScriptGetTextureIndex] =
            CreateScriptMethod<KdTileManager>(&KdTileManager::GetTextureIndex);
        methods[kScriptGetTexture] = CreateScriptMethod<KdTileManager>(&KdTileManager::GetTexture);
        methods[kScriptSetTexture] = CreateScriptMethod<KdTileManager>(&KdTileManager::SetTexture);
        methods[kScriptGetFragments] =
            CreateScriptMethod<KdTileManager>(&KdTileManager::GetFragments);
    }
    return methods.Find(name, out);
}

// RET 0x14. ITileManager vtable+0x14, script "Render": draws the tile with
// its anchor at (x, y), snapped down to whole pixels. Commands: 0 fragment
// (u, v), 1/2 flip x/y, 3 pen (x, y), 4 sub-tile at the pen, 5 show the rest
// only on frame (frame, period ms, frames), 6 color (hi, lo), 10 line (dy,
// dx), 11 fill / 12 outline (width, height), 15 blend mode, 16 joint.
// FUNCTION: VC1 0x004619b0
void KdTileManager::Render(const KdPtr<IGraphics>& graphics, int tile, float x, float y,
    unsigned flags)
{
    if (tile == -1 || (flags & kTileHidden)) {
        return;
    }
    Rect rect = GetRect(tile, flags);
    rect.x = static_cast<SQInteger>(floorf(rect.x + x));
    rect.y = static_cast<SQInteger>(floorf(rect.y + y));
    if (!IsRectVisible(graphics, rect)) {
        return;
    }
    int i = offsets[tile];
    int penX = 0;
    int penY = 0;
    const int end = offsets[tile + 1];
    while (i != end) {
        const unsigned short command = code[i];
        const unsigned op = command & 0x3f;
        ++i;
        switch (op) {
        case 0: {
            unsigned short u = code[i];
            unsigned short v = code[i + 1];
            i += 2;
            KdPtr<ITexture>& texture = GetTexture(FragmentTexture(command));
            if (!texture.ptr) {
                break;
            }
            if (flags & kTileFlipX) {
                u = static_cast<unsigned short>(u + rect.width);
                rect.x += rect.width;
                rect.width = -rect.width;
            }
            if (flags & kTileFlipY) {
                v = static_cast<unsigned short>(v + rect.height);
                rect.y += rect.height;
                rect.height = -rect.height;
            }
            graphics->DrawImage(texture, static_cast<float>(rect.x), static_cast<float>(rect.y),
                static_cast<float>(static_cast<short>(u)), static_cast<float>(static_cast<short>(v)),
                static_cast<float>(rect.width), static_cast<float>(rect.height));
            break;
        }
        case 1:
            flags ^= kTileFlipX;
            break;
        case 2:
            flags ^= kTileFlipY;
            break;
        case 3:
            penX = code[i];
            if (flags & kTileFlipX) {
                penX = -penX;
            }
            ++i;
            if (flags & kTileFlipY) {
                penY = -code[i];
            } else {
                penY = code[i];
            }
            ++i;
            break;
        case 4:
            Render(graphics, code[i], penX + x, penY + y, flags);
            ++i;
            break;
        case 5: {
            const int frame = code[i];
            const int period = code[i + 1];
            const int frames = code[i + 2];
            i += 3;
            if (frame == getTime() / period % frames) {
                flags &= ~kTileHidden;
            } else {
                flags |= kTileHidden;
            }
            break;
        }
        case 6:
            graphics->SetColor((code[i] << 16) | (code[i + 1] & 0xffff));
            i += 2;
            break;
        case 10: {
            int dy = code[i];
            if (flags & kTileFlipY) {
                dy = -dy;
            }
            ++i;
            int dx = (flags & kTileFlipX) ? -code[i] : code[i];
            ++i;
            const float fromY = penY + y;
            const float fromX = penX + x;
            graphics->DrawLine(fromX, fromY, dx + fromX, dy + fromY);
            break;
        }
        case 11:
        case 12:
        case 13:
        case 14: {
            const int width = code[i];
            const int height = code[i + 1];
            i += 2;
            const int offsetX = (flags & kTileFlipX) ? -width : 0;
            const int left = static_cast<int>(x) + offsetX + penX;
            const int offsetY = (flags & kTileFlipY) ? -height : 0;
            const int top = static_cast<int>(y) + offsetY + penY;
            if (op == 11) {
                graphics->FillRect(static_cast<float>(left), static_cast<float>(top),
                    static_cast<float>(width), static_cast<float>(height));
            } else if (op == 12) {
                graphics->DrawRect(static_cast<float>(left), static_cast<float>(top),
                    static_cast<float>(width), static_cast<float>(height));
            }
            break;
        }
        case 15:
            graphics->SetMode(code[i]);
            ++i;
            break;
        case 16:
            ++i;
            break;
        }
    }
}

// RET. Script "ResetTextureUsage": nothing to reset in this build.
// FUNCTION: VC1 0x004a1930
void VcTileManager::ResetTextureUsage()
{
}

// RET 4. Script "SetHighlightColor".
// FUNCTION: VC1 0x004a1940
void VcTileManager::SetHighlightColor(unsigned color)
{
    highlightColor = color;
}

// RET 4. Script "SetHighlightBlendMode".
// FUNCTION: VC1 0x004a1950
void VcTileManager::SetHighlightBlendMode(int mode)
{
    highlightMode = mode;
}

// XOR AL, AL; RET 8. IScriptObject vtable+0x10: script members are read-only.
// FUNCTION: VC1 0x004a1960
unsigned char VcTileManager::SetMember(void* name, void* value)
{
    (void)name;
    (void)value;
    return 0;
}

// RET 0xc (hidden return). Script "GetJointPosition": GetJoint without flags,
// (0, 0) when the tile has no such joint.
// FUNCTION: VC1 0x004a1970
Vector2 VcTileManager::GetJointPosition(int tile, int joint)
{
    TilePoint point;
    point.y = 0;
    point.x = 0;
    GetJoint(tile, joint, &point, 0);
    return Vector2(static_cast<float>(point.x), static_cast<float>(point.y));
}

// RET 4. KdTileManager's faces, then IAbstract again.
// FUNCTION: VC1 0x004a19b0
void* VcTileManager::QueryInterface(const char** iid)
{
    void* face = KdTileManager::QueryInterface(iid);
    if (face) {
        return face;
    }
    if (*iid == g_iidAbstract) {
        return static_cast<IAbstract*>(this);
    }
    return 0;
}

// FUNCTION: VC1 0x004a19e0
unsigned VcTileManager::Release()
{
    IAbstract* iface = static_cast<IAbstract*>(this);
    unsigned n;

    iface->refs -= 1;
    n = iface->refs;
    if (n == 0) {
        delete iface;
        return 0;
    }
    return n;
}

// FUNCTION: VC1 0x004a1a00
unsigned VcTileManager::AddRef()
{
    unsigned n = refs;
    if (n != 0) {
        n += 1;
        refs = n;
    }
    return n;
}

// EAX=this. No clip, highlight blend mode 2 in white.
// FUNCTION: VC1 0x004a1a40
VcTileManager::VcTileManager()
    : clip(0), highlightMode(2), highlightColor(0xffffffff)
{
}

// The singleton's IAbstract face, created on first use.
// FUNCTION: VC1 0x004a1aa0
void** CreateKdTileManager(void** out)
{
    IUnknown* object;

    if (!g_kdTileManager) {
        void* const memory = malloc(sizeof(VcTileManager));
        VcTileManager* created = memory ? new (memory) VcTileManager() : 0;
        if (!created) {
            g_kdTileManager = 0;
            object = 0;
        } else {
            g_kdTileManager = static_cast<IAbstract*>(created);
            object = g_kdTileManager
                ? static_cast<IUnknown*>(g_kdTileManager->QueryInterface(&g_iidAbstract)) : 0;
            if (object) {
                object->AddRef();
            }
        }
        object->Release();
        *out = object;
        object->AddRef();
        object->Release();
        return out;
    }
    object = static_cast<IUnknown*>(g_kdTileManager->QueryInterface(&g_iidAbstract));
    *out = object;
    if (object) {
        object->AddRef();
    }
    return out;
}

// RET 8. KdTileManager's members and methods, then this table (table
// 0x004ee68c, guard bit 0x004ee69c, atexit destructor 0x004ca250).
// FUNCTION: VC1 0x004a1e50
unsigned char VcTileManager::GetMember(const char* name, SquirrelObject* out)
{
    if (KdTileManager::GetMember(name, out)) {
        return 1;
    }
    static ScriptMethodTable methods;
    if (methods.Empty()) {
        methods[kScriptSetHighlightBlendMode] =
            CreateScriptMethod<VcTileManager>(&VcTileManager::SetHighlightBlendMode);
        methods[kScriptSetHighlightColor] =
            CreateScriptMethod<VcTileManager>(&VcTileManager::SetHighlightColor);
        methods[kScriptGetJointPosition] =
            CreateScriptMethod<VcTileManager>(&VcTileManager::GetJointPosition);
        methods[kScriptResetTextureUsage] =
            CreateScriptMethod<VcTileManager>(&VcTileManager::ResetTextureUsage);
    }
    return methods.Find(name, out);
}

// RET 0x14. ITileManager vtable+0x14: KdTileManager::Render with a float
// destination (whole pixels unless kTileSubPixel), fragments cut to clip, and
// under kTileHighlight each fragment drawn again in the highlight mode and
// color.
// FUNCTION: VC1 0x004a2040
void VcTileManager::Render(const KdPtr<IGraphics>& graphics, int tile, float x, float y,
    unsigned flags)
{
    if (tile == -1 || (flags & kTileHidden)) {
        return;
    }
    Rect rect = GetRect(tile, flags);
    RectF dest(rect.x + x, rect.y + y, static_cast<SQFloat>(rect.width),
        static_cast<SQFloat>(rect.height));
    if (!(flags & kTileSubPixel)) {
        dest.x = static_cast<SQFloat>(static_cast<int>(floorf(dest.x)));
        dest.y = static_cast<SQFloat>(static_cast<int>(floorf(dest.y)));
    }
    rect.x = static_cast<SQInteger>(dest.x);
    rect.y = static_cast<SQInteger>(dest.y);
    if (!IsRectVisible(graphics, rect)) {
        return;
    }
    int i = offsets[tile];
    int penX = 0;
    int penY = 0;
    const int end = offsets[tile + 1];
    while (i != end) {
        const unsigned short command = code[i];
        const unsigned op = command & 0x3f;
        ++i;
        switch (op) {
        case 0: {
            unsigned short u = code[i];
            unsigned short v = code[i + 1];
            i += 2;
            KdPtr<ITexture>& texture = GetTexture(FragmentTexture(command));
            if (!texture.ptr) {
                break;
            }
            if (clip) {
                const SQFloat oldX = dest.x;
                const SQFloat oldY = dest.y;
                if (!dest.Intersect(*clip)) {
                    break;
                }
                u = static_cast<unsigned short>(static_cast<int>(
                    dest.x - oldX + static_cast<short>(u)));
                v = static_cast<unsigned short>(static_cast<int>(
                    dest.y - oldY + static_cast<short>(v)));
            }
            if (flags & kTileFlipX) {
                u = static_cast<unsigned short>(static_cast<int>(
                    static_cast<short>(u) + dest.width));
                dest.x = dest.width + dest.x;
                dest.width = -dest.width;
            }
            if (flags & kTileFlipY) {
                v = static_cast<unsigned short>(static_cast<int>(
                    static_cast<short>(v) + dest.height));
                dest.y = dest.height + dest.y;
                dest.height = -dest.height;
            }
            graphics->DrawImage(texture, dest.x, dest.y,
                static_cast<float>(static_cast<short>(u)), static_cast<float>(static_cast<short>(v)),
                dest.width, dest.height);
            if (flags & kTileHighlight) {
                const unsigned mode = graphics->GetMode();
                const unsigned color = graphics->GetColor();
                graphics->SetMode(highlightMode);
                graphics->SetColor(highlightColor);
                graphics->DrawImage(texture, dest.x, dest.y,
                    static_cast<float>(static_cast<short>(u)),
                    static_cast<float>(static_cast<short>(v)), dest.width, dest.height);
                graphics->SetMode(mode);
                graphics->SetColor(color);
            }
            break;
        }
        case 1:
            flags ^= kTileFlipX;
            break;
        case 2:
            flags ^= kTileFlipY;
            break;
        case 3:
            penX = code[i];
            if (flags & kTileFlipX) {
                penX = -penX;
            }
            ++i;
            if (flags & kTileFlipY) {
                penY = -code[i];
            } else {
                penY = code[i];
            }
            ++i;
            break;
        case 4:
            Render(graphics, code[i], penX + x, penY + y, flags);
            ++i;
            break;
        case 5: {
            const int frame = code[i];
            const int period = code[i + 1];
            const int frames = code[i + 2];
            i += 3;
            if (frame == getTime() / period % frames) {
                flags &= ~kTileHidden;
            } else {
                flags |= kTileHidden;
            }
            break;
        }
        case 6:
            graphics->SetColor((code[i] << 16) | (code[i + 1] & 0xffff));
            i += 2;
            break;
        case 10: {
            int dy = code[i];
            if (flags & kTileFlipY) {
                dy = -dy;
            }
            ++i;
            int dx = (flags & kTileFlipX) ? -code[i] : code[i];
            ++i;
            const float fromY = penY + y;
            const float fromX = penX + x;
            graphics->DrawLine(fromX, fromY, dx + fromX, dy + fromY);
            break;
        }
        case 11:
        case 12:
        case 13:
        case 14: {
            const int width = code[i];
            const int height = code[i + 1];
            i += 2;
            const int offsetX = (flags & kTileFlipX) ? -width : 0;
            const int left = static_cast<int>(x) + offsetX + penX;
            const int offsetY = (flags & kTileFlipY) ? -height : 0;
            const int top = static_cast<int>(y) + offsetY + penY;
            if (op == 11) {
                graphics->FillRect(static_cast<float>(left), static_cast<float>(top),
                    static_cast<float>(width), static_cast<float>(height));
            } else if (op == 12) {
                graphics->DrawRect(static_cast<float>(left), static_cast<float>(top),
                    static_cast<float>(width), static_cast<float>(height));
            }
            break;
        }
        case 15:
            graphics->SetMode(code[i]);
            ++i;
            break;
        case 16:
            ++i;
            break;
        }
    }
}

}
