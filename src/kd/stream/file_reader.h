#ifndef VC1_KD_FILE_READER_H
#define VC1_KD_FILE_READER_H

#include "kd/factory.h"
#include "kdcore/platform.h"

namespace vc1 {

// OpenKdFile result: the file-system driver and its native handle.
// SIZE 0x08
struct KdFileHandle
{
    KdObject* driver; // 0x00
    int native;       // 0x04
};

typedef char Vc1KdFileHandleSizeCheck[sizeof(KdFileHandle) == 0x08 ? 1 : -1];

// Inferred names for the stream state bits: 1 error, 2 end of file, 4 failed.
enum {
    kStreamError = 1,
    kStreamEof = 2,
    kStreamFail = 4
};

// Inferred name. Owns one KdFileHandle; no vftable. Embedded in KdFileReader.
// SIZE 0x18
struct KdFileSource
{
    unsigned short state;  // 0x00
    unsigned short state2; // 0x02
    KdFileHandle* file;    // 0x04
    KdString name;         // 0x08

    KdFileSource() : state(0), state2(0), file(0) {}
    ~KdFileSource() throw()
    {
        state = 0;
        state2 = 0;
        Close();
    }

    void Attach(const char* path, KdFileHandle* handle);
    void Close();
    unsigned Read(void* buffer, unsigned length);
};

typedef char Vc1KdFileSourceSizeCheck[sizeof(KdFileSource) == 0x18 ? 1 : -1];

extern const char g_streamDelimiters[];
extern volatile unsigned g_streamBlockSize;

// Inferred name. Buffered input range on KdRange: (begin, size) is the valid
// data, pos the read offset. Its implicit destructor folds with KdRange's
// (0x00401f80 in slot 0).
// SIZE 0x28
// VTABLE: VC1 0x004d7ba0
class KdBufferStream : public KdRange
{
public:
    unsigned short state;     // 0x0c  kStream* bits
    unsigned short state2;    // 0x0e
    unsigned pos;             // 0x10
    char delimiters[0x10];    // 0x14
    unsigned char skipBlanks; // 0x24

    KdBufferStream() : state(0), pos(0), skipBlanks(0)
    {
        state2 = 0;
        memset(delimiters, 0, sizeof(delimiters));
        memcpy(delimiters, g_streamDelimiters, 9);
    }

    void Clear();                         // vtable+0x04
    virtual unsigned Fill(unsigned want); // vtable+0x08

    char SkipBlanks();
    void ReadToken(KdString& out);
    KdBufferStream& ReadLine(KdString& out);
};

typedef char Vc1KdBufferStreamSizeCheck[sizeof(KdBufferStream) == 0x28 ? 1 : -1];

// Inferred name. Text file input stream ("rt"): a 0xff-byte KdString buffer
// refilled from a KdFileSource. Constructed on the stack by KdFont::Load.
// SIZE 0x50
// VTABLE: VC1 0x004d7b88
class KdFileReader : public KdBufferStream
{
public:
    KdString buffer;     // 0x28
    KdFileSource source; // 0x38

    explicit KdFileReader(const char* path) throw();
    ~KdFileReader() throw();

    unsigned Fill(unsigned want); // vtable+0x08
};

typedef char Vc1KdFileReaderSizeCheck[sizeof(KdFileReader) == 0x50 ? 1 : -1];

// SYNTHETIC: VC1 0x0042e3a0
// vc1::KdFileReader::`scalar deleting destructor'

}

#endif
