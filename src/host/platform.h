// Backend contract for the existing KD services. No Win32 or renderer types.
// Select an implementation at build time; no dispatch table is introduced into
// the VC9 matching build. Native window/thread state belongs to the backend.
// These layouts still describe the original x86 ABI; a 64-bit port must also
// update the engine's pointer-sized handles and layout checks.
#ifndef VC1_HOST_PLATFORM_H
#define VC1_HOST_PLATFORM_H

#include <stdarg.h>

namespace vc1 {

struct TlsBlock;
struct MutexRef;
struct KdWindow;

// Inferred name. OpenKODE KDEvent (timestamp, type, userptr, data union) plus
// G5's queue links. kdCreateEvent (0x004964a0) hands out the 16 static slots
// of g_bumpArena first; +8 doubles as the slot's in-use mark.
// SIZE 0x30
struct KdEvent
{
    long long timestamp; // +0x00 QPC ticks, stamped by kdPostEvent when 0
    int type;          // +0x08 KD_EVENT_*
    void* userptr;     // +0x0c
    unsigned data[4];  // +0x10
    KdEvent* next;     // +0x20
    KdEvent* prev;     // +0x24
    void* owner;       // +0x28 compared to EBX in 0x004963b0
    unsigned pad2c;    // +0x2c
};

typedef char Vc1KdEventSizeCheck[sizeof(KdEvent) == 0x30 ? 1 : -1];

// OpenKODE event types seen in the host.
enum
{
    KD_EVENT_TIMER = 0x2a,
    KD_EVENT_QUIT = 0x2b,
    KD_EVENT_PAUSE = 0x2d,
    KD_EVENT_RESUME = 0x2e
};

typedef void (*KdCallbackFunc)(const KdEvent* event);

// Inferred name. kdSetTimer record on g_kdTimers. Times are QPC nanoseconds.
// periodic is KD_TIMER_ONCE (0x3d) or KD_TIMER_PERIODIC_AVERAGE (0x3f).
// SIZE 0x20
struct KdTimer
{
    long long interval; // +0x00
    int periodic;     // +0x08
    void* userptr;    // +0x0c
    long long last;     // +0x10 when the timer last fired (or was set)
    KdTimer* next;    // +0x18
    unsigned pad1c;   // +0x1c
};

extern KdTimer* g_kdTimers; // 0x004ed4fc

// Inferred name. kdInstallCallback record; type 0 matches every event type.
// The list ends at the static g_kdDefaultCallback.
// SIZE 0x14
struct KdCallback
{
    KdCallbackFunc func; // +0x00
    int type;            // +0x04
    void* userptr;       // +0x08
    KdCallback* next;    // +0x0c
    KdCallback* prev;    // +0x10
};

// Inferred name. KD file stat result of the driver stat/fstat slots.
// SIZE 0x18
struct KdFileStat
{
    int mode;                 // +0x00 st_mode
    int pad04;                // +0x04
    long long size;             // +0x08
    long long mtime;            // +0x10
};

// Inferred name. One file-system driver in the g_kdObjects list ("file",
// "bar" per archive, "http"). Every file call goes through these slots with
// the driver's native handle (a FILE* for "file").
// SIZE 0x15c
struct KdObject
{
    KdObject* prev;      // +0x00
    KdObject* next;      // +0x04
    void* data;          // +0x08 driver state ("bar": the open archive)
    const char* scheme;  // +0x0c
    char name[0x100];    // +0x10 CreateKdObject's name (the archive path)
    void (*destructor)(KdObject* self);                                    // +0x110
    int (*open)(KdObject* self, const char* path, const char* mode);       // +0x114
    int (*close)(KdObject* self, int handle);                              // +0x118
    int (*flush)(KdObject* self, int handle);                              // +0x11c
    int (*read)(KdObject* self, void* buffer, unsigned size, unsigned count,
        int handle);                                                       // +0x120
    int (*write)(KdObject* self, const void* buffer, unsigned size, unsigned count,
        int handle);                                                       // +0x124
    int (*getc)(KdObject* self, int handle);                               // +0x128
    int (*putc)(KdObject* self, int c, int handle);                        // +0x12c
    char* (*gets)(KdObject* self, char* buffer, int size, int handle);     // +0x130
    int (*eof)(KdObject* self, int handle);                                // +0x134
    int (*error)(KdObject* self, int handle);                              // +0x138
    void (*clearerr)(KdObject* self, int handle);                          // +0x13c
    int (*seek)(KdObject* self, int handle, long long offset, int origin);   // +0x140
    long long (*tell)(KdObject* self, int handle);                           // +0x144
    int (*stat)(KdObject* self, const char* path, KdFileStat* out);        // +0x148
    int (*fstat)(KdObject* self, int handle, KdFileStat* out);             // +0x14c
    int (*access)(KdObject* self, const char* path, int mode);             // +0x150
    void* (*map)(KdObject* self, int handle, unsigned* size);              // +0x154
    void (*unmap)(KdObject* self, int handle, void* view);                 // +0x158
};

typedef char Vc1KdObjectSizeCheck[sizeof(KdObject) == 0x15c ? 1 : -1];

enum {
    kKdEinval = 0x16,
    kKdEnomem = 0x0c,
    kKdEagain = 0x0b,
    kKdEbusy = 0x10,
    kKdEdeadlk = 0x24
};


KdEvent* AllocEvent();
int QueueEvent(KdEvent* event);
void DropMatchingEvents(const KdEvent* event);
void DropPointerMoves(const KdEvent* event);
void PollGamepad();
void UnlinkHeapByOwner(void* owner);
void UnlinkTimer(KdTimer* timer);
KdTimer* KdSetTimer(long long interval, void* userptr);
void FireTimers();
int KdInstallCallback(KdCallbackFunc func, int type, void* userptr);
void FreeBumpHeaps();
KdEvent* PopTimedEvent();

void* OpenKdPath(const char* path, const char* mode);
void* OpenKdFile(const char* path, const char* mode);
int KdFread(void* buffer, unsigned size, unsigned count, void* file);
int KdFclose(void* file);
void* KdMapFile(void* file, unsigned* size);
int StatKdPath(const char* path, KdFileStat* out);
void* DirRead(unsigned char* dir);
unsigned char* DirOpen();
char* CopyExeDir(const char* path, char* dest);
int KdRename(const char* oldPath, const char* newPath);
KdObject* CreateKdObject(int (*ctor)(KdObject* object), const char* name);
int AppendPath(char* dest, const char* src);
void DestroyKdObjects();
void InitKdFilesystem();
const char* GetKdEnv(const char* name);
const char* GetResourceLocale();

int AllocTlsBlock(void* destructor, TlsBlock** out);
int DestroyTlsBlock(TlsBlock* self);
void DestroyThreadLocal(void* thread);
int MutexUnlock(MutexRef* ref);
int MutexLock(MutexRef* ref);
int MutexDestroy(MutexRef* ref);
void ShutdownThreading();
int InitThreading();

int FormatBuffer(char* buf, unsigned cap, const char* fmt, ...);
void FormatBufferOpen(char* buf, const char* fmt, ...);
void KdLogV(const char* fmt, va_list args);
void KdLog(const char* fmt, ...);
void HostLogV(const char** prefix, const char* tag, const char* fmt, va_list args);
void HostLog(const char** prefix, const char* fmt, ...);
void HostLogWarning(const char** prefix, const char* fmt, ...);
void HostLogError(const char** prefix, const char* fmt, ...);
void HostLogInfo(const char** prefix, const char* fmt, ...);
extern unsigned g_kdLogTag;
extern const char* g_kdLogPrefix;
extern const char* g_emptyCountedBegin;
extern unsigned g_emptyCountedSize;

}
#endif
