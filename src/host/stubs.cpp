#include "host/runtime.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

// Unmapped callees of this round's functions. No FUNCTION markers.
namespace vc1 {

void* UnmappedAllocEvent()
{
    return 0;
}

void UnmappedVectorGrow(int newSize, char zero)
{
    (void)newSize;
    (void)zero;
}

void* UnmappedVectorInsert(void* factory, void* pos, int bytes)
{
    (void)factory;
    (void)bytes;
    return pos;
}

unsigned long long UnmappedQpcNow()
{
    return 0;
}

void UnmappedConstructDisplayBase(void* self)
{
    (void)self;
}

void* UnmappedDisplayBaseQueryInterface(void* self, const char** iid)
{
    (void)self;
    (void)iid;
    return 0;
}

void UnmappedConstructCompoundBase(void* self)
{
    (void)self;
}

void UnmappedRegisterPrint(HSQUIRRELVM vm, void* fn)
{
    (void)vm;
    (void)fn;
}

void UnmappedRegisterPrintAlt(HSQUIRRELVM vm, void* fn)
{
    (void)vm;
    (void)fn;
}

void UnmappedRegisterExtra0()
{
}

void UnmappedRegisterExtra1()
{
}

void UnmappedRegisterExtra2()
{
}

void UnmappedRegisterExtra3()
{
}

void UnmappedRegisterExtra4()
{
}

void UnmappedBindHostClasses()
{
}

void UnmappedCompilerError()
{
}

void UnmappedSharedStateCtor(void* ss)
{
    (void)ss;
}

void __stdcall UnmappedSharedStateInit(void* ss)
{
    (void)ss;
}

void UnmappedVmCtor(HSQUIRRELVM vm, void* ss)
{
    (void)vm;
    (void)ss;
}

int __stdcall UnmappedVmInit(HSQUIRRELVM vm, HSQUIRRELVM friendvm)
{
    (void)vm;
    (void)friendvm;
    return 0;
}

void UnmappedVmDelete(HSQUIRRELVM vm)
{
    (void)vm;
}

void UnmappedWrongArgType(HSQUIRRELVM vm, HSQOBJECT* obj)
{
    (void)vm;
    (void)obj;
}

void* UnmappedCreateString(void* strings, const SQChar* name)
{
    (void)strings;
    (void)name;
    return 0;
}

void UnmappedNewSlot(HSQUIRRELVM vm, HSQOBJECT* self, HSQOBJECT* key, HSQOBJECT* val,
    SQBool bstatic)
{
    (void)vm;
    (void)self;
    (void)key;
    (void)val;
    (void)bstatic;
}

void UnmappedRaiseFmt(const char* text)
{
    (void)text;
}

SQInteger UnmappedGetStringIdx(HSQUIRRELVM vm, SQInteger idx, const SQChar** out)
{
    (void)vm;
    (void)idx;
    (void)out;
    return -1;
}

void UnmappedPrintCallstack(HSQUIRRELVM vm)
{
    (void)vm;
}

int UnmappedLoadBytecode(HSQUIRRELVM vm)
{
    (void)vm;
    return -1;
}

void UnmappedTableCtor(void* table, void* sharedstate, SQInteger initialSize)
{
    (void)table;
    (void)sharedstate;
    (void)initialSize;
}

void* UnmappedRefLookup(void* refsTable, HSQOBJECT* obj, unsigned* index, void** prev,
    int create)
{
    (void)refsTable;
    (void)obj;
    (void)create;
    if (index) {
        *index = 0;
    }
    if (prev) {
        *prev = 0;
    }
    return 0;
}

void UnmappedClearRefSlot(unsigned* slot)
{
    (void)slot;
}

void UnmappedReleaseRefCounted(int* object)
{
    (void)object;
}

int UnmappedSqCall(HSQUIRRELVM vm, SQInteger nparams)
{
    (void)vm;
    (void)nparams;
    return -1;
}

int UnmappedSqCompile(HSQUIRRELVM vm, SQLEXREADFUNC read, SQUserPointer p,
    const SQChar* sourcename, int raiseerror)
{
    (void)vm;
    (void)read;
    (void)p;
    (void)sourcename;
    (void)raiseerror;
    return -1;
}

SQInteger UnmappedConsoleBufferRead(SQUserPointer p)
{
    (void)p;
    return 0;
}

int UnmappedGet(HSQUIRRELVM vm)
{
    (void)vm;
    return 0;
}

void UnmappedBindHostApi()
{
}

void UnmappedShutdownHost()
{
}

int UnmappedFindFirstFile(const char* path, WIN32_FIND_DATAA* data)
{
    (void)path;
    (void)data;
    return -1;
}

char* UnmappedCopyExeDir(char* dest)
{
    if (dest) {
        dest[0] = 0;
    }
    return dest;
}

void* UnmappedCrtPathSlot()
{
    static char slot[32];
    return slot;
}

int UnmappedKdConstruct(KdObject* object)
{
    (void)object;
    return 0;
}

int UnmappedKdConstructNamed(KdObject* object)
{
    (void)object;
    return 0;
}

int UnmappedKdConstructFail(KdObject* object)
{
    (void)object;
    return 0;
}

void UnmappedDirClose(void* dir)
{
    (void)dir;
}

int UnmappedTryLoadDbgHelpSoft()
{
    return 1;
}

void UnmappedLogOsInfo()
{
}

void UnmappedLogStackWalk()
{
}

void* UnmappedOpenDumpFile(const char* path)
{
    (void)path;
    return 0;
}

int UnmappedMiniDumpCallback()
{
    return 0;
}

int UnmappedMutexCheck(MutexRef* ref)
{
    (void)ref;
    return 0;
}

unsigned long long UnmappedThreadId64()
{
    return GetCurrentThreadId();
}

void UnmappedMutexDestroyBody(void* mutex)
{
    (void)mutex;
}

int UnmappedMutexLockInner(MutexRef* ref)
{
    (void)ref;
    return 0;
}

void UnmappedQueueEvent(void* node)
{
    (void)node;
}

int UnmappedWaitQueue()
{
    return 0;
}

unsigned long long UnmappedQpcDelta()
{
    return 0;
}

void UnmappedSimpleLockFallback()
{
}

void UnmappedApcFallback()
{
}

}
