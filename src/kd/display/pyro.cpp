#include "kd/display/pyro.h"

#include <stdlib.h>

namespace vc1 {

// Inlined into the scalar deleting destructor (0x0042ed40).
PyroLibrary::~PyroLibrary()
{
    if (vertices) {
        delete[] vertices;
        vertices = 0;
    }
    if (vertexBuffer) {
        delete vertexBuffer;
        vertexBuffer = 0;
    }
}

// FUNCTION: VC1 0x00450080
void PyroLibrary::Done()
{
    if (vertices) {
        delete[] vertices;
        vertices = 0;
    }
    if (vertexBuffer) {
        delete vertexBuffer;
        vertexBuffer = 0;
    }
}

// RET 4. Device buffer of 0x800 vertices (stride 0x18) plus the scratch copy.
// EAX keeps the scratch pointer; SetSize ignores it.
// FUNCTION: VC1 0x004500c0
void PyroLibrary::Init(IPyroDevice* value)
{
    device = value;
    capacity = 0x200;
    vertexCursor = 0;
    value->Pyro0c(&vertexBuffer, 0x800, 0x18, 0x10009, 0);
    vertices = new PyroVertex[capacity * 4];
}

// RET 4. Body not recovered.
// STUB: VC1 0x004566b0
int PyroLibrary::Pyro04(void* a)
{
    (void)a;
    return 0;
}

// RET 4. Deserializes a .pyro file; the file is dropped unless the whole
// stream was consumed.
// FUNCTION: VC1 0x00456800
PyroFile* PyroLibrary::LoadFile(IPyroStream* stream)
{
    PyroStreamReader reader(stream);
    PyroFile* const file = new PyroFile(this);
    PyroArchive ar;

    ar.stream = &reader;
    ar.storing = false;
    file->Deserialize(ar);
    if (stream->Tell() != reader.GetLength()) {
        if (file) {
            delete file;
        }
        return 0;
    }
    return file;
}

}
