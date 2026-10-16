#ifndef VC1_BINDINGS_COLOR_H
#define VC1_BINDINGS_COLOR_H
#include "squirrel_object.h"
#include "memory/fixed_pool.h"

namespace vc1 {

class KdString;

// Inferred name. Script class "Color" (0x004d7580); SqPlus ClassType at
// 0x004ee214. One ARGB dword in pools[1]; script constructs default to opaque
// black (construct 0x0041fe10). BindHostClasses also sets the shift/mask
// values and 141 named colours as class constants.
struct Color
{
    SQUnsignedInteger argb;

    Color() : argb(0xff000000) {}
    explicit Color(SQUnsignedInteger value) : argb(value) {}
    Color(unsigned char r, unsigned char g, unsigned char b)
        : argb((((0xffffff00 | r) << 8 | g) << 8) | b) {}
    Color(unsigned char a, unsigned char r, unsigned char g, unsigned char b)
        : argb((((static_cast<SQUnsignedInteger>(a) << 8 | r) << 8 | g) << 8) | b) {}

    VC1_SIZE_CLASS_NEW(Color)

    unsigned char GetA();
    unsigned char GetR();
    unsigned char GetG();
    unsigned char GetB();
    static SQUnsignedInteger MakeARGB(unsigned char a, unsigned char r, unsigned char g,
        unsigned char b);
    void SetValue(SQUnsignedInteger value);
    SQUnsignedInteger GetValue();
    KdString& ToString() const;              // "_tostring"
    SQInteger Cmp(const Color& other) const; // "_cmp"
    SQFloat GetBlue() const;
    SQFloat GetGreen() const;
    SQFloat GetRed() const;
    SQFloat GetAlpha() const;
    Color operator*(const Color& other) const; // per channel a * b / 255

    static SQInteger ConstructArgs(HSQUIRRELVM vm); // "constructor" with typemask "*"
};

typedef char Vc1ColorSizeCheck[sizeof(Color) == 4 ? 1 : -1];

}

DECLARE_INSTANCE_TYPE_NAME(vc1::Color, Color)

// SqPlus instantiations for Color. The four byte getters share one binder
// (0x004203e0) and the four float getters share Vector4's (0x00422b70).

// TEMPLATE: VC1 0x0041de10
// ??0?$SQClassDef@UColor@vc1@@USQNoBaseClass@SqPlus@@@SqPlus@@QAE@PBD@Z

// TEMPLATE: VC1 0x0041e3b0 FOLDED
// ??$PostConstruct@UColor@vc1@@@SqPlus@@YAHPAUSQVM@@PAUColor@vc1@@P6AHPAXH@Z@Z

// TEMPLATE: VC1 0x0041e440
// ??$staticFuncVarArgs@P6AHPAUSQVM@@@Z@?$SQClassDefBase@UColor@vc1@@USQNoBaseClass@SqPlus@@@SqPlus@@QAEAAU01@P6AHPAUSQVM@@@ZPBD2@Z

// TEMPLATE: VC1 0x0041eff0
// ??$RegisterClassType@UColor@vc1@@@SqPlus@@YA?AVSquirrelObject@@PAUSQVM@@PBD1@Z

// TEMPLATE: VC1 0x0041fe10
// ?construct@?$ConstructReleaseClass@UColor@vc1@@@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004201f0
// ?release@?$ConstructReleaseClass@UColor@vc1@@@SqPlus@@SAHPAXH@Z

// TEMPLATE: VC1 0x00420280
// ??$RegisterInstance@UColor@vc1@@P812@AEIXZ@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUColor@vc1@@P834@AEIXZPBD@Z

// TEMPLATE: VC1 0x00420330
// ??$RegisterInstance@UColor@vc1@@P812@AEXI@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUColor@vc1@@P834@AEXI@ZPBD@Z

// TEMPLATE: VC1 0x004203e0
// ??$RegisterInstance@UColor@vc1@@P812@AEEXZ@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUColor@vc1@@P834@AEEXZPBD@Z

// TEMPLATE: VC1 0x00420490
// ??$RegisterInstance@UColor@vc1@@P812@BEHABU12@@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUColor@vc1@@P834@BEHABU34@@ZPBD@Z

// TEMPLATE: VC1 0x00420540
// ??$RegisterInstance@UColor@vc1@@P812@BEAAVKdString@2@XZ@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUColor@vc1@@P834@BEAAVKdString@4@XZPBD@Z

// TEMPLATE: VC1 0x004205f0
// ??$Register@P6AIEEEE@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@P6AIEEEE@ZPBD@Z

// TEMPLATE: VC1 0x00422b70 FOLDED
// ??$RegisterInstance@UColor@vc1@@P812@BEMXZ@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUColor@vc1@@P834@BEMXZPBD@Z

// TEMPLATE: VC1 0x00423640
// ??$sq_typeof@UColor@vc1@@@SqPlus@@YAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004238d0
// ??$Register@P6AIEEEE@Z@SqPlus@@YAXPAUSQVM@@P6AIEEEE@ZPBD@Z

// TEMPLATE: VC1 0x00423e30
// ?Dispatch@?$DirectCallInstanceMemberFunction@UColor@vc1@@P812@AEIXZ@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00423e80
// ?Dispatch@?$DirectCallInstanceMemberFunction@UColor@vc1@@P812@AEEXZ@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00423ed0
// ?Dispatch@?$DirectCallInstanceMemberFunction@UColor@vc1@@P812@BEHABU12@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424470 FOLDED
// ?Dispatch@?$DirectCallInstanceMemberFunction@UColor@vc1@@P812@BEAAVKdString@2@XZ@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424af0 FOLDED
// ?Dispatch@?$DirectCallInstanceMemberFunction@UColor@vc1@@P812@AEXI@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424b90 FOLDED
// ?Dispatch@?$DirectCallInstanceMemberFunction@UColor@vc1@@P812@BEMXZ@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424eb0
// ?Dispatch@?$DirectCallFunction@P6AIEEEE@Z@SqPlus@@SAHPAUSQVM@@@Z

#endif
