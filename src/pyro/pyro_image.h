#ifndef VC1_PYRO_IMAGE_H
#define VC1_PYRO_IMAGE_H

#include <stdlib.h>

#include "pyro/pyro_stream.h"

namespace vc1 {

// Inferred names throughout (Pyro library image loading).

// Growable memory stream over a caller buffer; built on the stack over the
// image bytes kept in a .pyro file. Write grows a buffer it then owns.
// SIZE 0x18
// VTABLE: VC1 0x004d8030
class PyroMemoryStream : public PyroStreamBase
{
public:
    int size;            // 0x04
    int capacity;        // 0x08
    int position;        // 0x0c
    unsigned char* data; // 0x10
    bool field14;        // 0x14
    bool owns;           // 0x15

    PyroMemoryStream(void* buffer, int length)
        : size(length), capacity(0), position(0),
          data(static_cast<unsigned char*>(buffer)), field14(false), owns(false)
    {
    }
    ~PyroMemoryStream()
    {
        if (owns) {
            free(data);
        }
    }

    int Read(void* buffer, int count);
    int Write(const void* buffer, int count);
    int Seek(int offset, int origin);
    int Tell();
    int GetLength();
};

// SYNTHETIC: VC1 0x0042f150
// vc1::PyroMemoryStream::`scalar deleting destructor'

// Stream interface of the image decoders.
// SIZE 0x04
// VTABLE: VC1 0x004d7bf8
class PyroImageStream
{
public:
    virtual ~PyroImageStream() {}                        // vtable+0x00
    virtual int Read(void* buffer, int size) = 0;        // vtable+0x04
    virtual int Write(const void* buffer, int size) = 0; // vtable+0x08
    virtual int GetLength() = 0;                         // vtable+0x0c
    virtual int Seek(int offset, int origin) = 0;        // vtable+0x10
    virtual int Tell() = 0;                              // vtable+0x14
};

// The derived deleting destructor only stores the base vptr, so it is the
// same body.
// SYNTHETIC: VC1 0x0042e8a0
// vc1::PyroImageStream::`scalar deleting destructor'

// Read-only adapter from a library stream.
// SIZE 0x08
// VTABLE: VC1 0x004d7cb8
class PyroImageStreamReader : public PyroImageStream
{
public:
    PyroStreamBase* stream; // 0x04

    explicit PyroImageStreamReader(PyroStreamBase* source) : stream(source) {}

    int Read(void* buffer, int size);
    int Write(const void* buffer, int size);
    int GetLength();
    int Seek(int offset, int origin);
    int Tell();
};

// Decoded image: 32-bit pixels (B, G, R, A in memory).
// SIZE 0x0c
struct PyroImage
{
    int width;    // 0x00
    int height;   // 0x04
    void* pixels; // 0x08

    PyroImage() : width(0), height(0), pixels(0) {}
    ~PyroImage()
    {
        if (pixels) {
            free(pixels);
        }
    }

    void Clear()
    {
        if (pixels) {
            free(pixels);
            pixels = 0;
        }
        height = 0;
        width = 0;
    }
};

// ESI=this. Out of line only for PyroDecodePsd's two failure paths.
// FUNCTION: VC1 0x0042f2a0
// ?Clear@PyroImage@vc1@@QAEXXZ

bool PyroDecodeImage(PyroImageStream* stream, PyroImage* image);

typedef char Vc1PyroMemoryStreamSizeCheck[sizeof(PyroMemoryStream) == 0x18 ? 1 : -1];
typedef char Vc1PyroImageStreamReaderSizeCheck[sizeof(PyroImageStreamReader) == 0x08 ? 1 : -1];

}

#endif
