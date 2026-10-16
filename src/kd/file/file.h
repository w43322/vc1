#ifndef VC1_KD_FILE_FILE_H
#define VC1_KD_FILE_FILE_H

#include "kd/iface/iabstract.h"
#include "kd/iface/icomponent_faces.h"
#include "kd/iface/iscript_object.h"
#include "kd/iface/ishutdownable.h"
#include "kd/stream/file_reader.h"

namespace vc1 {

extern const char* g_kdNameFile;

// Factory "File": a script-openable KD file read and written as a stream
// (Level.nut "Load" opens res/<level>.lvl and .lvl.dat with "Open" and
// reads them through readComponent). QI at 0x0046a690: IInputStream this,
// IOutputStream +4, IScriptObject +8, IShutdownable +0x0c, IAbstract +0x10.
// SIZE 0x1c
// VTABLE: VC1 0x004d587c vc1::IInputStream
// VTABLE: VC1 0x004d588c vc1::IOutputStream
// VTABLE: VC1 0x004d589c vc1::IScriptObject
// VTABLE: VC1 0x004d58b4 vc1::IShutdownable
// VTABLE: VC1 0x004d58c4 vc1::IAbstract
class KdFile : public IInputStream,
               public IOutputStream,
               public IScriptObject,
               public IShutdownable,
               public IAbstract
{
public:
    KdFileHandle* handle; // 0x18  OpenKdFile result, or null

    KdFile() throw();
    ~KdFile();
    void* QueryInterface(const char** iid);
    unsigned AddRef();
    unsigned Release();
    unsigned Read(void* buffer, unsigned size);                     // IInputStream vtable+0x0c
    unsigned Write(const void* buffer, unsigned size);              // IOutputStream vtable+0x0c
    unsigned char GetMember(const char* name, SquirrelObject* out); // IScriptObject vtable+0x0c
    unsigned char SetMember(void* name, void* value);               // IScriptObject vtable+0x10
    const char* GetTypeName();                                      // IScriptObject vtable+0x14
    int Shutdown();                                                 // IShutdownable vtable+0x0c

    bool Open(const char* path, const char* mode);
};

typedef char Vc1KdFileSizeCheck[sizeof(KdFile) == 0x1c ? 1 : -1];

void** CreateKdFile(void** out);

// SYNTHETIC: VC1 0x00418590
// vc1::KdFile::`scalar deleting destructor'

// SYNTHETIC: VC1 0x00418710
// ??_EKdFile@vc1@@WBA@AEPAXI@Z
// SYNTHETIC: VC1 0x00418720
// ?AddRef@KdFile@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x00418730
// ?AddRef@KdFile@vc1@@WBA@AEIXZ
// SYNTHETIC: VC1 0x00418740
// ?Release@KdFile@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x00418750
// ?QueryInterface@KdFile@vc1@@W3AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00418760
// ?Release@KdFile@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x00418770
// ?QueryInterface@KdFile@vc1@@W7AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00418780
// ?Release@KdFile@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x00418790
// ?QueryInterface@KdFile@vc1@@WM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x004187a0
// ?AddRef@KdFile@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x004187b0
// ?Release@KdFile@vc1@@WBA@AEIXZ
// SYNTHETIC: VC1 0x004187c0
// ?QueryInterface@KdFile@vc1@@WBA@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x004187d0
// ?AddRef@KdFile@vc1@@W7AEIXZ

}

#endif
