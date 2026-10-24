#include "rect.h"
#include "value_types.h"
#include "kd/factory.h"

#include <new>

namespace vc1 {

// Inferred name. ECX=this, EAX=other. Strict overlap of two rectangles that
// share no more than an edge.
// FUNCTION: VC1 0x00402440
bool Rect::Overlaps(const Rect& other) const
{
    return x < other.width + other.x && y < other.height + other.y
        && width + x > other.x && height + y > other.y;
}

// Bound as "Union" (0x004d7418) via 0x004230a0 from 0x004c1419 (dispatcher
// 0x004249b0). this in ECX, other on the stack; RET 4. In-place xywh union.
// FUNCTION: VC1 0x00409b10
bool Rect::Union(const Rect& other)
{
    const SQInteger otherRight = other.width + other.x;
    const SQInteger thisRight = width + x;
    const SQInteger right = otherRight < thisRight ? thisRight : otherRight;
    const SQInteger otherBottom = other.height + other.y;
    const SQInteger thisBottom = height + y;
    const SQInteger bottom = otherBottom < thisBottom ? thisBottom : otherBottom;
    const SQInteger left = x < other.x ? x : other.x;
    const SQInteger top = y < other.y ? y : other.y;
    y = top;
    x = left;
    width = right - left;
    height = bottom - top;
    return !IsEmpty();
}

// Bound as "IsEmpty" (0x004d17dc) via 0x00422ff0 (dispatcher 0x00424a50).
// FUNCTION: VC1 0x00409b90
bool Rect::IsEmpty() const
{
    return width <= 0 || height <= 0;
}

// Bound as "IsPointIn" (0x004d742c) via 0x00423150 (dispatcher 0x00424a00).
// this in ECX; px, py on the stack; RET 8. Right and bottom edges exclusive.
// FUNCTION: VC1 0x0040aa30
bool Rect::IsPointIn(SQInteger px, SQInteger py) const
{
    return x <= px && px < x + width && y <= py && py < y + height;
}

// Bound as "Intersect" (0x004d7420) via 0x004230a0 from 0x004c1435. No
// Ghidra function at this VA. In-place xywh intersection.
// FUNCTION: VC1 0x0041d9a0
bool Rect::Intersect(const Rect& other)
{
    const SQInteger otherRight = other.width + other.x;
    const SQInteger thisRight = width + x;
    const SQInteger right = thisRight < otherRight ? thisRight : otherRight;
    const SQInteger otherBottom = other.height + other.y;
    const SQInteger thisBottom = height + y;
    const SQInteger bottom = thisBottom < otherBottom ? thisBottom : otherBottom;
    const SQInteger left = other.x < x ? x : other.x;
    const SQInteger top = other.y < y ? y : other.y;
    y = top;
    x = left;
    width = right - left;
    height = bottom - top;
    return !IsEmpty();
}

// Bound as "_tostring" via 0x00422f40 (dispatcher 0x00424470). this in ECX.
// Function-local static KdString (0x004ee4a8, guard 0x004ee4b8) with 0x40
// reserved characters. The format is shared with RectF::ToString.
// FUNCTION: VC1 0x0041da20
KdString& Rect::ToString() const
{
    // GLOBAL: VC1 0x004ee4a8
    static KdString text(0x40, '\0');
    // STRING: VC1 0x004d7478
    text.Format("Rect(%d, %d, %d, %d)", x, y, width, height);
    return text;
}

// Registered from 0x0041ec60 as "constructor" with typemask "*" (0x004d6b14).
// gettop 1: zeros, 2: copy of a Rect, 5: four ints; anything else throws.
// Error string 0x004d6acc.
// FUNCTION: VC1 0x004bdf10
SQInteger Rect::ConstructArgs(HSQUIRRELVM vm)
{
    StackHandler sa(vm);
    Rect* const rect = static_cast<Rect*>(Rect::operator new(sizeof(Rect)));
    const int count = sa.GetParamCount();
    if (count == 1) {
        ::new (rect) Rect();
        return SqPlus::PostConstruct<Rect>(vm, rect, SqPlus::ConstructReleaseClass<Rect>::release);
    }
    if (count == 2) {
        Rect* const source = SqPlus::GetInstance<Rect, false>(vm, 2);
        if (source) {
            ::new (rect) Rect(*source);
            return SqPlus::PostConstruct<Rect>(vm, rect, SqPlus::ConstructReleaseClass<Rect>::release);
        }
    }
    else if (count == 5) {
        ::new (rect) Rect(sa.GetInt(2), sa.GetInt(3), sa.GetInt(4), sa.GetInt(5));
        return SqPlus::PostConstruct<Rect>(vm, rect, SqPlus::ConstructReleaseClass<Rect>::release);
    }
    Rect::operator delete(rect);
    // STRING: VC1 0x004d6acc
    return sa.ThrowError("constructRect: invalid arguments");
}

}
