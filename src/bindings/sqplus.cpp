#include "bindings/sqplus.h"

#include <new>

namespace vc1 {

// GLOBAL: VC1 0x004cfaa4
extern const char g_sqplusGetError[] = "sq_get*() failed (type error)";
// GLOBAL: VC1 0x004d12cc
extern const char kScriptGetWidth[] = "GetWidth";
// GLOBAL: VC1 0x004d12d8
extern const char kScriptGetHeight[] = "GetHeight";

// ECX=table, EAX=name. Map operator[]: the closure stored under name, or a
// null one inserted at its lower bound.
// FUNCTION: VC1 0x00401080
SquirrelObject& ScriptMethodTable::operator[](const char* const& name)
{
    ScriptMethod* found = LowerBound(name);
    if (found == End() || strcmp(name, found->name) < 0) {
        ScriptMethod record(name);
        ScriptMethod* slot = InsertSlot(found);
        *slot = record;
        return slot->method;
    }
    return found->method;
}

// EAX=table, RET 4. Lower bound of name in the name-sorted table.
// FUNCTION: VC1 0x00401430
ScriptMethod* ScriptMethodTable::LowerBound(const char* const& name)
{
    ScriptMethod* first;
    ScriptMethod* last;
    ScriptMethod* middle;

    first = Begin();
    last = End();
    while (first != last) {
        middle = first + (last - first) / 2;
        if (strcmp(middle->name, name) < 0) {
            first = middle + 1;
        }
        else {
            last = middle;
        }
    }
    return first;
}

// ECX=table, EAX=pos. Grows by one record (inexact), default-constructs the
// new capacity tail and opens a 0x10-byte gap at pos.
// FUNCTION: VC1 0x00401650
ScriptMethod* ScriptMethodTable::InsertSlot(ScriptMethod* pos)
{
    unsigned offset;
    unsigned old;
    unsigned now;

    offset = static_cast<unsigned>(reinterpret_cast<unsigned char*>(pos)
        - reinterpret_cast<unsigned char*>(items.begin));
    old = items.capacity / sizeof(ScriptMethod);
    GrowCountedBuffer(&items,
        static_cast<int>((items.size / sizeof(ScriptMethod) + 1) * sizeof(ScriptMethod)), 0);
    now = items.capacity / sizeof(ScriptMethod);
    if (now > old) {
        ConstructScriptMethods(Begin() + old, Begin() + now);
    }
    return static_cast<ScriptMethod*>(InsertCountedBytes(&items,
        reinterpret_cast<unsigned char*>(items.begin) + offset, sizeof(ScriptMethod)));
}

// ECX=first, EAX=last.
// FUNCTION: VC1 0x00401780
void DestroyScriptMethods(ScriptMethod* first, ScriptMethod* last)
{
    for (; first < last; ++first) {
        first->~ScriptMethod();
    }
}

// ECX=first, ESI=last.
// FUNCTION: VC1 0x004017e0
void ConstructScriptMethods(ScriptMethod* first, ScriptMethod* last)
{
    for (; first < last; ++first) {
        new (first) ScriptMethod();
    }
}

// EAX=v, ECX=idx. SqPlus Get(TypeWrapper<int>) with sq_getinteger inlined.
// FUNCTION: VC1 0x00401af0
int Get(TypeWrapper<int>, HSQUIRRELVM v, int idx)
{
    SQInteger i;
    if (SQ_FAILED(sq_getinteger(v, idx, &i))) {
        KdLog(g_sqplusErrorFormat, g_sqplusGetError);
    }
    return i;
}

// ESI=v, EBX=value. Index -> string array.
// FUNCTION: VC1 0x0041a1c0
void Push(HSQUIRRELVM v, const KdStringList& value)
{
    const KdString* strings = reinterpret_cast<const KdString*>(value.items.begin);
    unsigned i;

    sq_newarray(v, value.items.size / sizeof(KdString));
    for (i = 0; i < value.items.size / sizeof(KdString); ++i) {
        sq_pushinteger(v, static_cast<SQInteger>(i));
        sq_pushstring(v, strings[i].c_str(), -1);
        sq_set(v, -3);
    }
}

}
