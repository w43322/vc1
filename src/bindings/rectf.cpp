#include "rectf.h"
#include "value_types.h"
#include "vector2.h"
#include "kd/factory.h"

#include <new>

namespace vc1 {

// Bound as "Intersect" (0x004d7420) via 0x00423360 from 0x004c150d
// (dispatcher 0x00424aa0). this in ECX, other on the stack; RET 4.
// FUNCTION: VC1 0x0040a480
bool RectF::Intersect(const RectF& other)
{
    const SQFloat right = KdMin(other.width + other.x, x + width);
    const SQFloat bottom = KdMin(other.height + other.y, y + height);
    const SQFloat left = KdMax(other.x, x);
    const SQFloat top = KdMax(other.y, y);
    x = left;
    y = top;
    width = right - left;
    height = bottom - top;
    return !IsEmpty();
}

// Bound as "IsEmpty" (0x004d17dc) via 0x004232b0 (dispatcher 0x00424a50).
// FUNCTION: VC1 0x0040a5a0
bool RectF::IsEmpty() const
{
    return width <= 0.0f || height <= 0.0f;
}

// Inferred name. Grows the rect to cover p (an empty rect becomes the 1x1
// cell at p); right and bottom stay exclusive. The original only calls it on
// TileObject's static bounds (0x004ee3b8), so LTCG bound this to that
// address. ECX=&p.
// FUNCTION: VC1 0x0040aa70
void RectF::Include(const Vector2& p)
{
    if (width <= 0.0f || height <= 0.0f) {
        x = p.x;
        y = p.y;
        width = 1.0f;
        height = 1.0f;
        return;
    }
    if (p.x < x) {
        x = p.x;
    }
    if (p.y < y) {
        y = p.y;
    }
    if (x + width <= p.x) {
        width = p.x + 1.0f - x;
    }
    if (y + height <= p.y) {
        height = p.y + 1.0f - y;
    }
}

// Bound as "Union" (0x004d7418) via 0x00423360 from 0x004c14f1. In-place
// xywh union.
// FUNCTION: VC1 0x00415ad0
bool RectF::Union(const RectF& other)
{
    const SQFloat right = KdMax(other.width + other.x, x + width);
    const SQFloat bottom = KdMax(other.height + other.y, y + height);
    const SQFloat left = KdMin(other.x, x);
    const SQFloat top = KdMin(other.y, y);
    x = left;
    y = top;
    width = right - left;
    height = bottom - top;
    return !IsEmpty();
}

// Bound as "_tostring" via 0x00423200 (dispatcher 0x00424470). this in ECX.
// Function-local static KdString (0x004ee494, guard 0x004ee4a4) with 0x40
// reserved characters. Reuses Rect's integer format (0x004d7478), truncating
// every field with _ftol (0x004c4060).
// FUNCTION: VC1 0x0041dae0
KdString& RectF::ToString() const
{
    // GLOBAL: VC1 0x004ee494
    static KdString text(0x40, '\0');
    text.Format("Rect(%d, %d, %d, %d)", static_cast<int>(x), static_cast<int>(y),
        static_cast<int>(width), static_cast<int>(height));
    return text;
}

// Registered from 0x0041ed40 as "constructor" with typemask "*" (0x004d6b14).
// gettop 1: zeros, 2: copy of a RectF, 5: four floats; anything else throws.
// Error 0x004d6af0.
// FUNCTION: VC1 0x004bdd60
SQInteger RectF::ConstructArgs(HSQUIRRELVM vm)
{
    StackHandler sa(vm);
    RectF* const rect = static_cast<RectF*>(RectF::operator new(sizeof(RectF)));
    const int count = sa.GetParamCount();
    if (count == 1) {
        ::new (rect) RectF();
        return SqPlus::PostConstruct<RectF>(vm, rect, SqPlus::ConstructReleaseClass<RectF>::release);
    }
    if (count == 2) {
        RectF* const source = SqPlus::GetInstance<RectF, false>(vm, 2);
        if (source) {
            ::new (rect) RectF(*source);
            return SqPlus::PostConstruct<RectF>(vm, rect, SqPlus::ConstructReleaseClass<RectF>::release);
        }
    }
    else if (count == 5) {
        ::new (rect) RectF(sa.GetFloat(2), sa.GetFloat(3), sa.GetFloat(4), sa.GetFloat(5));
        return SqPlus::PostConstruct<RectF>(vm, rect, SqPlus::ConstructReleaseClass<RectF>::release);
    }
    RectF::operator delete(rect);
    // STRING: VC1 0x004d6af0
    return sa.ThrowError("constructRectF: invalid arguments");
}

}
