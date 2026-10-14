#include "host/runtime.h"

#include "bindings/component.h"
#include "bindings/rect.h"
#include "bindings/sqplus.h"
#include "bindings/vector2.h"
#include "kd/font/font.h"
#include "kd/iface/idisplay.h"
#include "kd/iface/iid.h"

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

// Script "renderFrame"(graphics, rect, frames, flags). Not recovered yet.
// STUB: VC1 0x004bd350
void ScriptRenderFrame(const KdPtr<IGraphics>& graphics, const Rect& area,
    const KdArray<int>& frames, int flags)
{
    (void)graphics;
    (void)area;
    (void)frames;
    (void)flags;
}

// Script "fillRectByTile"(graphics, rect, tile, spacing, origin, relative):
// tiles area with ITileManager tile (static 0x004ee784) from origin, shifted
// by the camera when relative is set. Not recovered yet.
// STUB: VC1 0x004bd890
void ScriptFillRectByTile(const KdPtr<IGraphics>& graphics, const Rect& area, int tile,
    int spacing, const Vector2& origin, bool relative)
{
    (void)graphics;
    (void)area;
    (void)tile;
    (void)spacing;
    (void)origin;
    (void)relative;
}

// Script "setCursor": stores the resource id in the display window state
// (+0x0c), loads it from the module and makes it the window class cursor.
// FUNCTION: VC1 0x004bdb20
void ScriptSetCursor(int cursor)
{
    IDisplay* display = static_cast<IDisplay*>(AcquireNamed(&g_kdNameDisplay, &g_iidDisplay));
    unsigned char* window;
    HCURSOR handle;

    if (display) {
        window = static_cast<unsigned char*>(display->GetMixer());
        *reinterpret_cast<int*>(window + 0x0c) = cursor;
        handle = LoadCursorA(GetKdWindowClass()->hInstance, MAKEINTRESOURCEA(cursor & 0xffff));
        SetCursor(handle);
        SetClassLongA(*reinterpret_cast<HWND*>(window + 0x04), GCL_HCURSOR,
            reinterpret_cast<LONG>(handle));
        display->Release();
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
