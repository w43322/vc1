#include "rectf.h"
#include "vector4.h"
#include "value_types.h"
#include "memory/fixed_pool.h"

namespace vc1 {

static NativeTypeInfo g_rectFType;
static unsigned g_rectFTypeReady;
static const int kRectFTypeSpec = 0;

static void ZeroRectF(RectF* rect)
{
    rect->left = 0.0f;
    rect->top = 0.0f;
    rect->right = 0.0f;
    rect->bottom = 0.0f;
}

// this in ESI. Name 0x004d4ffc.
// FUNCTION: VC1 0x0041e290
NativeClassBinding* RectF::CreateBinding(NativeClassBinding* self)
{
    return CreateValueBinding(self, "RectF", RectF::RegisterClass);
}

// FUNCTION: VC1 0x0041ed40
NativeClassBinding* RectF::BindArgConstructor(NativeClassBinding* self)
{
    return BindStarConstructor(self, RectF::ConstructArgs);
}

// Once-flag 0x004ed858, type info 0x004ed844, spec 0x004cfba4, name 0x004d4ffc.
// FUNCTION: VC1 0x0041faf0
SquirrelObject* RectF::RegisterClass(SquirrelObject* out, HSQUIRRELVM vm,
    const SQChar* className)
{
    return RegisterValueClass(out, vm, className, &g_rectFType, &g_rectFTypeReady,
        &kRectFTypeSpec, "RectF", RectF::Constructor, RectF::TypeOf);
}

// Registered from 0x0041faf0 as "constructor" (0x004d2f10) with null typemask.
// Zeros four floats. Release hook 0x0041dd60 (Vector4). Factory 0x004bdd60.
// FUNCTION: VC1 0x004200c0
SQInteger RectF::Constructor(HSQUIRRELVM vm)
{
    g_sizeClassHeap.allocatedBytes += sizeof(RectF);
    RectF* const rect = static_cast<RectF*>(
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(RectF))]->Allocate());
    if (rect) {
        ZeroRectF(rect);
    }
    return BindNativeInstance(vm, rect, Vector4::ReleaseHook);
}

// FUNCTION: VC1 0x00423740
SQInteger RectF::TypeOf(HSQUIRRELVM vm)
{
    sq_pushstring(vm, "RectF", -1);
    return 1;
}

// Registered from 0x0041ed40 as "constructor" with typemask "*" (0x004d6b14).
// extra = gettop-1; extra==1 copies another RectF; extra==4 four floats. Error 0x004d6af0.
// FUNCTION: VC1 0x004bdd60
SQInteger RectF::ConstructArgs(HSQUIRRELVM vm)
{
    g_sizeClassHeap.allocatedBytes += sizeof(RectF);
    RectF* const rect = static_cast<RectF*>(
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(RectF))]->Allocate());
    const unsigned extra = static_cast<unsigned>(sq_gettop(vm) - 1);
    if (extra != 0 && extra != 1 && extra != 4) {
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(RectF))]->Release(rect);
        g_sizeClassHeap.releasedBytes += sizeof(RectF);
        return sq_throwerror(vm, "constructRectF: invalid arguments");
    }
    if (rect) {
        ZeroRectF(rect);
        if (extra == 4) {
            rect->left = NumericFloat(vm, 2);
            rect->top = NumericFloat(vm, 3);
            rect->right = NumericFloat(vm, 4);
            rect->bottom = NumericFloat(vm, 5);
        }
    }
    return BindNativeInstance(vm, rect, Vector4::ReleaseHook);
}

}
