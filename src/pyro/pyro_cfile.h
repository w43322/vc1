#ifndef VC1_PYRO_CFILE_H
#define VC1_PYRO_CFILE_H

#include "pyro/pyro_stream.h"
#include "pyro/pyro_string.h"

namespace vc1 {

// Inferred names throughout. The Pyro library's own file layer, reached only
// from PyroLibrary::LoadFromPath (0x004566b0, which nothing calls): the
// "CFile::Open(...)" text names the class.

// A KD file handle (OpenKdFile) as a stream. Every slot checks the handle and
// sets errno 0x11 without one.
// SIZE 0x08
// VTABLE: VC1 0x004d8014
class PyroKdFileStream : public PyroStreamBase
{
public:
    void* file; // 0x04  KdFileHandle

    explicit PyroKdFileStream(void* handle) : file(handle) {}

    int Read(void* buffer, int size);
    int Write(const void* buffer, int size);
    int Seek(int offset, int origin);
    int Tell();
    int GetLength();
};

// A window [start, start + length) of another stream; positions are absolute
// in the inner stream and reported relative to start.
// SIZE 0x14
// VTABLE: VC1 0x004d7ff8
class PyroSubStream : public PyroStreamBase
{
public:
    int start;              // 0x04
    int length;             // 0x08
    PyroStreamBase* inner;  // 0x0c
    int position;           // 0x10

    explicit PyroSubStream(PyroStreamBase* source)
    {
        inner = source;
        length = source->GetLength();
        source->Seek(0, 0);
        position = 0;
        start = 0;
    }

    int Read(void* buffer, int size);
    int Write(const void* buffer, int size);
    int Seek(int offset, int origin);
    int Tell();
};

class PyroCFile;

// The static options of CFile::Open: opening is enabled and the path is used
// as is when root is empty, else root + '/' + path.
// SIZE 0x108
struct PyroFileSystem
{
    bool enabled;     // 0x00
    char root[0x107]; // 0x01

    PyroFileSystem() : enabled(true) { root[0] = 0; }
    ~PyroFileSystem() {}

    bool Open(const char* path, PyroCFile* file);
};

// A stream over a named file: the KD file stream and a whole-file window on
// it. Built on the stack by PyroLibrary::LoadFromPath.
// SIZE 0x14
// VTABLE: VC1 0x004d7bdc
class PyroCFile : public PyroStreamBase
{
public:
    PyroKdFileStream* file; // 0x04
    PyroSubStream* stream;  // 0x08
    bool field0c;           // 0x0c  never read
    PyroString name;        // 0x10

    PyroCFile() : file(0), stream(0), field0c(false) { name.data = g_pyroStringEmptyData; }
    ~PyroCFile()
    {
        Close();
        name.Release();
    }

    int Read(void* buffer, int size) { return stream->Read(buffer, size); }
    int Write(const void* buffer, int size);
    int Seek(int offset, int origin) { return stream->Seek(offset, origin); }
    int Tell() { return stream->Tell(); }
    int GetLength() { return stream->GetLength(); }

    bool Open(const char* path);
    void Attach(void* handle);
    void Close();
};

// SYNTHETIC: VC1 0x0042e740
// vc1::PyroCFile::`scalar deleting destructor'
// FUNCTION: VC1 0x0044dbc0
// ?Read@PyroCFile@vc1@@UAEHPAXH@Z
// FUNCTION: VC1 0x0044dbd0
// ?Tell@PyroCFile@vc1@@UAEHXZ
// FUNCTION: VC1 0x0044dbe0
// ?Seek@PyroCFile@vc1@@UAEHHH@Z
// FUNCTION: VC1 0x0044dbf0
// ?GetLength@PyroCFile@vc1@@UAEHXZ

void PyroLog(const char* fmt, ...);

}

#endif
