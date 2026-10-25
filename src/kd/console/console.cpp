#include "kd/console/console.h"

#include "kd/iface/igraphics.h"
#include "kd/iface/iid.h"
#include "kd/iface/iresource_manager.h"
#include "kdcore/native.h"
#include "kd/stream/file_reader.h"
#include "bindings/sqplus.h"

#include <math.h>
#include <new>
#include <stdlib.h>
#include <string.h>

namespace vc1 {

extern const char kPercentS[];
extern const char kConsoleFont[];

// STRING: VC1 0x004d4bdc
static const char kConsole[] = "Console";
// GLOBAL: VC1 0x004d065c
const char* g_kdNameConsole = kConsole;

// STRING: VC1 0x004d5758
static const char kEnableStdio[] = "KD_ENABLE_STDIO";
// STRING: VC1 0x004d5768
static const char kConsoleScript[] = "scripts/sys/Console.nut";
// The console font's prompt glyph.
// STRING: VC1 0x004d5798
static const char kPrompt[] = "\x7f";
// STRING: VC1 0x004d579c
static const char kCursor[] = "_";
// STRING: VC1 0x004d57a0
static const char kConsoleTag[] = "[Console]";
// Write and Render both look for it.
// GLOBAL: VC1 0x004d57ac
extern const char kConsoleError[] = "error";
// STRING: VC1 0x004d57b4
static const char kConsoleErrorUpper[] = "ERROR";
// STRING: VC1 0x004d57bc
static const char kConsoleWarning[] = "warning";
// STRING: VC1 0x004d57c4
static const char kConsoleInfo[] = "info";
// Bytes that end the word Complete hands to Console.nut.
// STRING: VC1 0x004d57cc
static const char kDelimiters[] = "(){}[];,.=><!~?:+-*/&|^%# \n\r\t\"";

// Inferred name. The print hook: Write of the console, set while it exists
// (the stdin reader polls it as the console's lifetime flag).
// GLOBAL: VC1 0x004ed140
void (*g_kdConsolePrint)(const char* text);
// The singleton's IAbstract face.
// GLOBAL: VC1 0x004ed544
IAbstract* g_kdConsole;
// Non-null when the KD_ENABLE_STDIO environment variable is set.
// GLOBAL: VC1 0x004ed6e4
const char* g_kdEnableStdio = GetKdEnv(kEnableStdio);
// Storage for the string GetStringRef returns by reference.
// GLOBAL: VC1 0x004ee50c
const SQChar* g_scriptStringResult;

// floorf to int, out of line where the console pages its output.
// FUNCTION: VC1 0x00404ff0
static __declspec(noinline) int FloorToInt(float value)
{
    return static_cast<int>(floorf(value));
}

// XOR AL,AL; RET 4. IInputHandler vtable+0x10: pointer events are not used.
// FUNCTION: VC1 0x00418000 FOLDED
unsigned char KdConsole::OnPointer(const unsigned* data)
{
    (void)data;
    return 0;
}

// ESI=this, RET 4 (line). The stdin queue's push: drops the executed lines
// first, then appends line.
// FUNCTION: VC1 0x004180e0
void KdStringQueue::Push(const KdString& line)
{
    if (done != 0) {
        const unsigned bytes = done << 4;
        unsigned char* const first = reinterpret_cast<unsigned char*>(list.items.begin);
        EraseCountedBytes(first, first + bytes, first + list.items.size);
        list.items.size -= bytes;
        done = 0;
    }
    list.PushBack(line);
}

// ECX=&t0, EAX=&from, cdecl &to, &t. Inferred name. Linear from (at t0) to
// to (at 1000).
// FUNCTION: VC1 0x00418150
static __declspec(noinline) int InterpolateInt(const int& t0, const int& from, const int& to,
    const int& t)
{
    const int span = 1000 - t0;
    int value = from;

    if (span != 0) {
        value += (to - from) * (t - t0) / span;
    }
    return value;
}

// InterpolateInt's body, which InterpolateColor inlines once per channel.
static __forceinline int LerpChannel(int from, int to, int t0, int t)
{
    const int span = 1000 - t0;
    int value = from;

    if (span != 0) {
        value += (to - from) * (t - t0) / span;
    }
    return value;
}

// ECX=&from, cdecl out, &to, &t0, &t. Inferred name. InterpolateInt per byte
// of an ARGB color.
// FUNCTION: VC1 0x00418180
static __declspec(noinline) void InterpolateColor(const unsigned& from, unsigned* out,
    const unsigned& to, const int& t0, const int& t)
{
    const int b = LerpChannel(from & 0xff, to & 0xff, t0, t);
    const int g = LerpChannel((from >> 8) & 0xff, (to >> 8) & 0xff, t0, t);
    const int r = LerpChannel((from >> 16) & 0xff, (to >> 16) & 0xff, t0, t);
    const int a = LerpChannel(from >> 24, to >> 24, t0, t);

    *out = (((a & 0xff) << 8 | (r & 0xff)) << 8 | (g & 0xff)) << 8 | (b & 0xff);
}

static const SQChar*& GetStringRef(HSQUIRRELVM v, int idx);

// EAX=object. SqPlus SquirrelObject::Get<const SQChar*>: push, read, pop.
// Inferred name.
// FUNCTION: VC1 0x00418290
static __declspec(noinline) const SQChar* GetObjectString(const SquirrelObject& object)
{
    const SQChar* s;

    sq_pushobject(SquirrelVM::_VM, const_cast<SquirrelObject&>(object).GetObjectHandle());
    s = GetStringRef(SquirrelVM::_VM, -1);
    sq_poptop(SquirrelVM::_VM);
    return s;
}

// The game's SqPlus string Get: the string at idx through a static that is
// returned by reference; a type error is logged and leaves null.
// Inferred name.
// FUNCTION: VC1 0x00418310
static __declspec(noinline) const SQChar*& GetStringRef(HSQUIRRELVM v, int idx)
{
    const SQChar* s = 0;

    SQPLUS_CHECK_GET(sq_getstring(v, idx, &s));
    g_scriptStringResult = s;
    return g_scriptStringResult;
}

static KdString& LastLine(KdStringList& list)
{
    return reinterpret_cast<KdString*>(list.items.begin)[(list.items.size >> 4) - 1];
}

// MOV AL, [ECX+0x11]; RET. IVisible vtable+0x10.
// FUNCTION: VC1 0x0046ab50
bool KdConsole::IsVisible()
{
    return visible;
}

// RET 4. IVisible vtable+0x0c.
// FUNCTION: VC1 0x0046ab60
void KdConsole::SetVisible(bool value)
{
    visible = value;
}

// MOV AL, [ECX+0x14]; RET. IManualControl vtable+0x10: the console is open.
// FUNCTION: VC1 0x0046ab70
bool KdConsole::IsControlEnabled()
{
    return open;
}

// RET 4. IManualControl vtable+0x0c: opens or closes the console.
// FUNCTION: VC1 0x0046ab80
void KdConsole::EnableControl(bool enable)
{
    open = enable;
}

// MOV EAX, 0x7fffffff; RET. IRenderable vtable+0x14: drawn last.
// FUNCTION: VC1 0x0046ab90
int KdConsole::GetRenderOrder()
{
    return 0x7fffffff;
}

// cdecl text. Inferred name. The print hook g_kdConsolePrint.
// FUNCTION: VC1 0x0046aba0
static void ConsolePrint(const char* text)
{
    KdConsole* const console = g_kdConsole ? static_cast<KdConsole*>(g_kdConsole) : 0;
    IOutputStream* const out = console;

    out->Write(text, static_cast<unsigned>(strlen(text)));
}

// ECX=this, EAX=position. Clamps to the input and empties the selection.
// FUNCTION: VC1 0x0046abe0
void KdConsole::SetCursor(int position)
{
    const int length = static_cast<int>(input.size);

    if (position < 0) {
        position = 0;
    }
    else if (position > length) {
        position = length;
    }
    cursor = position;
    selectionStart = position;
    selectionLength = 0;
}

// EDI=graphics, cdecl thickness and color. Inferred name. A frame of the
// given thickness inside the view, drawn untransformed.
// FUNCTION: VC1 0x0046ac10
static void DrawFrame(KdPtr<IGraphics>& graphics, int thickness, unsigned color)
{
    const Rect* const view = graphics->GetView();
    Matrix3 saved = graphics->GetTransform();

    graphics->SetTransform(Matrix3::Identity);
    graphics->SetColor(color);
    graphics->FillRect(static_cast<float>(view->x), static_cast<float>(view->y),
        static_cast<float>(view->width), static_cast<float>(thickness));
    graphics->FillRect(static_cast<float>(view->x), static_cast<float>(view->y + view->height),
        static_cast<float>(view->width), static_cast<float>(-thickness));
    graphics->FillRect(static_cast<float>(view->x), static_cast<float>(view->y + thickness),
        static_cast<float>(thickness), static_cast<float>(view->height - thickness * 2));
    graphics->FillRect(static_cast<float>(view->x + view->width),
        static_cast<float>(view->y + thickness), static_cast<float>(-thickness),
        static_cast<float>(view->height - thickness * 2));
    graphics->SetTransform(saved);
}

// RET 4. this = IRenderable. The IAbstract case has no null check.
// FUNCTION: VC1 0x0046ada0
void* KdConsole::QueryInterface(const char** iid)
{
    const char* const name = *iid;

    if (name == g_iidRenderable) {
        return static_cast<IRenderable*>(this);
    }
    if (name == g_iidInputHandler) {
        return static_cast<IInputHandler*>(this);
    }
    if (name == g_iidOutputStream) {
        return static_cast<IOutputStream*>(this);
    }
    if (name == g_iidManualControl) {
        return static_cast<IManualControl*>(this);
    }
    if (name == g_iidVisible) {
        return static_cast<IVisible*>(this);
    }
    if (name == g_iidAbstract) {
        return reinterpret_cast<char*>(static_cast<IRenderable*>(this)) + 0x14;
    }
    return 0;
}

// ADD ECX, 0x14, then IAbstract::Release (0x00401ee0) inlined by LTCG: the deleting
// destructor (vtable+0x0c) on zero refs.
// FUNCTION: VC1 0x0046ae10
unsigned KdConsole::Release()
{
    return IAbstract::Release();
}

// refs at +0x18.
// FUNCTION: VC1 0x0046ae30
unsigned KdConsole::AddRef()
{
    unsigned value = refs;
    if (value != 0) {
        ++value;
        refs = value;
    }
    return value;
}

// EDI=this. Removes the selected (completed) text, then clamps the cursor.
// FUNCTION: VC1 0x0046ae40
void KdConsole::DeleteSelection()
{
    if (selectionStart >= 0 && selectionLength != 0
        && selectionStart + selectionLength <= static_cast<int>(input.size)) {
        input.Erase(selectionStart, selectionLength);
    }
    SetCursor(cursor);
}

// ECX=this, EAX=key (KD virtual key + 0x40000000). PageUp / PageDown scroll
// the output by pageStep of a page, End / Home / Left / Right move the cursor,
// Up / Down walk the history, Delete removes the character at the cursor.
// FUNCTION: VC1 0x0046ae90
void KdConsole::OnSpecialKey(unsigned key)
{
    switch (key) {
    case 0x40000021: {
        const int last = static_cast<int>(lines.items.size >> 4) - pageLines + 1;
        const int limit = last < 0 ? 0 : last;
        const int next = FloorToInt(pageStep * static_cast<float>(pageLines)) + scroll;
        scroll = next < limit ? next : limit;
        break;
    }
    case 0x40000022: {
        const int next = scroll - FloorToInt(static_cast<float>(pageLines) * pageStep);
        scroll = next > 0 ? next : 0;
        break;
    }
    case 0x40000023:
        DeleteSelection();
        SetCursor(static_cast<int>(input.TextLength()));
        break;
    case 0x40000024:
        DeleteSelection();
        scroll = 0;
        SetCursor(0);
        break;
    case 0x40000025:
        DeleteSelection();
        SetCursor(cursor - 1);
        break;
    case 0x40000026:
        if (historyIndex > 0) {
            --historyIndex;
            input.Assign(reinterpret_cast<KdString*>(history.items.begin)[historyIndex]);
            SetCursor(static_cast<int>(input.size));
        }
        break;
    case 0x40000027:
        DeleteSelection();
        SetCursor(cursor + 1);
        break;
    case 0x40000028:
        if (historyIndex < static_cast<int>(history.items.size >> 4) - 1) {
            ++historyIndex;
            input.Assign(reinterpret_cast<KdString*>(history.items.begin)[historyIndex]);
            SetCursor(static_cast<int>(input.size));
        }
        break;
    case 0x40000029:
    case 0x4000002a:
    case 0x4000002b:
    case 0x4000002c:
    case 0x4000002d:
        break;
    case 0x4000002e:
        DeleteSelection();
        if (input.TextLength() != 0 && cursor < static_cast<int>(input.TextLength())) {
            char* const at = input.data() + cursor;
            EraseCountedBytes(at, at + 1, input.data() + input.size);
            --input.size;
            input.data()[input.size] = 0;
        }
        break;
    }
}

// EAX=this, RET 4 (line). Inferred name. Queues a line for Render to run, as
// the stdin reader does; nothing in the game calls it.
// FUNCTION: VC1 0x0046b060
void KdConsole::Enqueue(const KdString& line)
{
    pthread_mutex_t* const lock = mutex;

    if (lock) {
        pthread_mutex_lock(lock);
    }
    pending.Push(line);
    if (lock) {
        pthread_mutex_unlock(lock);
    }
}

// RET 8. this = IOutputStream (+8). Echoes to stdout with KD_ENABLE_STDIO,
// then appends the bytes to the output, one line per '\n'. A finished line
// naming "error" stamps errorTime (the red frame while closed). A scrolled
// view keeps its place.
// FUNCTION: VC1 0x0046b0a0
unsigned KdConsole::Write(const void* buffer, unsigned size)
{
    const char* p = static_cast<const char*>(buffer);
    const char* const end = p + size;

    if (g_kdEnableStdio) {
        g_kdStdout.driver->write(g_kdStdout.driver, buffer, 1, size, g_kdStdout.native);
        g_kdStdout.driver->flush(g_kdStdout.driver, g_kdStdout.native);
    }
    if (lines.items.size == 0) {
        lines.PushBack(KdString());
        if (scroll > 0) {
            ++scroll;
        }
    }
    for (; p != end; ++p) {
        if (*p == '\n') {
            if (LastLine(lines).Find(KdString(kConsoleError)) != -1) {
                LARGE_INTEGER now;
                QueryPerformanceCounter(&now);
                errorTime = static_cast<__int64>(static_cast<double>(now.QuadPart) * g_nsPerTick);
            }
            lines.PushBack(KdString());
            if (scroll > 0) {
                ++scroll;
            }
        }
        else {
            LastLine(lines).Append(*p);
        }
    }
    return size;
}

// cdecl console. Inferred name. The stdin reader thread: one byte at a time
// from the stdin driver while the console exists; every line is queued.
// FUNCTION: VC1 0x0046b300
static void* ReadStdin(void* arg)
{
    KdString line;

    while (g_kdConsolePrint) {
        char c;
        if (g_kdStdin.driver->read(g_kdStdin.driver, &c, 1, 1, g_kdStdin.native) == 0) {
            break;
        }
        if (c == '\n' || c == '\r') {
            KdConsole* const console = static_cast<KdConsole*>(arg);
            pthread_mutex_t* const lock = console->mutex;
            if (lock) {
                pthread_mutex_lock(lock);
            }
            console->pending.Push(line);
            if (lock) {
                pthread_mutex_unlock(lock);
            }
            line.Reset();
        }
        else {
            line.Append(c);
        }
    }
    return 0;
}

// ESI=this. Stops the reader (join), destroys the mutex, then the members.
// FUNCTION: VC1 0x0046b4c0
KdConsole::~KdConsole() throw()
{
    g_kdConsolePrint = 0;
    if (thread) {
        if (pthread_join(thread->handle, 0) == 0) {
            free(thread);
        }
        thread = 0;
    }
    if (mutex) {
        if (pthread_mutex_destroy(mutex) == 0) {
            free(mutex);
        }
        mutex = 0;
    }
    g_kdConsole = 0;
}

// RET 0x14 (this, result, line, &start, &length). Inferred name. With the
// cursor at a word end, Console.nut's main function completes the word
// before it (two characters or more); the completion replaces the word and
// *length becomes the added length, which the caller selects. Otherwise line
// is returned unchanged.
// FUNCTION: VC1 0x0046b600
KdString KdConsole::Complete(const KdString& line, int* start, int* length)
{
    static KdString delimiters(kDelimiters);
    const int position = *start;
    int wordStart;
    KdString word;

    if (position < static_cast<int>(line.size)
        && delimiters.IndexOf(line.c_str()[position], 0) == -1) {
        return line;
    }
    wordStart = line.FindLastOf(delimiters, position - 1);
    if (wordStart != -1) {
        ++wordStart;
        word.Assign(line.Substr(wordStart, position - wordStart));
    }
    else {
        wordStart = 0;
        word.Assign(line);
    }
    if (word.size < 2) {
        return line;
    }
    SquirrelVM::BeginCall(script);
    SquirrelVM::PushParam(word.c_str());
    SquirrelObject result = SquirrelVM::EndCall();
    if (result.GetType() == OT_NULL) {
        return line;
    }
    KdString completion(GetObjectString(result));
    KdString edited(line);
    unsigned last = wordStart + word.size;
    unsigned first = wordStart;
    if (last > edited.size) {
        last = edited.size;
    }
    if (first > edited.size) {
        first = edited.size;
    }
    edited.Replace(edited.data() + first, edited.data() + last, completion.c_str(),
        completion.c_str() + completion.size);
    *length = static_cast<int>(completion.size - word.size);
    return edited;
}

// ECX=this, EAX=line. Starts a new output line, logs the command and runs it.
// FUNCTION: VC1 0x0046b910
void KdConsole::Execute(const KdString& line)
{
    if (lines.items.size != 0 && LastLine(lines).size != 0) {
        lines.Append(KdString());
    }
    scroll = 0;
    HostLog(&g_kdNameConsole, kPercentS, line.c_str());
    SquirrelVM::RunScript(SquirrelVM::CompileBuffer(line.c_str()));
}

// EAX=this, RET 4. A typed character: Backspace, Tab (completion), Enter (run
// and keep in the history), Escape (close); others are inserted at the cursor
// over the selection.
// FUNCTION: VC1 0x0046ba20
void KdConsole::OnChar(char c)
{
    switch (c) {
    case '\b':
        DeleteSelection();
        if (input.TextLength() != 0 && cursor > 0) {
            char* const at = input.data() + cursor - 1;
            EraseCountedBytes(at, at + 1, input.data() + input.size);
            --input.size;
            input.data()[input.size] = 0;
            SetCursor(cursor - 1);
        }
        return;
    case '\t':
        SetCursor(selectionStart + selectionLength);
        input.Assign(Complete(input, &selectionStart, &selectionLength));
        return;
    case '\n':
    case '\r':
        DeleteSelection();
        if (input.TextLength() == 0) {
            return;
        }
        Execute(input);
        history.Append(input);
        historyIndex = static_cast<int>(history.items.size >> 4);
        input.Reset();
        SetCursor(0);
        return;
    case 0x1b: {
        IManualControl* const control = this;
        control->EnableControl(!control->IsControlEnabled());
        return;
    }
    default:
        if (c == 0) {
            return;
        }
        DeleteSelection();
        {
            const unsigned at = static_cast<unsigned>(cursor);
            const unsigned length = input.size + 1;
            char* text;
            if (input.capacity < input.Count() + length) {
                GrowCountedBuffer(&input, static_cast<int>(length), 0);
            }
            input.size = length;
            text = input.data();
            EraseCountedBytes(text + at, text + length - 1, text + length);
            input.data()[at] = c;
            input.data()[input.size] = 0;
        }
        SetCursor(cursor + 1);
        return;
    }
}

// RET 4. this = IInputHandler. Pressed keys only: the backquote opens and
// closes the console; while open, characters and special keys edit it.
// FUNCTION: VC1 0x0046bc40
unsigned char KdConsole::OnKey(const unsigned* data)
{
    const unsigned key = data[0];

    if (data[2] == 0) {
        return 0;
    }
    if (key >= 0x40000100 && key < 0x400001ff) {
        const char c = static_cast<char>(key);
        if (c == '`') {
            IManualControl* const control = this;
            control->EnableControl(!control->IsControlEnabled());
            return 1;
        }
        if (!open) {
            return 0;
        }
        OnChar(c);
        return 1;
    }
    if (!open) {
        return 0;
    }
    OnSpecialKey(key);
    return 1;
}

// ESI=this. Runs scripts/sys/Console.nut for the completion function and,
// with KD_ENABLE_STDIO, creates the queue mutex (pthread_mutex_init inlined)
// and the stdin reader thread.
// FUNCTION: VC1 0x0046bcc0
KdConsole::KdConsole() throw()
    : mutex(0), thread(0)
{
    open = false;
    visible = false;
    historyIndex = 0;
    scroll = 0;
    selectionLength = 0;
    g_kdConsolePrint = ConsolePrint;
    SetCursor(0);
    script = SquirrelVM::RunScript(SquirrelVM::CompileScript(kConsoleScript));
    height = 1.0f;
    pageStep = 0.3f;
    errorTime = 0;
    if (!g_kdEnableStdio) {
        return;
    }

    pthread_mutex_t* lock = static_cast<pthread_mutex_t*>(malloc(sizeof(pthread_mutex_t)));
    if (!lock || pthread_mutex_init(lock, 0) != 0) {
        free(lock);
        lock = 0;
    }
    mutex = lock;

    KdThread* const reader = static_cast<KdThread*>(malloc(sizeof(KdThread)));
    KdThread* started = reader;
    reader->autoFree = 0;
    reader->arg = this;
    reader->proc = ReadStdin;
    if (pthread_create(&reader->handle, 0, KdThreadStart, reader) != 0) {
        free(reader);
        started = 0;
    }
    thread = started;
}

// The singleton: the first call builds it, later calls return it.
// FUNCTION: VC1 0x0046bf20
void** CreateKdConsole(void** out)
{
    KdConsole* object;
    IUnknown* queried;

    if (g_kdConsole) {
        queried = static_cast<IUnknown*>(g_kdConsole->QueryInterface(&g_iidAbstract));
        *out = queried;
        if (queried) {
            queried->AddRef();
        }
        return out;
    }
    object = static_cast<KdConsole*>(malloc(sizeof(KdConsole)));
    if (object && (object = new (object) KdConsole()) != 0) {
        g_kdConsole = static_cast<IAbstract*>(object);
        if (!g_kdConsole) {
            queried = 0;
        }
        else {
            queried = static_cast<IUnknown*>(g_kdConsole->QueryInterface(&g_iidAbstract));
            if (queried) {
                queried->AddRef();
            }
        }
    }
    else {
        g_kdConsole = 0;
        queried = 0;
    }
    queried->Release();
    *out = queried;
    queried->AddRef();
    queried->Release();
    return out;
}

// RET 4. graphics is the IGraphics reference. Runs the queued stdin lines.
// Closed and not visible: only a fading red frame for a second after an
// error line. Otherwise the console fills the top share of the view (a tenth
// while closed): the input line with the selected completion and the cursor
// when open, then the output from the newest line up, colored by tags.
// FUNCTION: VC1 0x0046bfd0
int KdConsole::Render(void* graphics)
{
    KdPtr<IGraphics>& g = *static_cast<KdPtr<IGraphics>*>(graphics);
    float share = height;
    bool shown = true;
    pthread_mutex_t* const lock = mutex;

    if (lock) {
        pthread_mutex_lock(lock);
    }
    while ((pending.list.items.size >> 4) != pending.done) {
        Execute(reinterpret_cast<KdString*>(pending.list.items.begin)[pending.done]);
        ++pending.done;
        if (pending.done >= (pending.list.items.size >> 4)) {
            pending.done = 0;
            pending.list.items.size = 0;
        }
    }
    if (lock) {
        pthread_mutex_unlock(lock);
    }

    if (!open) {
        if (!visible) {
            if (errorTime == 0) {
                return 0;
            }
            LARGE_INTEGER now;
            QueryPerformanceCounter(&now);
            int remaining = 1000 - static_cast<int>(static_cast<unsigned __int64>(
                static_cast<__int64>(static_cast<double>(now.QuadPart) * g_nsPerTick) - errorTime)
                / 1000000);
            if (remaining <= 0) {
                errorTime = 0;
                return 0;
            }
            const unsigned faded = 0x00ff0000;
            const unsigned red = 0xffff0000;
            const int start = 0;
            const int thin = 0;
            const int thick = 16;
            unsigned color;
            InterpolateColor(faded, &color, red, start, remaining);
            DrawFrame(g, InterpolateInt(start, thin, thick, remaining), color);
            return 0;
        }
        shown = false;
        share = 0.1f;
    }

    Rect view = *g->GetView();
    view.height = static_cast<int>(floorf(static_cast<float>(view.height) * share));
    if (!font) {
        KdString path(kConsoleFont);
        KdRef<IResourceManager> manager;
        KdPtr<IUnknown> loaded((*GetKdResourceManager(&manager.ptr))->GetFont(path));

        AssignFont(&font, &loaded.ptr);
        pageLines = view.height / font->GetHeight();
    }

    Matrix3 saved = g->GetTransform();
    g->SetTransform(Matrix3::Identity);
    g->SetFont(*reinterpret_cast<const KdPtr<IFont>*>(&font.ptr));
    g->SetColor(0xbb000000);
    g->FillRect(static_cast<float>(view.x), static_cast<float>(view.y),
        static_cast<float>(view.width), static_cast<float>(view.height));
    const int lineHeight = font->GetHeight();
    int y = view.y - (shown ? lineHeight : 0) + view.height;

    if (shown) {
        input.Assign(Complete(input, &selectionStart, &selectionLength));
        KdString text = KdString(kPrompt) + input;
        g->SetColor(0xff7fff00);
        g->DrawString(text.c_str(), static_cast<float>(view.x), static_cast<float>(y));

        KdString selected = text.Substr(selectionStart + 1, selectionLength);
        const int selectedX = font->GetTextWidth(text.Slice(0, selectionStart + 1).c_str()) + view.x;
        g->SetColor(0xff8fbc8b);
        g->DrawString(selected.c_str(), static_cast<float>(selectedX), static_cast<float>(y));

        const int cursorX = font->GetTextWidth(text.Slice(0, cursor + 1).c_str()) + view.x;
        g->SetColor(0xffffffff);
        g->DrawString(kCursor, static_cast<float>(cursorX), static_cast<float>(y));
        y -= lineHeight;
    }

    KdString* const first = reinterpret_cast<KdString*>(lines.items.begin);
    for (KdString* line = first + (lines.items.size >> 4) - scroll; line != first; ) {
        --line;
        unsigned color = 0xffffffff;
        if (line->Find(KdString(kConsoleTag)) != -1) {
            color = 0xff7fff00;
        }
        else if (line->Find(KdString(kConsoleError)) != -1
            || line->Find(KdString(kConsoleErrorUpper)) != -1) {
            color = 0xffff0000;
        }
        else if (line->Find(KdString(kConsoleWarning)) != -1) {
            color = 0xffffff00;
        }
        else if (line->Find(KdString(kConsoleInfo)) != -1) {
            color = 0xff00ffff;
        }
        g->SetColor(color);
        g->DrawString(line->c_str(), static_cast<float>(view.x), static_cast<float>(y));
        y -= lineHeight;
        if (y < view.y) {
            break;
        }
    }
    g->SetTransform(saved);
    return 0;
}

}
