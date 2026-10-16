#ifndef VC1_PYRO_FILE_H
#define VC1_PYRO_FILE_H

#include "pyro/pyro_device.h"
#include "pyro/pyro_stream.h"
#include "pyro/pyro_string.h"

namespace vc1 {

class PyroLibrary;
class PyroEmitter;
struct IPyroDevice;
struct PyroResources;
struct PyroFrameTable;

// Inferred names throughout: the statically linked Pyro particle library has
// no symbols. Types are named after what the loader does with them.

PyroArchive& operator>>(PyroArchive& ar, PyroString& text);
char* PyroStrDup(const char* text);

// SIZE 0x08
struct PyroKey8
{
    VC1_PYRO_HEAP
    int v[2];
};

// SIZE 0x0c
struct PyroKey12
{
    VC1_PYRO_HEAP
    int v[3];
};

// Raw texture file bytes stored in the .pyro file.
// SIZE 0x18
struct PyroTextureData
{
    VC1_PYRO_HEAP

    PyroResources* owner;     // 0x00
    int id;                   // 0x04
    void* data;               // 0x08
    int size;                 // 0x0c
    PyroDeviceObject* object; // 0x10
    PyroDeviceImage* image;   // 0x14  device image made by CreateTexture

    PyroTextureData() : owner(0), data(0), size(0), object(0) {}
    ~PyroTextureData()
    {
        if (data) {
            free(data);
        }
        if (object) {
            delete object;
        }
    }

    void Read(PyroArchive& ar);
    void CreateTexture(PyroLibrary* library);
};

// SIZE 0x1c
struct PyroTextureLayer
{
    VC1_PYRO_HEAP

    PyroResources* owner;  // 0x00
    int field04[5];        // 0x04
    PyroTextureData* data; // 0x18  looked up by id in owner->textureData

    PyroTextureLayer() : owner(0), data(0) {}

    void Read(PyroArchive& ar);
};

// SIZE 0x44
struct PyroTexture
{
    VC1_PYRO_HEAP

    PyroResources* owner;     // 0x00
    int fields[14];           // 0x04
    PyroTextureLayer* first;  // 0x3c
    PyroTextureLayer* second; // 0x40

    PyroTexture() : owner(0), first(0), second(0) {}
    ~PyroTexture()
    {
        if (first) {
            delete first;
        }
        if (second) {
            delete second;
        }
    }

    void Read(PyroArchive& ar);
};

// SIZE 0x10
struct PyroFrame
{
    PyroKey8* pairs;    // 0x00
    int pairCount;      // 0x04
    PyroKey12* triples; // 0x08
    int tripleCount;    // 0x0c

    PyroFrame() : pairs(0), pairCount(0), triples(0), tripleCount(0) {}
    ~PyroFrame()
    {
        if (pairs) {
            delete[] pairs;
        }
        if (triples) {
            delete[] triples;
        }
    }

    void Read(PyroArchive& ar);
};

// Device objects released by the frame table destructor through slots 3 and 5.
// SIZE 0x04
struct PyroDeviceBuffer
{
    virtual void Buffer00() = 0;                          // vtable+0x00
    virtual void Buffer04() = 0;                          // vtable+0x04
    virtual void Buffer08() = 0;                          // vtable+0x08
    virtual void* DeletingDestructor(unsigned flags) = 0; // vtable+0x0c
};

// SIZE 0x04
struct PyroDeviceBuffer2
{
    virtual void Buffer00() = 0;                          // vtable+0x00
    virtual void Buffer04() = 0;                          // vtable+0x04
    virtual void Buffer08() = 0;                          // vtable+0x08
    virtual void Buffer0c() = 0;                          // vtable+0x0c
    virtual void Buffer10() = 0;                          // vtable+0x10
    virtual void* DeletingDestructor(unsigned flags) = 0; // vtable+0x14
};

// SIZE 0x818
struct PyroFrameTable
{
    PyroKey12* keysA;           // 0x00
    int keyCountA;              // 0x04
    PyroKey12* keysB;           // 0x08
    int keyCountB;              // 0x0c
    PyroDeviceBuffer* buffer;   // 0x10
    PyroDeviceBuffer2* buffer2; // 0x14
    PyroFrame frames[128];      // 0x18

    PyroFrameTable();
    ~PyroFrameTable();

    void Read(PyroArchive& ar);
};

void PyroCreateFrameBuffers(IPyroDevice* device, PyroFrameTable* frames);

// Base of the typed graphic objects in PyroResources (only type 1 exists).
// SIZE 0x3c
// VTABLE: VC1 0x004d7ce4
class PyroGraphicBase
{
public:
    VC1_PYRO_HEAP

    PyroResources* owner; // 0x04
    int id;               // 0x08
    float matrix[3][4];   // 0x0c

    PyroGraphicBase() : owner(0) {}
    virtual ~PyroGraphicBase() {}                           // vtable+0x00
    virtual int GetType() = 0;                              // vtable+0x04
    virtual void Graphic08() {}                             // vtable+0x08
    virtual void Deserialize(PyroArchive& ar, int version); // vtable+0x0c
};

// SYNTHETIC: VC1 0x0042e8c0
// vc1::PyroGraphicBase::`scalar deleting destructor'

// SIZE 0x858
// VTABLE: VC1 0x004d7cf4
class PyroGraphic : public PyroGraphicBase
{
public:
    PyroFrameTable frames; // 0x3c
    PyroTexture* texture;  // 0x854

    PyroGraphic() : texture(0) {}

    int GetType() { return 1; }
    void Deserialize(PyroArchive& ar, int version);
};

// SYNTHETIC: VC1 0x0042e8e0
// vc1::PyroGraphic::`scalar deleting destructor'

// SIZE 0x18
struct PyroResources
{
    PyroTextureData* textureData; // 0x00
    int textureDataCount;         // 0x04
    PyroTexture* textures;        // 0x08
    int textureCount;             // 0x0c
    PyroGraphicBase** graphics;   // 0x10
    int graphicCount;             // 0x14

    ~PyroResources();

    void Read(PyroArchive& ar, int version);
};

// SIZE 0x14
struct PyroGraphicRef
{
    VC1_PYRO_HEAP

    int value;                // 0x00
    int fields[3];            // 0x04
    PyroGraphicBase* graphic; // 0x10

    PyroGraphicRef() : graphic(0) {}
    ~PyroGraphicRef() {}
};

// SIZE 0x20
struct PyroHeader : public PyroResources
{
    PyroGraphicRef* refs; // 0x18
    int refCount;         // 0x1c

    ~PyroHeader() { delete[] refs; }

    void Read(PyroArchive& ar, int version);
};

// SIZE 0x20
struct PyroShape
{
    VC1_PYRO_HEAP

    int field00;              // 0x00
    PyroDeviceImage* image;   // 0x04  only alive inside CreateTexture
    PyroDeviceObject* object; // 0x08
    PyroLibrary* library;     // 0x0c
    float field10;            // 0x10
    float field14;            // 0x14
    void* data;               // 0x18  frame 0 image bytes
    int dataSize;             // 0x1c

    PyroShape() : image(0), object(0), data(0), dataSize(0) {}
    ~PyroShape()
    {
        if (data) {
            free(data);
            data = 0;
        }
        if (object) {
            delete object;
            object = 0;
        }
    }

    void Read(PyroArchive& ar);
    bool CreateTexture();
};

// SYNTHETIC: VC1 0x0042edb0
// vc1::PyroShape::`vector deleting destructor'
// SYNTHETIC: VC1 0x0042ebd0
// vc1::PyroTextureData::`vector deleting destructor'
// SYNTHETIC: VC1 0x0042ec20
// vc1::PyroTexture::`vector deleting destructor'

// SIZE 0x04
// VTABLE: VC1 0x004d7c7c
class PyroFileBase
{
public:
    VC1_PYRO_HEAP

    virtual void CreateTextures() = 0;                        // vtable+0x00
    virtual void DestroyTextures() = 0;                       // vtable+0x04
    virtual PyroEmitter* CreateEmitter(const char* name) = 0; // vtable+0x08
    virtual int GetEmitterCount() = 0;                        // vtable+0x0c
    virtual PyroEmitter* FindEmitter(const char* name) = 0;   // vtable+0x10
    virtual PyroEmitter* GetEmitter(int index) = 0;           // vtable+0x14
    virtual void File18() = 0;                                // vtable+0x18
    virtual ~PyroFileBase() {}                                // vtable+0x1c
};

// SYNTHETIC: VC1 0x0042e800
// vc1::PyroFileBase::`scalar deleting destructor'

// A loaded .pyro file: shared resources, emitter prototypes and shapes.
// Built by PyroLibrary::LoadFile (0x00456800).
// SIZE 0x38
// VTABLE: VC1 0x004d7d7c
class PyroFile : public PyroFileBase
{
public:
    PyroHeader header;     // 0x04
    PyroLibrary* library;  // 0x24
    int emitterCount;      // 0x28
    PyroEmitter* emitters; // 0x2c
    int shapeCount;        // 0x30
    PyroShape* shapes;     // 0x34

    explicit PyroFile(PyroLibrary* owner)
    {
        header.textureData = 0;
        header.textureDataCount = 0;
        header.textures = 0;
        header.textureCount = 0;
        header.graphics = 0;
        header.graphicCount = 0;
        header.refs = 0;
        header.refCount = 0;
        library = owner;
        emitterCount = 0;
        emitters = 0;
        shapeCount = 0;
        shapes = 0;
    }
    ~PyroFile();

    void Deserialize(PyroArchive& ar);

    void CreateTextures();
    void DestroyTextures();
    PyroEmitter* CreateEmitter(const char* name);
    int GetEmitterCount();
    PyroEmitter* FindEmitter(const char* name);
    PyroEmitter* GetEmitter(int index);
    void File18();
};

// SYNTHETIC: VC1 0x0042ed90
// vc1::PyroFile::`scalar deleting destructor'

typedef char Vc1PyroTextureDataSizeCheck[sizeof(PyroTextureData) == 0x18 ? 1 : -1];
typedef char Vc1PyroTextureSizeCheck[sizeof(PyroTexture) == 0x44 ? 1 : -1];
typedef char Vc1PyroGraphicSizeCheck[sizeof(PyroGraphic) == 0x858 ? 1 : -1];
typedef char Vc1PyroHeaderSizeCheck[sizeof(PyroHeader) == 0x20 ? 1 : -1];
typedef char Vc1PyroShapeSizeCheck[sizeof(PyroShape) == 0x20 ? 1 : -1];
typedef char Vc1PyroFileSizeCheck[sizeof(PyroFile) == 0x38 ? 1 : -1];

}

#endif
