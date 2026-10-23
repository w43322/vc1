#include "app/script.h"
#include "kdcore/platform.h"
#include "memory/fixed_pool.h"

#include "bindings/component.h"
#include "bindings/rect.h"
#include "bindings/sqplus.h"
#include "bindings/vector2.h"
#include "kd/font/font.h"
#include "kd/iface/icamera.h"
#include "kd/iface/idisplay.h"
#include "kd/iface/igraphics.h"
#include "kd/iface/iid.h"
#include "kd/tile/tile_manager.h"

#include <new>
#include <squirrel.h>
#include <sqstdaux.h>
#include <sqstdblob.h>
#include <sqstdio.h>
#include <sqstdmath.h>
#include <sqstdstring.h>
#include <sqstdsystem.h>

namespace vc1 {

// Inferred name. enableCursor's per-cursor flag.
// SIZE 0x08
struct CursorState
{
    int cursor;   // 0x00
    bool enabled; // 0x04

    CursorState() : cursor(0), enabled(false) {}
};

typedef char Vc1CursorStateSizeCheck[sizeof(CursorState) == 0x08 ? 1 : -1];

// Inferred name. CursorState records sorted by cursor in a KdFactory buffer;
// the only instance is enableCursor's function-local static (0x004ee3c8), so
// LTCG folds this into the member bodies.
// SIZE 0x10
struct CursorMap
{
    KdFactory items; // 0x00

    CursorState* Begin() const { return reinterpret_cast<CursorState*>(items.begin); }
    CursorState* End() const
    {
        return reinterpret_cast<CursorState*>(reinterpret_cast<char*>(items.begin) + items.size);
    }
    bool& operator[](const int& cursor);
    CursorState* LowerBound(const int& cursor);
    void Reserve(unsigned count);
};

// EAX=&cursor. Inserts a disabled record when cursor is missing.
// FUNCTION: VC1 0x004024e0
bool& CursorMap::operator[](const int& cursor)
{
    CursorState* found = LowerBound(cursor);
    CursorState* first = Begin();
    unsigned bytes = items.size;
    CursorState* slot;
    int key;

    if (found == End() || cursor < found->cursor) {
        key = cursor;
        Reserve(bytes / sizeof(CursorState) + 1);
        slot = static_cast<CursorState*>(InsertCountedBytes(&items,
            reinterpret_cast<char*>(items.begin) + (reinterpret_cast<char*>(found)
                - reinterpret_cast<char*>(first)),
            sizeof(CursorState)));
        slot->cursor = key;
        slot->enabled = false;
        return slot->enabled;
    }
    return found->enabled;
}

// EDX=&cursor. First record whose cursor is not below the key.
// FUNCTION: VC1 0x00402650
CursorState* CursorMap::LowerBound(const int& cursor)
{
    CursorState* first = Begin();
    CursorState* last = End();
    CursorState* middle;

    while (first != last) {
        middle = first + (last - first) / 2;
        if (middle->cursor < cursor) {
            first = middle + 1;
        }
        else {
            last = middle;
        }
    }
    return first;
}

// EAX=count. Grows the buffer and default-constructs the new capacity.
// FUNCTION: VC1 0x00402d50
void CursorMap::Reserve(unsigned count)
{
    unsigned old = items.capacity / sizeof(CursorState);
    CursorState* p;
    CursorState* last;

    GrowCountedBuffer(&items, static_cast<int>(count * sizeof(CursorState)), 0);
    if (old < items.capacity / sizeof(CursorState)) {
        last = Begin() + items.capacity / sizeof(CursorState);
        for (p = Begin() + old; p < last; ++p) {
            new (p) CursorState();
        }
    }
}

// ESI=v, EDI=v. New script array, then array[i] = value[i].
// FUNCTION: VC1 0x00403860
void Push(HSQUIRRELVM v, KdArray<unsigned>& value)
{
    unsigned i;

    sq_newarray(v, value.Count());
    for (i = 0; i < value.Count(); ++i) {
        sq_pushinteger(v, i);
        sq_pushinteger(v, value[i]);
        sq_set(v, -3);
    }
}

// EDI=v, ESI=result. A type error is logged and leaves the empty string.
// FUNCTION: VC1 0x00403990
KdString Get(SqPlus::TypeWrapper<const KdString&>, HSQUIRRELVM v, int idx)
{
    const SQChar* s = 0;
    SQPLUS_CHECK_GET(sq_getstring(v, idx, &s));
    return KdString(s);
}

// EAX=argc, EBX=argv from RunApp's g_runScratch0/1. Args array then root["ARGS"].
// FUNCTION: VC1 0x00463b40
void InitScriptEngine(int argc, char** argv)
{
    int i;

    InitSizeClassPools();
    SquirrelVM::Init();
    BindHostClasses();
    InitHostServices();

    SquirrelObject args = SquirrelVM::CreateArray(argc);
    for (i = 0; i < argc; ++i) {
        args.SetValue(i, argv[i]);
    }
    // STRING: VC1 0x004d6384
    SquirrelVM::GetRootTable().SetValue("ARGS", args);
}

// Script "profilerReset". Unwinds the open profiler scopes (0x004e970c),
// clears every node's counters (list 0x004e96fc) and restarts the frame
// clock (0x004eb0d0). The profiler is not recovered yet.
// STUB: VC1 0x004bd0f0
void ScriptProfilerReset()
{
}

// Script "decodeUTF8": the code points of text, up to the first NUL.
// FUNCTION: VC1 0x004bd240
KdArray<unsigned> ScriptDecodeUtf8(const KdString& text)
{
    KdArray<unsigned> codes;
    Utf8Cursor cursor;

    cursor.text = text.c_str();
    cursor.code = 0;
    DecodeUtf8(&cursor);
    while (cursor.code) {
        codes.PushBack(cursor.code);
        ++cursor.text;
        DecodeUtf8(&cursor);
    }
    return codes;
}

// Script "encodeUTF8". The original encodes into a fixed 0x100-byte stack
// buffer without a bound check.
// FUNCTION: VC1 0x004bd2d0
KdString ScriptEncodeUtf8(const KdArray<unsigned>& codes)
{
    char buffer[0x100];
    char* out = buffer;
    unsigned i;

    for (i = 0; i < codes.Count(); ++i) {
        EncodeUtf8(&out, codes[i]);
    }
    return KdString(buffer, out);
}

// Script "renderFrame"(graphics, rect, frames, color): a nine-tile frame.
// frames holds the tiles corner/top/corner, left/centre/right,
// corner/bottom/corner; the side tiles' sizes give the inner rectangle. The
// centre and the sides repeat their tile and are cut with the game tile
// manager's clip rect (VcTileManager+0x1068); the corners are drawn at the
// inner rectangle's corners without a clip. TileManager is this function's
// own static (guard 0x004ee780, value 0x004ee77c).
// FUNCTION: VC1 0x004bd350
void ScriptRenderFrame(const KdPtr<IGraphics>& graphics, const Rect& area,
    const KdArray<int>& frames, int color)
{
    static KdPtr<ITileManager> manager;
    static bool initialized;

    if (!initialized) {
        void* unknown;

        initialized = true;
        GetKdFactory()->QueryNamed(&unknown, &g_kdNameTileManager);
        if (unknown) {
            manager.ptr = static_cast<ITileManager*>(
                static_cast<IUnknown*>(unknown)->QueryInterface(&g_iidTileManager));
            if (manager.ptr) {
                manager.ptr->AddRef();
            }
            static_cast<IUnknown*>(unknown)->Release();
        }
        else {
            manager.ptr = 0;
        }
    }

    graphics.ptr->SetColor(color);
    const int left = area.x + manager.ptr->GetRect(frames[3], 0).width;
    const int right = area.x + area.width - manager.ptr->GetRect(frames[5], 0).width;
    const int top = area.y + manager.ptr->GetRect(frames[1], 0).height;
    const int bottom = area.y + area.height - manager.ptr->GetRect(frames[7], 0).height;
    const SQFloat leftF = static_cast<SQFloat>(left);
    const SQFloat topF = static_cast<SQFloat>(top);
    const SQFloat widthF = static_cast<SQFloat>(right - left);
    const SQFloat heightF = static_cast<SQFloat>(bottom - top);
    VcTileManager* const game = static_cast<VcTileManager*>(manager.ptr);
    RectF clip;
    Rect step;
    int x;
    int y;

    game->clip = &clip;
    clip.x = leftF;
    clip.y = topF;
    clip.width = widthF;
    clip.height = heightF;
    step = manager.ptr->GetRect(frames[4], 0);
    for (y = top; y < bottom; y += step.height) {
        for (x = left; x < right; x += step.width) {
            manager.ptr->Render(graphics, frames[4], static_cast<SQFloat>(x),
                static_cast<SQFloat>(y), 0);
        }
    }

    clip.x = leftF;
    clip.y = static_cast<SQFloat>(area.y);
    clip.width = widthF;
    clip.height = static_cast<SQFloat>(area.height);
    step = manager.ptr->GetRect(frames[1], 0);
    for (x = left; x < right; x += step.width) {
        manager.ptr->Render(graphics, frames[1], static_cast<SQFloat>(x), topF, 0);
    }
    step = manager.ptr->GetRect(frames[7], 0);
    for (x = left; x < right; x += step.width) {
        manager.ptr->Render(graphics, frames[7], static_cast<SQFloat>(x),
            static_cast<SQFloat>(bottom), 0);
    }

    clip.x = static_cast<SQFloat>(area.x);
    clip.y = topF;
    clip.width = static_cast<SQFloat>(area.width);
    clip.height = heightF;
    step = manager.ptr->GetRect(frames[3], 0);
    for (y = top; y < bottom; y += step.height) {
        manager.ptr->Render(graphics, frames[3], leftF, static_cast<SQFloat>(y), 0);
    }
    step = manager.ptr->GetRect(frames[5], 0);
    for (y = top; y < bottom; y += step.height) {
        manager.ptr->Render(graphics, frames[5], static_cast<SQFloat>(right),
            static_cast<SQFloat>(y), 0);
    }

    game->clip = 0;
    const SQFloat rightF = static_cast<SQFloat>(right);
    const SQFloat bottomF = static_cast<SQFloat>(bottom);
    manager.ptr->Render(graphics, frames[0], leftF, topF, 0);
    manager.ptr->Render(graphics, frames[2], rightF, topF, 0);
    manager.ptr->Render(graphics, frames[6], leftF, bottomF, 0);
    manager.ptr->Render(graphics, frames[8], rightF, bottomF, 0);
}

// Script "fillRectByTile"(graphics, rect, tile, spacing, origin, relative):
// covers area with tile every (tile size + spacing) pixels, phased so a tile
// starts at origin; with relative set origin is moved by the camera position
// in screen space. Tiles that miss the graphics clip rect are skipped.
// TileManager is this function's own static (guard 0x004ee788, value
// 0x004ee784).
// FUNCTION: VC1 0x004bd890
void ScriptFillRectByTile(const KdPtr<IGraphics>& graphics, const Rect& area, int tile,
    int spacing, const Vector2& origin, bool relative)
{
    static KdPtr<ITileManager> manager;
    static bool initialized;

    if (!initialized) {
        void* unknown;

        initialized = true;
        GetKdFactory()->QueryNamed(&unknown, &g_kdNameTileManager);
        if (unknown) {
            manager.ptr = static_cast<ITileManager*>(
                static_cast<IUnknown*>(unknown)->QueryInterface(&g_iidTileManager));
            if (manager.ptr) {
                manager.ptr->AddRef();
            }
            static_cast<IUnknown*>(unknown)->Release();
        }
        else {
            manager.ptr = 0;
        }
    }

    if (!graphics.ptr) {
        return;
    }
    Vector2 offset = origin;
    if (relative) {
        ICamera* const camera = static_cast<ICamera*>(*graphics.ptr->GetCamera());
        if (!camera) {
            return;
        }
        const Matrix3& position = camera->GetPosition();
        const Matrix3& m = camera->GetWorldToScreen();
        const SQFloat screenX = position._32 * m._21 + position._31 * m._11 + m._31;
        const SQFloat screenY = position._31 * m._12 + position._32 * m._22 + m._32;
        offset.x -= screenX;
        offset.y -= screenY;
    }

    const Rect bounds = manager.ptr->GetRect(tile, 0);
    const int stepX = bounds.width + spacing;
    const int stepY = bounds.height + spacing;
    int shiftX = static_cast<int>(-offset.x) % stepX;
    if (shiftX < 0) {
        shiftX += stepX;
    }
    const int right = area.width - bounds.x + area.x;
    int x = area.x - shiftX - bounds.x;
    int shiftY = static_cast<int>(-offset.y) % stepY;
    if (shiftY < 0) {
        shiftY += stepY;
    }
    const int top = area.y - bounds.y - shiftY;
    const int bottom = area.height - bounds.y + area.y;

    for (; x < right; x += stepX) {
        int y;
        for (y = top; y < bottom; y += stepY) {
            const int tileLeft = bounds.x + x;
            const int tileTop = bounds.y + y;
            const Rect& clip = graphics.ptr->GetClipRect();
            if (clip.x < bounds.width + tileLeft && clip.y < bounds.height + tileTop
                && clip.x + clip.width > tileLeft && clip.y + clip.height > tileTop) {
                manager.ptr->Render(graphics, tile, static_cast<SQFloat>(x),
                    static_cast<SQFloat>(y), 0);
            }
        }
    }
}

// Script "enableCursor": flags cursor, then shows the highest-numbered
// enabled cursor (none: the current one stays).
// FUNCTION: VC1 0x004bdbc0
void ScriptEnableCursor(int cursor, bool enable)
{
    static CursorMap cursors;
    CursorState* first;
    CursorState* it;

    cursors[cursor] = enable;
    first = cursors.Begin();
    for (it = cursors.End(); it != first; --it) {
        if (it[-1].enabled) {
            ScriptSetCursor(it[-1].cursor);
            return;
        }
    }
}

// FUNCTION: VC1 0x004bdc50
void RegisterStdBindings()
{
    // STRING: VC1 0x004d0d20
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &ScriptProfilerReset, "profilerReset");
    // STRING: VC1 0x004d0d30
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &ScriptSetCursor, "setCursor");
    // STRING: VC1 0x004d0d3c
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &ScriptEnableCursor, "enableCursor");
    // STRING: VC1 0x004d0d4c
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &ScriptFillRectByTile, "fillRectByTile");
    // STRING: VC1 0x004d0d5c
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &ScriptRenderFrame, "renderFrame");
    // STRING: VC1 0x004d0d68
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &ScriptEncodeUtf8, "encodeUTF8");
    // STRING: VC1 0x004d0d74
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &ScriptDecodeUtf8, "decodeUTF8");
}

}
