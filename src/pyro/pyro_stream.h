#ifndef VC1_PYRO_STREAM_H
#define VC1_PYRO_STREAM_H

namespace vc1 {

// Inferred names throughout. The Pyro library reads .pyro files from a
// caller-supplied stream with four slots (the game's is KdPyroStream,
// vtable 0x004d69a8).
// SIZE 0x04
struct IPyroStream
{
    virtual int Read(void* buffer, int size) = 0;        // vtable+0x00
    virtual int Write(const void* buffer, int size) = 0; // vtable+0x04
    virtual int Seek(int offset, int origin) = 0;        // vtable+0x08
    virtual int Tell() = 0;                              // vtable+0x0c
};

// Library-side stream base. Deleting destructor 0x0042f120 only stores this
// vptr, so derived streams with trivial destructors share it.
// SIZE 0x04
// VTABLE: VC1 0x004d7bc0
class PyroStreamBase
{
public:
    virtual ~PyroStreamBase() {}                         // vtable+0x00
    virtual int Read(void* buffer, int size) = 0;        // vtable+0x04
    virtual int Write(const void* buffer, int size) = 0; // vtable+0x08
    virtual int Seek(int offset, int origin) = 0;        // vtable+0x0c
    virtual int Tell() = 0;                              // vtable+0x10
    virtual int GetLength();                             // vtable+0x14
    virtual int Skip(int count);                         // vtable+0x18
};

// SYNTHETIC: VC1 0x0042f120 FOLDED
// vc1::PyroStreamBase::`scalar deleting destructor'
// SYNTHETIC: VC1 0x0042f120 FOLDED
// vc1::PyroStreamReader::`scalar deleting destructor'
// SYNTHETIC: VC1 0x0042f120 FOLDED
// vc1::PyroSubStream::`scalar deleting destructor'
// SYNTHETIC: VC1 0x0042f120 FOLDED
// vc1::PyroKdFileStream::`scalar deleting destructor'

// Adapter from IPyroStream; built on the stack by PyroLibrary::LoadFile.
// SIZE 0x08
// VTABLE: VC1 0x004d7c9c
class PyroStreamReader : public PyroStreamBase
{
public:
    IPyroStream* stream; // 0x04

    explicit PyroStreamReader(IPyroStream* source) : stream(source) {}

    int Read(void* buffer, int size);
    int Write(const void* buffer, int size);
    int Seek(int offset, int origin);
    int Tell();
};

// Serialization context passed to every Deserialize: the stream and a
// direction flag (false = loading).
// SIZE 0x08
struct PyroArchive
{
    PyroStreamBase* stream; // 0x00
    bool storing;           // 0x04

    int Read(void* buffer, int size) { return stream->Read(buffer, size); }
};

typedef char Vc1PyroStreamReaderSizeCheck[sizeof(PyroStreamReader) == 0x08 ? 1 : -1];

}

#endif
