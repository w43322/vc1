#ifndef VC1_KD_BLOB_BLOB_H
#define VC1_KD_BLOB_BLOB_H

#include "kd/factory.h"
#include "kd/iface/iabstract.h"
#include "kd/iface/icomponent_faces.h"
#include "kd/iface/iscript_object.h"

class SquirrelObject;

namespace vc1 {

// Inferred name. Factory "Blob": a growable byte buffer that is both an
// input and an output stream with one shared position. In a stream it is a
// size header (headerSize bytes, 4 by default; Level.nut uses 2 for the
// object records) followed by the bytes; in a script table it is a hex
// string. QI 0x0046c990 establishes the face offsets below.
// SIZE 0x3c
// VTABLE: VC1 0x004d56d0 vc1::IInputStream
// VTABLE: VC1 0x004d56e0 vc1::IOutputStream
// VTABLE: VC1 0x004d56f0 vc1::IReadable
// VTABLE: VC1 0x004d5700 vc1::IWriteable
// VTABLE: VC1 0x004d5710 vc1::IScriptLoadable
// VTABLE: VC1 0x004d5720 vc1::IScriptSaveable
// VTABLE: VC1 0x004d5730 vc1::IScriptObject
// VTABLE: VC1 0x004d5748 vc1::IAbstract
class KdBlob : public IInputStream,
               public IOutputStream,
               public IReadable,
               public IWriteable,
               public IScriptLoadable,
               public IScriptSaveable,
               public IScriptObject,
               public IAbstract
{
public:
    KdArray<unsigned char> data; // 0x24
    unsigned position;           // 0x34 read and write position
    int headerSize;              // 0x38 bytes of the size header

    KdBlob() throw();
    ~KdBlob() throw();
    void* QueryInterface(const char** iid);
    unsigned AddRef();
    unsigned Release();

    unsigned Read(void* buffer, unsigned size);
    unsigned Write(const void* buffer, unsigned size);
    bool ReadFrom(const KdPtr<IInputStream>& stream);
    bool WriteTo(const KdPtr<IOutputStream>& stream);
    bool LoadFromScript(SquirrelObject& table);
    SquirrelObject SaveToScript();

    unsigned char GetMember(const char* name, SquirrelObject* out);
    unsigned char SetMember(void* name, void* value);
    const char* GetTypeName();

    void SetHeaderSize(int size);
};

typedef char Vc1KdBlobSizeCheck[sizeof(KdBlob) == 0x3c ? 1 : -1];

void** CreateKdBlob(void** out);

// SYNTHETIC: VC1 0x00417d50
// vc1::KdBlob::`scalar deleting destructor'

// TEMPLATE: VC1 0x00417de0
// ??$SetByValue@PAD@SquirrelObject@@QAE?AV0@PAD@Z

// SYNTHETIC: VC1 0x00417ea0
// ?Release@KdBlob@vc1@@WBM@AEIXZ
// SYNTHETIC: VC1 0x00417eb0
// ?AddRef@KdBlob@vc1@@WBE@AEIXZ
// SYNTHETIC: VC1 0x00417ec0
// ??_EKdBlob@vc1@@WBM@AEPAXI@Z
// SYNTHETIC: VC1 0x00417ed0
// ?QueryInterface@KdBlob@vc1@@W3AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00417ee0
// ?AddRef@KdBlob@vc1@@WBI@AEIXZ
// SYNTHETIC: VC1 0x00417ef0
// ?QueryInterface@KdBlob@vc1@@W7AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00417f00
// ?Release@KdBlob@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x00417f10
// ?AddRef@KdBlob@vc1@@WBM@AEIXZ
// SYNTHETIC: VC1 0x00417f20
// ?QueryInterface@KdBlob@vc1@@WM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00417f30
// ?Release@KdBlob@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x00417f40
// ?QueryInterface@KdBlob@vc1@@WBA@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00417f50
// ?Release@KdBlob@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x00417f60
// ?AddRef@KdBlob@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x00417f70
// ?QueryInterface@KdBlob@vc1@@WBE@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00417f80
// ?Release@KdBlob@vc1@@WBA@AEIXZ
// SYNTHETIC: VC1 0x00417f90
// ?AddRef@KdBlob@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x00417fa0
// ?QueryInterface@KdBlob@vc1@@WBI@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00417fb0
// ?Release@KdBlob@vc1@@WBE@AEIXZ
// SYNTHETIC: VC1 0x00417fc0
// ?AddRef@KdBlob@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x00417fd0
// ?QueryInterface@KdBlob@vc1@@WBM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00417fe0
// ?Release@KdBlob@vc1@@WBI@AEIXZ
// SYNTHETIC: VC1 0x00417ff0
// ?AddRef@KdBlob@vc1@@WBA@AEIXZ

}

#endif
