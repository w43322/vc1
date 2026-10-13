#include "host/runtime.h"

#include <squirrel.h>
#include <sqstdaux.h>
#include <sqstdblob.h>
#include <sqstdio.h>
#include <sqstdmath.h>
#include <sqstdstring.h>
#include <sqstdsystem.h>

namespace vc1 {

static void BindHostClasses();

// STUB: VC1 0x00402690
static void RegisterPrintBinding(HSQUIRRELVM vm, void* fn)
{
    (void)vm;
    (void)fn;
}

// STUB: VC1 0x00402770
static void RegisterPrintAltBinding(HSQUIRRELVM vm, void* fn)
{
    (void)vm;
    (void)fn;
}

// STUB: VC1 0x00402850
static void RegisterStdExtra0()
{
}

// STUB: VC1 0x004028f0
static void RegisterStdExtra1()
{
}

// STUB: VC1 0x00402990
static void RegisterStdExtra2()
{
}

// STUB: VC1 0x00402a30
static void RegisterStdExtra3()
{
}

// STUB: VC1 0x00402ad0
static void RegisterStdExtra4()
{
}

// EAX=argc, EBX=argv from RunApp's g_runScratch0/1. Args array then root["ARGS"].
// FUNCTION: VC1 0x00463b40
void InitScriptEngine(int argc, char** argv)
{
    int i;

    InitSizeClassPools();
    SquirrelVM::Init();
    BindHostClasses();
    InitHostServices();

    SquirrelObject args = SquirrelVM::CreateArray(argc);
    for (i = 0; i < argc; ++i) {
        args.SetValue(i, argv[i]);
    }
    // STRING: VC1 0x004d6384
    SquirrelVM::GetRootTable().SetValue("ARGS", args);
}

// Original 0x004bdc50 registers print/error closures then five extra stdlib batches on SquirrelVM::_VM.
// FUNCTION: VC1 0x004bdc50
void RegisterStdBindings()
{
    RegisterPrintBinding(SquirrelVM::_VM, 0);
    RegisterPrintAltBinding(SquirrelVM::_VM, 0);
    RegisterStdExtra0();
    RegisterStdExtra1();
    RegisterStdExtra2();
    RegisterStdExtra3();
    RegisterStdExtra4();
}

// Color/Vector/Matrix/Rect native class table. Called from InitScriptEngine.
// STUB: VC1 0x004bf610
static void BindHostClasses()
{
}

}
