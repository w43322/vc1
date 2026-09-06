#include "host/runtime.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

// Unmapped callees of this round's functions. No FUNCTION markers.
namespace vc1 {

void* UnmappedFactory()
{
    return 0;
}

int UnmappedQueryNamed(void* factory, void** outUnknown, const char* name)
{
    (void)factory;
    (void)name;
    if (outUnknown) {
        *outUnknown = 0;
    }
    return 0;
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

void UnmappedOpenVm()
{
}

void UnmappedBindHostClasses()
{
}

void UnmappedInitHostServices()
{
}

void UnmappedPushRegistry()
{
}

void UnmappedBindIndex(unsigned index)
{
    (void)index;
}

void UnmappedFinishRoot()
{
}

int UnmappedGetSlot(SquirrelObject* object, const char* name)
{
    (void)object;
    (void)name;
    return 0;
}

int UnmappedLoadNut(SquirrelObject* out, const char* path)
{
    (void)path;
    if (out) {
        InitSquirrelObject(out);
    }
    return 0;
}

void UnmappedNewSlot(SquirrelObject* object, int flags)
{
    (void)object;
    (void)flags;
}

void UnmappedPopValue(SquirrelObject* dest)
{
    if (dest) {
        InitSquirrelObject(dest);
    }
}

unsigned UnmappedReadInt()
{
    return 0;
}

int UnmappedHasSlot()
{
    return 0;
}

void UnmappedRunConsole(const char* text)
{
    (void)text;
}

void UnmappedReleaseRef()
{
}

void UnmappedLog(const char* text)
{
    (void)text;
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

char* UnmappedFindBytes(const char* haystack, unsigned haystackLen, const char* needle,
    unsigned needleLen)
{
    (void)haystack;
    (void)haystackLen;
    (void)needle;
    (void)needleLen;
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
