#include "matrix3.h"
#include "value_types.h"
#include "memory/fixed_pool.h"

namespace vc1 {

static NativeTypeInfo g_matrix3Type;
static unsigned g_matrix3TypeReady;
static const int kMatrix3TypeSpec = 0;

static void IdentityMatrix3(Matrix3* matrix)
{
    matrix->m[0] = 1.0f;
    matrix->m[1] = 0.0f;
    matrix->m[2] = 0.0f;
    matrix->m[3] = 0.0f;
    matrix->m[4] = 1.0f;
    matrix->m[5] = 0.0f;
    matrix->m[6] = 0.0f;
    matrix->m[7] = 0.0f;
    matrix->m[8] = 1.0f;
}

// SQRELEASEHOOK; size unused. Puts 36 bytes back on pools[9] / 0x004ed47c.
// FUNCTION: VC1 0x0041dd20
SQInteger Matrix3::ReleaseHook(SQUserPointer object, SQInteger size)
{
    (void)size;
    if (object) {
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Matrix3))]->Release(object);
        g_sizeClassHeap.releasedBytes += sizeof(Matrix3);
    }
    return 0;
}

// this in ESI. Name 0x004d1920.
// FUNCTION: VC1 0x0041e050
NativeClassBinding* Matrix3::CreateBinding(NativeClassBinding* self)
{
    return CreateValueBinding(self, "Matrix3", Matrix3::RegisterClass);
}

// FUNCTION: VC1 0x0041e860
NativeClassBinding* Matrix3::BindArgConstructor(NativeClassBinding* self)
{
    return BindStarConstructor(self, Matrix3::ConstructArgs);
}

// Once-flag 0x004ed838, type info 0x004ed824, spec 0x004d1928, name 0x004d1920.
// FUNCTION: VC1 0x0041f570
SquirrelObject* Matrix3::RegisterClass(SquirrelObject* out, HSQUIRRELVM vm,
    const SQChar* className)
{
    return RegisterValueClass(out, vm, className, &g_matrix3Type, &g_matrix3TypeReady,
        &kMatrix3TypeSpec, "Matrix3", Matrix3::Constructor, Matrix3::TypeOf);
}

// Registered from 0x0041f570 as "constructor" (0x004d2f10) with null typemask.
// Writes column-major identity. Arg-taking factory is 0x004be540.
// FUNCTION: VC1 0x0041ff30
SQInteger Matrix3::Constructor(HSQUIRRELVM vm)
{
    g_sizeClassHeap.allocatedBytes += sizeof(Matrix3);
    Matrix3* const matrix = static_cast<Matrix3*>(
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Matrix3))]->Allocate());
    if (matrix) {
        IdentityMatrix3(matrix);
    }
    return BindNativeInstance(vm, matrix, Matrix3::ReleaseHook);
}

// FUNCTION: VC1 0x004236c0
SQInteger Matrix3::TypeOf(HSQUIRRELVM vm)
{
    sq_pushstring(vm, "Matrix3", -1);
    return 1;
}

// Registered from 0x0041e860 as "constructor" with typemask "*" (0x004d6b14).
// extra = gettop-1; identity when extra==0. Error string 0x004d6a5c.
// extra 1-3 copy/from-vectors via unmapped helpers; identity kept as fallback.
// FUNCTION: VC1 0x004be540
SQInteger Matrix3::ConstructArgs(HSQUIRRELVM vm)
{
    g_sizeClassHeap.allocatedBytes += sizeof(Matrix3);
    Matrix3* const matrix = static_cast<Matrix3*>(
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Matrix3))]->Allocate());
    const unsigned extra = static_cast<unsigned>(sq_gettop(vm) - 1);
    if (extra > 3) {
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Matrix3))]->Release(matrix);
        g_sizeClassHeap.releasedBytes += sizeof(Matrix3);
        return sq_throwerror(vm, "constructMatrix3: invalid arguments");
    }
    if (matrix) {
        IdentityMatrix3(matrix);
    }
    return BindNativeInstance(vm, matrix, Matrix3::ReleaseHook);
}

}
