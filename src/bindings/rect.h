#ifndef VC1_BINDINGS_RECT_H
#define VC1_BINDINGS_RECT_H
#include "squirrel_object.h"
#include "memory/fixed_pool.h"

namespace vc1 {

class KdString;

// Inferred by-value min/max: RectF::Union/Intersect spill both float operands
// before comparing. Ties return the first argument.
template<class T> inline T KdMax(T a, T b) { return a < b ? b : a; }
template<class T> inline T KdMin(T a, T b) { return b < a ? b : a; }

// Inferred name. Script class "Rect" (0x004cfb4c); SqPlus ClassType at
// 0x004ea81c. Script fields X/Y/Width/Height (0x004d7400-0x004d7410).
// Pool pools[4]; the release hook is folded with Vector4's (0x0041dd60).
// The default constructor stores height, width, y, x in that order
// (construct 0x00420070, ConstructArgs 0x004bdf10).
struct Rect
{
    SQInteger x;
    SQInteger y;
    SQInteger width;
    SQInteger height;

    Rect() { x = y = width = height = 0; }
    Rect(SQInteger ax, SQInteger ay, SQInteger aw, SQInteger ah)
        : x(ax), y(ay), width(aw), height(ah) {}
    // KdObjArray element copy.
    void Assign(const Rect& source) { *this = source; }

    VC1_SIZE_CLASS_NEW(Rect)

    bool Union(const Rect& other);                     // "Union"
    bool IsEmpty() const;                              // "IsEmpty"
    bool IsPointIn(SQInteger px, SQInteger py) const;  // "IsPointIn"
    bool Intersect(const Rect& other);                 // "Intersect"
    bool Overlaps(const Rect& other) const;
    KdString& ToString() const;                        // "_tostring"

    static SQInteger ConstructArgs(HSQUIRRELVM vm); // "constructor" with typemask "*"
};

typedef char Vc1RectSizeCheck[sizeof(Rect) == 16 ? 1 : -1];

}

DECLARE_INSTANCE_TYPE_NAME(vc1::Rect, Rect)

// SqPlus instantiations for Rect.

// TEMPLATE: VC1 0x00401510
// ??$Set@URect@vc1@@@SquirrelObject@@QAEAAV0@AAURect@vc1@@@Z

// TEMPLATE: VC1 0x0041dd60 FOLDED
// ?release@?$ConstructReleaseClass@URect@vc1@@@SqPlus@@SAHPAXH@Z

// TEMPLATE: VC1 0x0041e200
// ??0?$SQClassDef@URect@vc1@@USQNoBaseClass@SqPlus@@@SqPlus@@QAE@PBD@Z

// TEMPLATE: VC1 0x0041e3b0 FOLDED
// ??$PostConstruct@URect@vc1@@@SqPlus@@YAHPAUSQVM@@PAURect@vc1@@P6AHPAXH@Z@Z

// TEMPLATE: VC1 0x0041ec60
// ??$staticFuncVarArgs@P6AHPAUSQVM@@@Z@?$SQClassDefBase@URect@vc1@@USQNoBaseClass@SqPlus@@@SqPlus@@QAEAAU01@P6AHPAUSQVM@@@ZPBD2@Z

// TEMPLATE: VC1 0x0041ece0
// ??$var@H@?$SQClassDefBase@URect@vc1@@USQNoBaseClass@SqPlus@@@SqPlus@@QAEAAU01@PQRect@vc1@@HPBDW4VarAccessType@1@@Z

// TEMPLATE: VC1 0x0041f990
// ??$RegisterClassType@URect@vc1@@@SqPlus@@YA?AVSquirrelObject@@PAUSQVM@@PBD1@Z

// TEMPLATE: VC1 0x00420070
// ?construct@?$ConstructReleaseClass@URect@vc1@@@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00422ea0
// ??$RegisterInstanceVariable@H@SqPlus@@YAXAAVSquirrelObject@@PAUClassTypeBase@0@PAHPBDW4VarAccessType@0@@Z

// TEMPLATE: VC1 0x00422f40
// ??$RegisterInstance@URect@vc1@@P812@BEAAVKdString@2@XZ@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAURect@vc1@@P834@BEAAVKdString@4@XZPBD@Z

// TEMPLATE: VC1 0x00422ff0
// ??$RegisterInstance@URect@vc1@@P812@BE_NXZ@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAURect@vc1@@P834@BE_NXZPBD@Z

// TEMPLATE: VC1 0x004230a0
// ??$RegisterInstance@URect@vc1@@P812@AE_NABU12@@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAURect@vc1@@P834@AE_NABU34@@ZPBD@Z

// TEMPLATE: VC1 0x00423150
// ??$RegisterInstance@URect@vc1@@P812@BE_NHH@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAURect@vc1@@P834@BE_NHH@ZPBD@Z

// TEMPLATE: VC1 0x00423720
// ??$sq_typeof@URect@vc1@@@SqPlus@@YAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424470 FOLDED
// ?Dispatch@?$DirectCallInstanceMemberFunction@URect@vc1@@P812@BEAAVKdString@2@XZ@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004249b0
// ?Dispatch@?$DirectCallInstanceMemberFunction@URect@vc1@@P812@AE_NABU12@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424a00
// ?Dispatch@?$DirectCallInstanceMemberFunction@URect@vc1@@P812@BE_NHH@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424a50 FOLDED
// ?Dispatch@?$DirectCallInstanceMemberFunction@URect@vc1@@P812@BE_NXZ@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424c00 FOLDED
// ??0?$DirectCallInstanceFuncPicker@URect@vc1@@P812@AE_NABU12@@Z@SqPlus@@QAE@PAUSQVM@@@Z

// TEMPLATE: VC1 0x00428d50
// ??$GetInstance@URect@vc1@@$0A@@SqPlus@@YAPAURect@vc1@@PAUSQVM@@H@Z

// TEMPLATE: VC1 0x00401a70
// ??$GetInstance@URect@vc1@@$00@SqPlus@@YAPAURect@vc1@@PAUSQVM@@H@Z

#endif
