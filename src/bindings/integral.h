#ifndef VC1_BINDINGS_INTEGRAL_H
#define VC1_BINDINGS_INTEGRAL_H
#include "squirrel_object.h"
#include "memory/fixed_pool.h"
#include "kd/factory.h"

namespace vc1 {

void AssignSamples(KdFactory& values, const SQFloat* first, const SQFloat* last);

// Inferred member names. Script class "IntegralF" (0x004d75ac); SqPlus
// ClassType at 0x004ee228. Sliding window of the last
// integralSize floats pushed with PushValue. The script constructor is the
// default SqPlus construct (no staticFuncVarArgs). Pool pools[5].
// SIZE 0x14
struct IntegralF
{
    unsigned integralSize; // 0x00
    KdFactory values;      // 0x04 float samples, size in bytes

    IntegralF() : integralSize(1) {}
    // SqPlus ClassTypeCopyImpl<IntegralF>::copy (0x00427350) is this
    // assignment: the size, then the samples through AssignSamples.
    IntegralF& operator=(const IntegralF& other)
    {
        integralSize = other.integralSize;
        AssignSamples(values, other.First(), other.Last());
        return *this;
    }

    VC1_SIZE_CLASS_NEW(IntegralF)

    void SetIntegralSize(unsigned size);  // "SetIntegralSize"
    void PushValue(SQFloat value);        // "PushValue"
    void Clear();                         // "Clear"
    SQFloat GetAverageValue() const;      // "GetAverageValue"
    SQFloat GetIntegralValue() const;     // "GetIntegralValue"

    const SQFloat* First() const { return reinterpret_cast<const SQFloat*>(values.begin); }
    const SQFloat* Last() const
    {
        return reinterpret_cast<const SQFloat*>(
            reinterpret_cast<const unsigned char*>(values.begin) + values.size);
    }
};

typedef char Vc1IntegralFSizeCheck[sizeof(IntegralF) == 0x14 ? 1 : -1];

SQFloat SumFloats(const SQFloat* first, const SQFloat* last, SQFloat sum);

}

DECLARE_INSTANCE_TYPE_NAME(vc1::IntegralF, IntegralF)

// SqPlus instantiations for IntegralF.

// TEMPLATE: VC1 0x0041e320
// ??0?$SQClassDef@UIntegralF@vc1@@USQNoBaseClass@SqPlus@@@SqPlus@@QAE@PBD@Z

// TEMPLATE: VC1 0x0041fc50
// ??$RegisterClassType@UIntegralF@vc1@@@SqPlus@@YA?AVSquirrelObject@@PAUSQVM@@PBD1@Z

// TEMPLATE: VC1 0x00420110
// ?construct@?$ConstructReleaseClass@UIntegralF@vc1@@@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00420230
// ?release@?$ConstructReleaseClass@UIntegralF@vc1@@@SqPlus@@SAHPAXH@Z

// TEMPLATE: VC1 0x00422b70 FOLDED
// ??$RegisterInstance@UIntegralF@vc1@@P812@BEMXZ@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUIntegralF@vc1@@P834@BEMXZPBD@Z

// TEMPLATE: VC1 0x00423410
// ??$RegisterInstance@UIntegralF@vc1@@P812@AEXI@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUIntegralF@vc1@@P834@AEXI@ZPBD@Z

// TEMPLATE: VC1 0x004234c0
// ??$RegisterInstance@UIntegralF@vc1@@P812@AEXM@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUIntegralF@vc1@@P834@AEXM@ZPBD@Z

// TEMPLATE: VC1 0x00423570
// ??$RegisterInstance@UIntegralF@vc1@@P812@AEXXZ@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUIntegralF@vc1@@P834@AEXXZPBD@Z

// TEMPLATE: VC1 0x00423760
// ??$sq_typeof@UIntegralF@vc1@@@SqPlus@@YAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00423810
// ??$PostConstruct@UIntegralF@vc1@@@SqPlus@@YAHPAUSQVM@@PAUIntegralF@vc1@@P6AHPAXH@Z@Z

// TEMPLATE: VC1 0x00424060 FOLDED
// ?Dispatch@?$DirectCallInstanceMemberFunction@UIntegralF@vc1@@P812@AEXXZ@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424af0 FOLDED
// ?Dispatch@?$DirectCallInstanceMemberFunction@UIntegralF@vc1@@P812@AEXI@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424b40
// ?Dispatch@?$DirectCallInstanceMemberFunction@UIntegralF@vc1@@P812@AEXM@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424b90 FOLDED
// ?Dispatch@?$DirectCallInstanceMemberFunction@UIntegralF@vc1@@P812@BEMXZ@SqPlus@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x00426250 FOLDED
// ??$Call@UIntegralF@vc1@@I@?$ReturnSpecialization@X@SqPlus@@SAHAAUIntegralF@vc1@@P823@AEXI@ZPAUSQVM@@H@Z

#endif
