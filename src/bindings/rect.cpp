#include "rect.h"
#include "vector4.h"
#include "value_types.h"
#include "memory/fixed_pool.h"

namespace vc1 {

static NativeTypeInfo g_rectType;
static unsigned g_rectTypeReady;
static const int kRectTypeSpec = 0;

static void ZeroRect(Rect* rect)
{
    rect->left = 0;
    rect->top = 0;
    rect->right = 0;
    rect->bottom = 0;
}

// this in ESI. Name 0x004cfb4c.
// FUNCTION: VC1 0x0041e200
NativeClassBinding* Rect::CreateBinding(NativeClassBinding* self)
{
    return CreateValueBinding(self, "Rect", Rect::RegisterClass);
}

// FUNCTION: VC1 0x0041ec60
NativeClassBinding* Rect::BindArgConstructor(NativeClassBinding* self)
{
    return BindStarConstructor(self, Rect::ConstructArgs);
}

// Once-flag 0x004ea844, type info 0x004ea81c, spec 0x004cfba4, name 0x004cfb4c.
// FUNCTION: VC1 0x0041f990
SquirrelObject* Rect::RegisterClass(SquirrelObject* out, HSQUIRRELVM vm,
    const SQChar* className)
{
    return RegisterValueClass(out, vm, className, &g_rectType, &g_rectTypeReady,
        &kRectTypeSpec, "Rect", Rect::Constructor, Rect::TypeOf);
}

// Registered from 0x0041f990 as "constructor" (0x004d2f10) with null typemask.
// Zeros four ints. Release hook 0x0041dd60 (Vector4). Factory 0x004bdf10.
// FUNCTION: VC1 0x00420070
SQInteger Rect::Constructor(HSQUIRRELVM vm)
{
    g_sizeClassHeap.allocatedBytes += sizeof(Rect);
    Rect* const rect = static_cast<Rect*>(
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Rect))]->Allocate());
    if (rect) {
        ZeroRect(rect);
    }
    return BindNativeInstance(vm, rect, Vector4::ReleaseHook);
}

// FUNCTION: VC1 0x00423720
SQInteger Rect::TypeOf(HSQUIRRELVM vm)
{
    sq_pushstring(vm, "Rect", -1);
    return 1;
}

// Registered from 0x0041ec60 as "constructor" with typemask "*" (0x004d6b14).
// extra = gettop-1; extra==1 copies another Rect (unmapped helper); extra==4 four ints.
// Other extras throw. Error string 0x004d6acc.
// FUNCTION: VC1 0x004bdf10
SQInteger Rect::ConstructArgs(HSQUIRRELVM vm)
{
    g_sizeClassHeap.allocatedBytes += sizeof(Rect);
    Rect* const rect = static_cast<Rect*>(
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Rect))]->Allocate());
    const unsigned extra = static_cast<unsigned>(sq_gettop(vm) - 1);
    if (extra != 0 && extra != 1 && extra != 4) {
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Rect))]->Release(rect);
        g_sizeClassHeap.releasedBytes += sizeof(Rect);
        return sq_throwerror(vm, "constructRect: invalid arguments");
    }
    if (rect) {
        ZeroRect(rect);
        if (extra == 4) {
            rect->left = NumericInt(vm, 2);
            rect->top = NumericInt(vm, 3);
            rect->right = NumericInt(vm, 4);
            rect->bottom = NumericInt(vm, 5);
        }
    }
    return BindNativeInstance(vm, rect, Vector4::ReleaseHook);
}

}
