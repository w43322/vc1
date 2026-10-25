#include "kdcore/runtime.h"

#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kd/stream/file_reader.h"

namespace vc1 {

// dbghelp records, declared here because dbghelp.dll is loaded at run time
// (layouts of the SDK's SYMBOL_INFO, ADDRESS64, STACKFRAME64,
// IMAGEHLP_STACK_FRAME and the MINIDUMP callback records).

// SIZE 0x58 (+ name)
struct CrashSymbolInfo
{
    unsigned sizeOfStruct;      // 0x00
    unsigned typeIndex;         // 0x04
    unsigned __int64 reserved[2]; // 0x08
    unsigned index;             // 0x18
    unsigned size;              // 0x1c
    unsigned __int64 modBase;   // 0x20
    unsigned flags;             // 0x28  0x10 register relative, 0x40 parameter, 0x80 local
    unsigned __int64 value;     // 0x30
    unsigned __int64 address;   // 0x38
    unsigned registerId;        // 0x40
    unsigned scope;             // 0x44
    unsigned tag;               // 0x48
    unsigned nameLen;           // 0x4c
    unsigned maxNameLen;        // 0x50
    char name[1];               // 0x54
};

// SIZE 0x10
struct CrashAddress
{
    unsigned __int64 offset; // 0x00
    unsigned short segment;  // 0x08
    unsigned mode;           // 0x0c  3: flat
};

// SIZE 0x108
struct CrashStackFrame
{
    CrashAddress pc;         // 0x00
    CrashAddress returnAddr; // 0x10
    CrashAddress frame;      // 0x20
    CrashAddress stack;      // 0x30
    CrashAddress backing;    // 0x40
    void* funcTableEntry;    // 0x50
    unsigned __int64 params[4]; // 0x58
    int farCall;             // 0x78
    int isVirtual;           // 0x7c
    unsigned __int64 reserved[3]; // 0x80
    unsigned char kdHelp[0x70];   // 0x98
};

typedef char Vc1CrashStackFrameSizeCheck[sizeof(CrashStackFrame) == 0x108 ? 1 : -1];

// SIZE 0x80
struct CrashSymbolContext
{
    unsigned __int64 instructionOffset; // 0x00
    unsigned __int64 other[15];         // 0x08
};

// The old IMAGEHLP_LINE size (0x14) is what the game passes.
// SIZE 0x18
struct CrashLine
{
    unsigned sizeOfStruct;    // 0x00
    void* key;                // 0x04
    unsigned lineNumber;      // 0x08
    const char* fileName;     // 0x0c
    unsigned __int64 address; // 0x10
};

// SIZE 0x0c
struct CrashDumpException
{
    DWORD threadId;               // 0x00
    EXCEPTION_POINTERS* pointers; // 0x04
    BOOL clientPointers;          // 0x08
};

// SIZE 0x08
struct CrashDumpCallback
{
    void* routine; // 0x00
    void* param;   // 0x04
};

#pragma pack(push, 4)

// The ModuleCallback member of MINIDUMP_CALLBACK_INPUT (after process id,
// process handle and callback type).
// SIZE 0x5c
struct CrashDumpModule
{
    const wchar_t* fullPath;      // 0x00
    unsigned __int64 baseOfImage; // 0x04
    unsigned sizeOfImage;         // 0x0c
    unsigned checkSum;            // 0x10
    unsigned timeDateStamp;       // 0x14
    unsigned signature;           // 0x18  VS_FIXEDFILEINFO
    unsigned strucVersion;        // 0x1c
    unsigned fileVersionMS;       // 0x20
    unsigned fileVersionLS;       // 0x24
    unsigned rest[13];            // 0x28
};

typedef char Vc1CrashDumpModuleSizeCheck[sizeof(CrashDumpModule) == 0x5c ? 1 : -1];

struct CrashDumpInput
{
    DWORD processId;        // 0x00
    HANDLE process;         // 0x04
    unsigned callbackType;  // 0x08  0: module
    CrashDumpModule module; // 0x0c
};

#pragma pack(pop)

// The SymEnumSymbols context of LogFrame.
struct CrashFrameContext
{
    const CrashStackFrame* frame; // 0x00
    int index;                    // 0x04
    int parameters;               // 0x08  1: parameters, 0: locals
};

typedef BOOL (WINAPI *SymGetTypeInfoFn)(HANDLE process, unsigned __int64 modBase, unsigned typeId,
    int info, void* out);
typedef BOOL (WINAPI *SymFromAddrFn)(HANDLE process, unsigned __int64 address,
    unsigned __int64* displacement, CrashSymbolInfo* symbol);
typedef BOOL (WINAPI *SymSetContextFn)(HANDLE process, CrashSymbolContext* frame, void* context);
typedef BOOL (WINAPI *SymEnumSymbolsFn)(HANDLE process, unsigned __int64 base, const char* mask,
    void* callback, void* context);
typedef BOOL (WINAPI *SymGetLineFromAddr64Fn)(HANDLE process, unsigned __int64 address,
    DWORD* displacement, CrashLine* line);
typedef BOOL (WINAPI *MiniDumpWriteDumpFn)(HANDLE process, DWORD processId, HANDLE file,
    int type, CrashDumpException* exception, void* userStream, CrashDumpCallback* callback);
typedef BOOL (WINAPI *StackWalk64Fn)(DWORD machine, HANDLE process, HANDLE thread,
    CrashStackFrame* frame, void* context, void* readMemory, FARPROC functionTableAccess,
    FARPROC getModuleBase, void* translateAddress);
typedef int (WINAPI *SymCleanupFn)(HANDLE process);

// GLOBAL: VC1 0x004ed538
int g_crashParamFrame;
// GLOBAL: VC1 0x004ed53c
int g_crashFrameIndex;

// GLOBAL: VC1 0x004d46a8
extern const char kCrashHexFormat[] = "0x%08x";

// version.dll GetFileVersionInfo* on dbghelp.dll; 1 if its file version is
// at least 6.6.7.5.
// FUNCTION: VC1 0x00476d80
static int TryLoadDbgHelpSoft()
{
    typedef DWORD (WINAPI *GetSizeFn)(const char* file, DWORD* handle);
    typedef BOOL (WINAPI *GetInfoFn)(const char* file, DWORD handle, DWORD size, void* data);
    typedef BOOL (WINAPI *QueryFn)(const void* block, const char* sub, void** out, unsigned* size);
    // STRING: VC1 0x004d48e8
    const HMODULE module = LoadLibraryA("version.dll");
    // STRING: VC1 0x004d48f4
    const GetSizeFn getSize = reinterpret_cast<GetSizeFn>(GetProcAddress(module, "GetFileVersionInfoSizeA"));
    // STRING: VC1 0x004d490c
    const GetInfoFn getInfo = reinterpret_cast<GetInfoFn>(GetProcAddress(module, "GetFileVersionInfoA"));
    // STRING: VC1 0x004d4920
    const QueryFn query = reinterpret_cast<QueryFn>(GetProcAddress(module, "VerQueryValueA"));
    unsigned version = 0;
    DWORD handle = 0;
    // STRING: VC1 0x004d4930
    const DWORD size = getSize("dbghelp.dll", &handle);

    if (size) {
        void* const data = malloc(size);
        if (getInfo("dbghelp.dll", handle, size, data)) {
            void* block;
            unsigned length = 0;
            // STRING: VC1 0x004d493c
            if (query(data, "\\", &block, &length)) {
                const unsigned char* const info = static_cast<const unsigned char*>(block);
                version = (static_cast<unsigned>(static_cast<unsigned short>((info[0x0a] << 8) | info[0x08])) << 16)
                    | static_cast<unsigned short>(info[0x0e] << 8) | info[0x0c];
            }
        }
        free(data);
    }
    FreeLibrary(module);
    return version >= 0x06060705;
}

// LoadLibraryA("kernel32.dll") + IsDebuggerPresent. Returns 1 if a debugger is attached.
// FUNCTION: VC1 0x00476e60
__declspec(noinline) int ProbeDebuggerPresent()
{
    // STRING: VC1 0x004d48c4
    const HMODULE kernel32 = LoadLibraryA("kernel32.dll");
    // STRING: VC1 0x004d48d4
    const FARPROC isDebuggerPresent = GetProcAddress(kernel32, "IsDebuggerPresent");
    if (isDebuggerPresent) {
        const int present = isDebuggerPresent();
        if (present) {
            FreeLibrary(kernel32);
            return 1;
        }
    }
    FreeLibrary(kernel32);
    return 0;
}

// EAX=base type, ESI=data, EDI=out; the size (64-bit) on the stack. A char
// pointer is printed as a string and then overwritten by "%d" (the original
// falls through).
// FUNCTION: VC1 0x00476eb0
static void FormatBasicValue(char* out, unsigned baseType, const void* data, unsigned __int64 size)
{
    switch (size) {
    case 1:
        // STRING: VC1 0x004d4804
        sprintf(out, "%x", *static_cast<const unsigned char*>(data));
        break;
    case 2:
        sprintf(out, "%x", *static_cast<const unsigned short*>(data));
        break;
    case 4:
        switch (baseType) {
        case 2:
            if (!IsBadStringPtrA(*static_cast<char* const*>(data), 0x20)) {
                // STRING: VC1 0x004d4808
                sprintf(out, "\"%.31s\"", *static_cast<char* const*>(data));
            }
            else {
                sprintf(out, "%x", *static_cast<const unsigned*>(data));
            }
        case 6:
            sprintf(out, "%d", *static_cast<const int*>(data));
            break;
        case 8:
            sprintf(out, "%f", *static_cast<const float*>(data));
            break;
        default:
            sprintf(out, "%x", *static_cast<const unsigned*>(data));
            break;
        }
        break;
    case 8:
        if (baseType == 8) {
            // STRING: VC1 0x004d4810
            sprintf(out, "%lf", *static_cast<const double*>(data));
        }
        else {
            // STRING: VC1 0x004d4814
            sprintf(out, "%I64X", *static_cast<const unsigned __int64*>(data));
        }
        break;
    }
}

// EAX=code. Inferred name.
// FUNCTION: VC1 0x00476fd0
static const char* ExceptionName(DWORD code)
{
    switch (code) {
    // STRING: VC1 0x004d44fc
    case 0xc0000005: return "ACCESS_VIOLATION";
    // STRING: VC1 0x004d4510
    case 0x80000002: return "DATATYPE_MISALIGNMENT";
    // STRING: VC1 0x004d4528
    case 0x80000003: return "BREAKPOINT";
    // STRING: VC1 0x004d4534
    case 0x80000004: return "SINGLE_STEP";
    // STRING: VC1 0x004d4540
    case 0xc000008c: return "ARRAY_BOUNDS_EXCEEDED";
    // STRING: VC1 0x004d4558
    case 0xc000008d: return "FLT_DENORMAL_OPERAND";
    // STRING: VC1 0x004d4570
    case 0xc000008e: return "FLT_DIVIDE_BY_ZERO";
    // STRING: VC1 0x004d4584
    case 0xc000008f: return "FLT_INEXACT_RESULT";
    // STRING: VC1 0x004d4598
    case 0xc0000090: return "FLT_INVALID_OPERATION";
    // STRING: VC1 0x004d45b0
    case 0xc0000091: return "FLT_OVERFLOW";
    // STRING: VC1 0x004d45c0
    case 0xc0000092: return "FLT_STACK_CHECK";
    // STRING: VC1 0x004d45d0
    case 0xc0000093: return "FLT_UNDERFLOW";
    // STRING: VC1 0x004d45e0
    case 0xc0000094: return "INT_DIVIDE_BY_ZERO";
    // STRING: VC1 0x004d45f4
    case 0xc0000095: return "INT_OVERFLOW";
    // STRING: VC1 0x004d4604
    case 0xc0000096: return "PRIV_INSTRUCTION";
    // STRING: VC1 0x004d4618
    case 0xc0000006: return "IN_PAGE_ERROR";
    // STRING: VC1 0x004d4628
    case 0xc000001d: return "ILLEGAL_INSTRUCTION";
    // STRING: VC1 0x004d463c
    case 0xc0000025: return "NONCONTINUABLE_EXCEPTION";
    // STRING: VC1 0x004d4658
    case 0xc00000fd: return "STACK_OVERFLOW";
    // STRING: VC1 0x004d4668
    case 0xc0000026: return "INVALID_DISPOSITION";
    // STRING: VC1 0x004d467c
    case 0x80000001: return "GUARD_PAGE";
    // STRING: VC1 0x004d4688
    case 0xc0000008: return "INVALID_HANDLE";
    }
    // STRING: VC1 0x004d4698
    return "UNKNOWN_ERROR";
}

// EBX=text. Inferred name. A malloc'd multibyte copy, 0 when it does not convert.
// FUNCTION: VC1 0x004771f0
static char* WideToAnsi(const wchar_t* text)
{
    const size_t length = wcslen(text);
    char* const out = static_cast<char*>(malloc(length * 2 + 1));
    const size_t converted = wcstombs(out, text, length);

    if (converted == static_cast<size_t>(-1)) {
        return 0;
    }
    out[converted] = 0;
    return out;
}

// Load dbghelp.dll and resolve MiniDump/Sym* exports. Original compares each pointer to 0.
// FUNCTION: VC1 0x00477240
int LoadDbgHelp()
{
    if (!TryLoadDbgHelpSoft()) {
        return 0;
    }

    g_dbghelp = LoadLibraryA("dbghelp.dll");
    if (g_dbghelp == reinterpret_cast<HMODULE>(-1)) {
        return 0;
    }

    // STRING: VC1 0x004d4940
    g_MiniDumpWriteDump = GetProcAddress(g_dbghelp, "MiniDumpWriteDump");
    // STRING: VC1 0x004d4954
    g_SymInitialize = GetProcAddress(g_dbghelp, "SymInitialize");
    // STRING: VC1 0x004d4964
    g_SymSetOptions = GetProcAddress(g_dbghelp, "SymSetOptions");
    // STRING: VC1 0x004d4974
    g_SymGetOptions = GetProcAddress(g_dbghelp, "SymGetOptions");
    // STRING: VC1 0x004d4984
    g_SymCleanup = GetProcAddress(g_dbghelp, "SymCleanup");
    // STRING: VC1 0x004d4990
    g_SymGetTypeInfo = GetProcAddress(g_dbghelp, "SymGetTypeInfo");
    // STRING: VC1 0x004d49a0
    g_SymGetLineFromAddr64 = GetProcAddress(g_dbghelp, "SymGetLineFromAddr64");
    // STRING: VC1 0x004d49b8
    g_SymEnumSymbols = GetProcAddress(g_dbghelp, "SymEnumSymbols");
    // STRING: VC1 0x004d49c8
    g_SymSetContext = GetProcAddress(g_dbghelp, "SymSetContext");
    // STRING: VC1 0x004d49d8
    g_SymFromAddr = GetProcAddress(g_dbghelp, "SymFromAddr");
    // STRING: VC1 0x004d49e4
    g_StackWalk64 = GetProcAddress(g_dbghelp, "StackWalk64");
    // STRING: VC1 0x004d49f0
    g_SymFunctionTableAccess64 = GetProcAddress(g_dbghelp, "SymFunctionTableAccess64");
    // STRING: VC1 0x004d4a0c
    g_SymGetModuleBase64 = GetProcAddress(g_dbghelp, "SymGetModuleBase64");

    if (g_MiniDumpWriteDump && g_SymInitialize && g_SymSetOptions && g_SymGetOptions
        && g_SymCleanup && g_SymGetTypeInfo && g_SymGetLineFromAddr64 && g_SymEnumSymbols
        && g_SymSetContext && g_SymFromAddr && g_SymGetModuleBase64 && g_StackWalk64
        && g_SymFunctionTableAccess64) {
        typedef unsigned (WINAPI *SymGetOptionsFn)();
        typedef unsigned (WINAPI *SymSetOptionsFn)(unsigned options);
        typedef int (WINAPI *SymInitializeFn)(HANDLE process, const char* search, int invade);
        const unsigned options =
            reinterpret_cast<SymGetOptionsFn>(g_SymGetOptions)();
        reinterpret_cast<SymSetOptionsFn>(g_SymSetOptions)(options | 0x14);
        reinterpret_cast<SymInitializeFn>(g_SymInitialize)(GetCurrentProcess(), 0, 1);
        return 1;
    }

    if (g_dbghelp) {
        FreeLibrary(g_dbghelp);
        g_dbghelp = 0;
    }
    return 0;
}

// cdecl. Inferred name. One symbol's value by its type: pointers are
// followed for the depth, UDTs print their name, base types their value.
// FUNCTION: VC1 0x00477410
static void FormatTypedValue(char* out, unsigned __int64 modBase, unsigned typeId, const void* data)
{
    const SymGetTypeInfoFn getTypeInfo = reinterpret_cast<SymGetTypeInfoFn>(g_SymGetTypeInfo);
    const HANDLE process = GetCurrentProcess();
    int depth = 0;
    unsigned tag;

    if (getTypeInfo(process, modBase, typeId, 0, &tag)) {
        while (tag == 0x0e) {
            ++depth;
            getTypeInfo(process, modBase, typeId, 3, &typeId);
            if (!getTypeInfo(process, modBase, typeId, 0, &tag)) {
                break;
            }
        }
    }
    switch (tag) {
    case 0x0b: {
        wchar_t* name;
        if (getTypeInfo(process, modBase, typeId, 1, &name)) {
            char* const text = WideToAnsi(name);
            LocalFree(name);
            if (depth == 0) {
                // STRING: VC1 0x004d481c
                sprintf(out, "(%s) 0x%08x", text, data);
            }
            else if (depth == 1) {
                // STRING: VC1 0x004d4828
                sprintf(out, "(%s *) 0x%08x", text, *static_cast<const unsigned*>(data));
            }
            else {
                // STRING: VC1 0x004d4838
                sprintf(out, "(%s **) 0x%08x", text, data);
            }
            free(text);
        }
        break;
    }
    case 0x0c:
        sprintf(out, "%d", *static_cast<const int*>(data));
        break;
    case 0x0d:
        sprintf(out, kCrashHexFormat, *static_cast<const unsigned*>(data));
        break;
    case 0x10: {
        unsigned __int64 length;
        unsigned baseType;
        getTypeInfo(process, modBase, typeId, 2, &length);
        if (depth == 1) {
            if (length == 1) {
                // STRING: VC1 0x004d4848
                sprintf(out, "0x%08x \"%s\"", *static_cast<const unsigned*>(data),
                    *static_cast<char* const*>(data));
                break;
            }
        }
        else if (depth < 1) {
            if (getTypeInfo(process, modBase, typeId, 5, &baseType)) {
                FormatBasicValue(out, baseType, data, length);
            }
            break;
        }
        sprintf(out, kCrashHexFormat, *static_cast<const unsigned*>(data));
        break;
    }
    }
}

// ESI=context. Command line, build, OS version, exception and registers.
// FUNCTION: VC1 0x00477650
void LogCrashHeader(const EXCEPTION_RECORD* record, const CONTEXT* context)
{
    OSVERSIONINFOA version;

    // STRING: VC1 0x004d46d4
    KdLog("Cmd line: %.65s\n", GetCommandLineA());
    // STRING: VC1 0x004d4700
    KdLog("Version compiled %s, %s\n",
        // STRING: VC1 0x004d46f4
        "Dec 21 2009",
        // STRING: VC1 0x004d46e8
        "22:47:12");
    version.dwOSVersionInfoSize = sizeof(version);
    GetVersionExA(&version);
    // STRING: VC1 0x004d471c
    KdLog("Platform: Windows OS version %d.%d build %d %s\n\n", version.dwMajorVersion,
        version.dwMinorVersion, version.dwBuildNumber, version.szCSDVersion);
    // STRING: VC1 0x004d4750
    KdLog("Exception: %s\n\n", ExceptionName(record->ExceptionCode));
    // STRING: VC1 0x004d4760
    KdLog("Registers:\n");
    // STRING: VC1 0x004d476c
    KdLog("eax=%08x ebx=%08x ecx=%08x edx=%08x esi=%08x edi=%08x\n", context->Eax,
        context->Ebx, context->Ecx, context->Edx, context->Esi, context->Edi);
    // STRING: VC1 0x004d47a4
    KdLog("eip=%08x esp=%08x ebp=%08x efl=%08x\n", context->Eip, context->Esp,
        context->Ebp, context->EFlags);
    // STRING: VC1 0x004d47cc
    KdLog("cs=%04x  ss=%04x  ds=%04x  es=%04x  fs=%04x  gs=%04x\n", context->SegCs,
        context->SegSs, context->SegDs, context->SegEs, context->SegFs, context->SegGs);
}

// RET 0xc. MiniDumpWriteDump callback: logs every module (base, path,
// file version, size).
// FUNCTION: VC1 0x00477770
static BOOL WINAPI LogDumpModule(void* param, const CrashDumpInput* input, void* output)
{
    (void)param;
    (void)output;
    if (input && input->callbackType == 0) {
        const CrashDumpModule module = input->module;
        char* const path = WideToAnsi(module.fullPath);
        KdLog(kCrashHexFormat, module.baseOfImage);
        // STRING: VC1 0x004d46b0
        KdLog("  %s", path);
        free(path);
        // STRING: VC1 0x004d46b8
        KdLog(" (%d.%d.%d.%d, %d bytes)\n", module.fileVersionMS >> 16,
            module.fileVersionMS & 0xffff, module.fileVersionLS >> 16,
            module.fileVersionLS & 0xffff, module.sizeOfImage);
    }
    return TRUE;
}

// RET 0xc. SymEnumSymbols callback of LogFrame: frame-relative parameters
// (comma separated on the frame line) or locals (one per line).
// FUNCTION: VC1 0x00477800
static BOOL WINAPI LogFrameSymbol(const CrashSymbolInfo* symbol, unsigned size, void* user)
{
    const CrashFrameContext* const context = static_cast<const CrashFrameContext*>(user);
    const int index = context->index;
    const int parameters = context->parameters;
    char value[0x100];
    unsigned address;

    (void)size;
    value[0] = 0;
    memset(value + 1, 0, sizeof(value) - 1);
    if (!(symbol->flags & 0x10)) {
        return FALSE;
    }
    address = static_cast<unsigned>(context->frame->frame.offset) + static_cast<unsigned>(symbol->address);
    if (parameters == 1) {
        if (symbol->flags & 0x40) {
            if (g_crashParamFrame == index) {
                // STRING: VC1 0x004d4854
                KdLog(", ", 2);
            }
            else {
                g_crashParamFrame = index;
            }
            FormatTypedValue(value, symbol->modBase, symbol->typeIndex,
                reinterpret_cast<const void*>(address));
            // STRING: VC1 0x004d4858
            KdLog("%s=%s", symbol->name, value);
            return TRUE;
        }
    }
    else if (parameters == 0 && (symbol->flags & 0x80)) {
        FormatTypedValue(value, symbol->modBase, symbol->typeIndex,
            reinterpret_cast<const void*>(address));
        // STRING: VC1 0x004d4860
        KdLog("        %s = %s\n", symbol->name, value);
    }
    return TRUE;
}

// cdecl, the frame by value. "#n pc in function(params) at file:line" and
// then the locals. The 64-bit pc goes to one %08x, so the name prints from
// its high half, as in the original.
// FUNCTION: VC1 0x00477910
static void LogFrame(CrashStackFrame frame)
{
    struct {
        CrashSymbolInfo info;
        char name[0x104];
    } symbol;
    unsigned __int64 displacement = 0;
    DWORD lineDisplacement = 0;
    CrashLine line;
    CrashSymbolContext symbolContext;
    CrashFrameContext context;
    const HANDLE process = GetCurrentProcess();

    ++g_crashFrameIndex;
    symbol.info.sizeOfStruct = 0x58;
    symbol.info.maxNameLen = 0x104;
    if (reinterpret_cast<SymFromAddrFn>(g_SymFromAddr)(process, frame.pc.offset,
            &displacement, &symbol.info) == 1) {
        // STRING: VC1 0x004d4874
        KdLog("#%d  0x%08x in %.200s (", g_crashFrameIndex, frame.pc.offset, symbol.info.name);
        symbolContext.instructionOffset = frame.pc.offset;
        reinterpret_cast<SymSetContextFn>(g_SymSetContext)(process, &symbolContext, 0);
        context.frame = &frame;
        context.index = g_crashFrameIndex;
        context.parameters = 1;
        reinterpret_cast<SymEnumSymbolsFn>(g_SymEnumSymbols)(process, 0, 0,
            reinterpret_cast<void*>(LogFrameSymbol), &context);
        // STRING: VC1 0x004d488c
        KdLog(")");
    }
    else {
        // STRING: VC1 0x004d4890
        KdLog("#%d  0x%08x in (unknown function)", g_crashFrameIndex, frame.pc.offset);
    }
    line.sizeOfStruct = 0x14;
    if (reinterpret_cast<SymGetLineFromAddr64Fn>(g_SymGetLineFromAddr64)(process,
            frame.pc.offset, &lineDisplacement, &line)) {
        // STRING: VC1 0x004d48b4
        KdLog(" at %s:%d\n", line.fileName, line.lineNumber);
    }
    else {
        // STRING: VC1 0x004d48c0
        KdLog("\n");
    }
    context.parameters = 0;
    reinterpret_cast<SymEnumSymbolsFn>(g_SymEnumSymbols)(process, 0, 0,
        reinterpret_cast<void*>(LogFrameSymbol), &context);
}

// EAX=info. "/data/kdCrashDump.dmp" through the KD file layer; only a file
// of the first driver (stdio, a FILE*) is dumped. The modules are logged by
// the dump callback.
// FUNCTION: VC1 0x00477a90
int WriteCrashDump(EXCEPTION_POINTERS* info)
{
    // STRING: VC1 0x004d4a54
    KdFileHandle* const file = static_cast<KdFileHandle*>(OpenKdFile("/data/kdCrashDump.dmp", "wb"));
    CrashDumpException exception;
    CrashDumpCallback callback;
    HANDLE handle;

    if (!file || file->driver != g_kdObjects) {
        return 0;
    }
    handle = reinterpret_cast<HANDLE>(_get_osfhandle(_fileno(reinterpret_cast<FILE*>(file->native))));
    if (handle == INVALID_HANDLE_VALUE) {
        return 0;
    }
    exception.pointers = info;
    exception.threadId = GetCurrentThreadId();
    exception.clientPointers = 0;
    callback.routine = reinterpret_cast<void*>(LogDumpModule);
    callback.param = 0;
    reinterpret_cast<MiniDumpWriteDumpFn>(g_MiniDumpWriteDump)(GetCurrentProcess(),
        GetCurrentProcessId(), handle, 0, info ? &exception : 0, 0, &callback);
    file->driver->close(file->driver, file->native);
    free(file);
    return 1;
}

// EAX=context. StackWalk64 from the exception context, or from the current
// thread (skipping this frame) without one.
// FUNCTION: VC1 0x00477b60
static void LogStackWalk(CONTEXT* context)
{
    const HANDLE process = GetCurrentProcess();
    int index = 0;
    int skip = 0;
    CrashStackFrame frame;
    CONTEXT local;

    if (!context) {
        skip = 1;
        local.ContextFlags = 0x10007;
        if (!GetThreadContext(GetCurrentThread(), &local)) {
            return;
        }
        context = &local;
    }
    memset(&frame, 0, sizeof(frame));
    frame.pc.offset = context->Eip;
    frame.pc.segment = 0;
    frame.pc.mode = 3;
    frame.frame.offset = context->Ebp;
    frame.frame.segment = 0;
    frame.frame.mode = 3;
    frame.stack.offset = context->Esp;
    frame.stack.segment = 0;
    frame.stack.mode = 3;
    while (reinterpret_cast<StackWalk64Fn>(g_StackWalk64)(0x14c, process, GetCurrentThread(),
               &frame, context, 0, g_SymFunctionTableAccess64, g_SymGetModuleBase64, 0)) {
        if (index >= skip && frame.pc.offset != 0) {
            LogFrame(frame);
        }
        ++index;
    }
}

// SetUnhandledExceptionFilter target. Original returns 1 after MessageBoxA, else 0.
// FUNCTION: VC1 0x00477cb0
LONG WINAPI UnhandledExceptionFilter(EXCEPTION_POINTERS* info)
{
    if (g_dbghelp != reinterpret_cast<HMODULE>(-1) || ProbeDebuggerPresent() == 1
        || LoadDbgHelp() == 0) {
        return 0;
    }
    // STRING: VC1 0x004d4a20
    KdLog("\nProcess info:\n");
    LogCrashHeader(info ? info->ExceptionRecord : 0, info ? info->ContextRecord : 0);
    // STRING: VC1 0x004d4a30
    KdLog("\nStacktrace:\n");
    LogStackWalk(info ? info->ContextRecord : 0);
    // STRING: VC1 0x004d4a40
    KdLog("\n\nLoaded modules:\n");
    WriteCrashDump(info);
    reinterpret_cast<SymCleanupFn>(g_SymCleanup)(GetCurrentProcess());
    FreeLibrary(g_dbghelp);
    MessageBoxA(GetActiveWindow(),
        // STRING: VC1 0x004d4a78
        "This application has halted due to an unexpected error.\nA crash report and minidump file were saved to disk:\n\nkdCrashDump.dmp\nkdLogMessage.txt\n\nPlease send these files to developers to help us analyse\nand solve this problem.\n",
        // STRING: VC1 0x004d4a6c
        "Exception",
        0x10);
    return 1;
}

}
