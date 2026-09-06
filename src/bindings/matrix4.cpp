#include "matrix4.h"
#include "value_types.h"
#include "memory/fixed_pool.h"

namespace vc1 {

static NativeTypeInfo g_matrix4Type;
static unsigned g_matrix4TypeReady;
static const int kMatrix4TypeSpec = 0;

// Original writes 1 then zeros then diagonal 1s via FLD1/FLDZ (0x0041ffb4 / helper 0x00474090).
static void IdentityMatrix4(Matrix4* matrix)
{
    matrix->m[0] = 1.0f;
    matrix->m[1] = 0.0f;
    matrix->m[2] = 0.0f;
    matrix->m[3] = 0.0f;
    matrix->m[4] = 0.0f;
    matrix->m[5] = 1.0f;
    matrix->m[6] = 0.0f;
    matrix->m[7] = 0.0f;
    matrix->m[8] = 0.0f;
    matrix->m[9] = 0.0f;
    matrix->m[10] = 1.0f;
    matrix->m[11] = 0.0f;
    matrix->m[12] = 0.0f;
    matrix->m[13] = 0.0f;
    matrix->m[14] = 0.0f;
    matrix->m[15] = 1.0f;
}

// SQRELEASEHOOK; size unused. Puts 64 bytes back on pools[16] / 0x004ed498.
// FUNCTION: VC1 0x0041dd40
SQInteger Matrix4::ReleaseHook(SQUserPointer object, SQInteger size)
{
    (void)size;
    if (object) {
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Matrix4))]->Release(object);
        g_sizeClassHeap.releasedBytes += sizeof(Matrix4);
    }
    return 0;
}

// this in ESI. Name 0x004d7598.
// FUNCTION: VC1 0x0041e0e0
NativeClassBinding* Matrix4::CreateBinding(NativeClassBinding* self)
{
    return CreateValueBinding(self, "Matrix4", Matrix4::RegisterClass);
}

// FUNCTION: VC1 0x0041e9c0
NativeClassBinding* Matrix4::BindArgConstructor(NativeClassBinding* self)
{
    return BindStarConstructor(self, Matrix4::ConstructArgs);
}

// Once-flag 0x004ed870, type info 0x004ed85c, spec 0x004d75c0, name 0x004d7598.
// FUNCTION: VC1 0x0041f6d0
SquirrelObject* Matrix4::RegisterClass(SquirrelObject* out, HSQUIRRELVM vm,
    const SQChar* className)
{
    return RegisterValueClass(out, vm, className, &g_matrix4Type, &g_matrix4TypeReady,
        &kMatrix4TypeSpec, "Matrix4", Matrix4::Constructor, Matrix4::TypeOf);
}

// Registered from 0x0041f6d0 as "constructor" (0x004d2f10) with null typemask.
// Writes column-major identity ([0],[20],[40],[60] = 1). Arg-taking factory is 0x004be3b0.
// FUNCTION: VC1 0x0041ff90
SQInteger Matrix4::Constructor(HSQUIRRELVM vm)
{
    g_sizeClassHeap.allocatedBytes += sizeof(Matrix4);
    Matrix4* const matrix = static_cast<Matrix4*>(
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Matrix4))]->Allocate());
    if (matrix) {
        IdentityMatrix4(matrix);
    }
    return BindNativeInstance(vm, matrix, Matrix4::ReleaseHook);
}

// FUNCTION: VC1 0x004236e0
SQInteger Matrix4::TypeOf(HSQUIRRELVM vm)
{
    sq_pushstring(vm, "Matrix4", -1);
    return 1;
}

// Registered from 0x0041e9c0 as "constructor" with typemask "*" (0x004d6b14).
// extra = gettop-1; switch cases 1,2,3,5 so extra 3 is invalid. Error string 0x004d6a80.
// FUNCTION: VC1 0x004be3b0
SQInteger Matrix4::ConstructArgs(HSQUIRRELVM vm)
{
    g_sizeClassHeap.allocatedBytes += sizeof(Matrix4);
    Matrix4* const matrix = static_cast<Matrix4*>(
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Matrix4))]->Allocate());
    const unsigned extra = static_cast<unsigned>(sq_gettop(vm) - 1);
    if (extra == 3 || extra > 4) {
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Matrix4))]->Release(matrix);
        g_sizeClassHeap.releasedBytes += sizeof(Matrix4);
        return sq_throwerror(vm, "constructMatrix4: invalid arguments");
    }
    if (matrix) {
        IdentityMatrix4(matrix);
    }
    return BindNativeInstance(vm, matrix, Matrix4::ReleaseHook);
}

}
