#ifndef VC1_KD_CONSOLE_CONSOLE_H
#define VC1_KD_CONSOLE_CONSOLE_H

#include "kd/factory.h"
#include "kd/iface/iabstract.h"
#include "kd/iface/icomponent_faces.h"
#include "kd/iface/ifont.h"
#include "kd/iface/iinput_handler.h"
#include "kd/iface/irenderable.h"
#include "kdcore/runtime.h"
#include "bindings/squirrel_object.h"

namespace vc1 {

extern const char* g_kdNameConsole;

// Inferred name. Lines read from stdin, executed by Render. done counts the
// executed ones; the next push erases them first.
// SIZE 0x14
struct KdStringQueue
{
    KdStringList list; // 0x00
    unsigned done;     // 0x10

    __declspec(noinline) void Push(const KdString& line);
};

// Factory "Console" (singleton g_kdConsole). The in-game script console: the
// backquote key opens it (IManualControl), typed lines run as Squirrel and
// are kept in a history, KdLog output is collected line by line through
// IOutputStream, and scripts/sys/Console.nut completes the word before the
// cursor. With KD_ENABLE_STDIO set, output is echoed to stdout and lines
// read from stdin run too. QI at 0x0046ada0: IRenderable this,
// IInputHandler +4, IOutputStream +8, IManualControl +0x0c, IVisible +0x10,
// IAbstract +0x14.
// SIZE 0xa8
// VTABLE: VC1 0x004d57ec vc1::IRenderable
// VTABLE: VC1 0x004d5804 vc1::IInputHandler
// VTABLE: VC1 0x004d5818 vc1::IOutputStream
// VTABLE: VC1 0x004d5828 vc1::IManualControl
// VTABLE: VC1 0x004d583c vc1::IVisible
// VTABLE: VC1 0x004d5850 vc1::IAbstract
class KdConsole : public IRenderable,
                  public IInputHandler,
                  public IOutputStream,
                  public IManualControl,
                  public IVisible,
                  public IAbstract
{
public:
    KdRef<IFont> font;        // 0x1c  res/fonts/console.fnt, loaded on first draw
    bool open;                // 0x20  IManualControl: the console takes input
    bool visible;             // 0x21  IVisible: drawn (one tenth high) while closed
    KdStringList lines;       // 0x24  output, the last line being written
    KdString input;           // 0x34
    KdStringQueue pending;    // 0x44  stdin lines (under mutex)
    KdStringList history;     // 0x58
    int historyIndex;         // 0x68
    int cursor;               // 0x6c
    int scroll;               // 0x70  output lines scrolled back
    int pageLines;            // 0x74  output lines per screen
    int selectionStart;       // 0x78  completed suffix, replaced by the next edit
    int selectionLength;      // 0x7c
    SquirrelObject script;    // 0x80  result of running scripts/sys/Console.nut
    float height;             // 0x8c  share of the view when open
    float pageStep;           // 0x90  share of a page per PageUp / PageDown
    __int64 errorTime;        // 0x98  ns of the last output line naming "error"
    pthread_mutex_t* mutex;   // 0xa0  only with KD_ENABLE_STDIO
    KdThread* thread;         // 0xa4  stdin reader, only with KD_ENABLE_STDIO

    KdConsole() throw();
    ~KdConsole() throw();
    void* QueryInterface(const char** iid);
    unsigned AddRef();
    unsigned Release();
    int Render(void* graphics);                       // IRenderable vtable+0x10
    int GetRenderOrder();                             // IRenderable vtable+0x14
    unsigned char OnKey(const unsigned* data);        // IInputHandler vtable+0x0c
    unsigned char OnPointer(const unsigned* data);    // IInputHandler vtable+0x10
    unsigned Write(const void* buffer, unsigned size); // IOutputStream vtable+0x0c
    void EnableControl(bool enable);                  // IManualControl vtable+0x0c
    bool IsControlEnabled();                          // IManualControl vtable+0x10
    void SetVisible(bool visible);                    // IVisible vtable+0x0c
    bool IsVisible();                                 // IVisible vtable+0x10

    void SetCursor(int position);
    void DeleteSelection();
    void OnSpecialKey(unsigned key);
    void Enqueue(const KdString& line);
    void Execute(const KdString& line);
    KdString Complete(const KdString& line, int* start, int* length);
    void OnChar(char c);
};

typedef char Vc1KdConsoleSizeCheck[sizeof(KdConsole) == 0xa8 ? 1 : -1];

void** CreateKdConsole(void** out);

// SYNTHETIC: VC1 0x00418070
// vc1::KdConsole::`scalar deleting destructor'
// SYNTHETIC: VC1 0x00418380
// ?Release@KdConsole@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x00418390
// ?QueryInterface@KdConsole@vc1@@W7AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x004183a0
// ?Release@KdConsole@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x004183b0
// ?QueryInterface@KdConsole@vc1@@WM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x004183c0
// ?AddRef@KdConsole@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x004183d0
// ?Release@KdConsole@vc1@@WBA@AEIXZ
// SYNTHETIC: VC1 0x004183e0
// ?QueryInterface@KdConsole@vc1@@WBA@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x004183f0
// ?AddRef@KdConsole@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x00418400
// ?Release@KdConsole@vc1@@WBE@AEIXZ
// SYNTHETIC: VC1 0x00418410
// ?QueryInterface@KdConsole@vc1@@WBE@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00418420
// ?AddRef@KdConsole@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x00418430
// ??_EKdConsole@vc1@@WBE@AEPAXI@Z
// SYNTHETIC: VC1 0x00418440
// ?AddRef@KdConsole@vc1@@WBA@AEIXZ
// SYNTHETIC: VC1 0x00418450
// ?AddRef@KdConsole@vc1@@WBE@AEIXZ
// SYNTHETIC: VC1 0x00418460
// ?Release@KdConsole@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x00418470
// ?QueryInterface@KdConsole@vc1@@W3AEPAXPAPBD@Z

}

#endif
