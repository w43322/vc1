#include "pyro/pyro_file.h"

#include <errno.h>
#include <string.h>

#include "host/runtime.h"
#include "kd/display/pyro.h"
#include "kd/iface/ipyro_device.h"
#include "pyro/pyro_emitter.h"
#include "pyro/pyro_image.h"

namespace vc1 {

// EDI=text. malloc'd copy with a bounds-checked copy (errno 0x11 on failure).
// FUNCTION: VC1 0x0042e680
char* PyroStrDup(const char* text)
{
    const size_t size = strlen(text) + 1;
    char* const copy = static_cast<char*>(malloc(size));
    if (size != 0) {
        const size_t length = strlen(text) + 1;
        if (length <= size) {
            memcpy(copy, text, length);
            return copy;
        }
        *copy = 0;
    }
    errno = 0x11;
    return copy;
}

// EAX=text, stack ar; returns ar. MFC CArchive string format: a byte length,
// 0xff escapes to a word, a word of 0xfffe or more escapes to a dword.
// FUNCTION: VC1 0x0044e310
PyroArchive& operator>>(PyroArchive& ar, PyroString& text)
{
    unsigned char length8;
    unsigned short length16;
    unsigned length32;
    int length;
    char* buffer;

    ar.stream->Read(&length8, 1);
    if (length8 < 0xff) {
        length = length8;
    }
    else {
        ar.stream->Read(&length16, 2);
        if (length16 < 0xfffe) {
            length = length16;
        }
        else {
            ar.stream->Read(&length32, 4);
            length = static_cast<int>(length32);
        }
    }
    if (length == 0) {
        text.Assign(reinterpret_cast<const char*>(&g_kdLogTag));
        return ar;
    }
    text.AllocBeforeWrite(length);
    buffer = text.Header() != &g_pyroStringEmpty.header ? text.data : 0;
    ar.stream->Read(buffer, length);
    buffer[length] = 0;
    text.Header()->length = length;
    return ar;
}

// RET 4.
// FUNCTION: VC1 0x0044e990
PyroEmitter* PyroFile::GetEmitter(int index)
{
    if (index >= 0 && index < emitterCount) {
        return &emitters[index];
    }
    return 0;
}

// FUNCTION: VC1 0x0044e9b0
int PyroFile::GetEmitterCount()
{
    return emitterCount;
}

// RET 4. Compares through the emitter's GetName slot (vtable+0x1c).
// FUNCTION: VC1 0x0044efb0
PyroEmitter* PyroFile::FindEmitter(const char* name)
{
    for (int i = 0; i < emitterCount; ++i) {
        if (strcmp(emitters[i].GetName(), name) == 0) {
            return &emitters[i];
        }
    }
    return 0;
}

// Deletes each shape's device object.
// FUNCTION: VC1 0x0044f030
void PyroFile::DestroyTextures()
{
    for (int i = 0; i < shapeCount; ++i) {
        PyroShape* const shape = &shapes[i];
        if (shape->object) {
            delete shape->object;
            shape->object = 0;
        }
    }
}

// Calls slot 1 of each shape's device object.
// FUNCTION: VC1 0x0044f070
void PyroFile::RestoreTextures()
{
    for (int i = 0; i < shapeCount; ++i) {
        shapes[i].object->Restore();
    }
}

// EDI=this, EAX=ar. The texture data is looked up by id.
// FUNCTION: VC1 0x0044f9c0
void PyroTextureLayer::Read(PyroArchive& ar)
{
    int dataId;
    PyroTextureData* found;

    ar.Read(&field04[0], 4);
    ar.Read(&field04[1], 4);
    ar.Read(&field04[2], 4);
    ar.Read(&field04[3], 4);
    ar.Read(&field04[4], 4);
    ar.Read(&dataId, 4);
    found = 0;
    for (int i = 0; i < owner->textureDataCount; ++i) {
        if (owner->textureData[i].id == dataId) {
            found = &owner->textureData[i];
            break;
        }
    }
    data = found;
}

// EAX=this. The frames are cleared before the key arrays.
// FUNCTION: VC1 0x00452e00
PyroFrameTable::PyroFrameTable()
{
    keysA = 0;
    keyCountA = 0;
    keysB = 0;
    keyCountB = 0;
    buffer = 0;
    buffer2 = 0;
}

// EAX=this.
// FUNCTION: VC1 0x00452e40
PyroFrameTable::~PyroFrameTable()
{
    if (keysA) {
        delete[] keysA;
    }
    if (keysB) {
        delete[] keysB;
    }
    if (buffer) {
        delete buffer;
        buffer = 0;
    }
    if (buffer2) {
        delete buffer2;
        buffer2 = 0;
    }
}

// ECX=device, ESI=frames. Uploads the graphic's mesh as a white triangle list:
// keysA are the positions, keysB the position index triples, frame 0 holds the
// UVs (pairs) and their index triples. The dwords are copied unconverted.
// FUNCTION: VC1 0x00452ed0
void PyroCreateFrameBuffers(IPyroDevice* device, PyroFrameTable* frames)
{
    device->CreateVertexBuffer(&frames->buffer, frames->keyCountB * 3, 0x18, 0x10009, 0);
    const int vertexCount = frames->keyCountB * 3;
    PyroVertex* const vertices = new PyroVertex[vertexCount];
    PyroVertex* out = vertices;
    for (int i = 0; i < frames->keyCountB; ++i) {
        const int* const corners = frames->keysB[i].v;
        const int* const uvs = frames->frames[0].triples[i].v;
        out[0].x = reinterpret_cast<const float*>(frames->keysA[corners[0]].v)[0];
        out[0].y = reinterpret_cast<const float*>(frames->keysA[corners[0]].v)[1];
        out[0].z = reinterpret_cast<const float*>(frames->keysA[corners[0]].v)[2];
        out[0].u = reinterpret_cast<const float*>(frames->frames[0].pairs[uvs[0]].v)[0];
        out[0].color = 0xffffffff;
        out[0].v = reinterpret_cast<const float*>(frames->frames[0].pairs[uvs[0]].v)[1];
        out[1].x = reinterpret_cast<const float*>(frames->keysA[corners[1]].v)[0];
        out[1].y = reinterpret_cast<const float*>(frames->keysA[corners[1]].v)[1];
        out[1].z = reinterpret_cast<const float*>(frames->keysA[corners[1]].v)[2];
        out[1].u = reinterpret_cast<const float*>(frames->frames[0].pairs[uvs[1]].v)[0];
        out[1].color = 0xffffffff;
        out[1].v = reinterpret_cast<const float*>(frames->frames[0].pairs[uvs[1]].v)[1];
        out[2].x = reinterpret_cast<const float*>(frames->keysA[corners[2]].v)[0];
        out[2].y = reinterpret_cast<const float*>(frames->keysA[corners[2]].v)[1];
        out[2].z = reinterpret_cast<const float*>(frames->keysA[corners[2]].v)[2];
        out[2].u = reinterpret_cast<const float*>(frames->frames[0].pairs[uvs[2]].v)[0];
        out[2].color = 0xffffffff;
        out[2].v = reinterpret_cast<const float*>(frames->frames[0].pairs[uvs[2]].v)[1];
        out += 3;
    }
    frames->buffer->Upload(vertices, 0, vertexCount * 0x18, 1);
    delete[] vertices;
}

// ESI=this, EDI=ar.
// FUNCTION: VC1 0x00453080
void PyroFrame::Read(PyroArchive& ar)
{
    ar.Read(&pairCount, 4);
    if (pairCount) {
        pairs = new PyroKey8[pairCount];
        for (int i = 0; i < pairCount; ++i) {
            ar.Read(&pairs[i].v[0], 4);
            ar.Read(&pairs[i].v[1], 4);
        }
    }
    ar.Read(&tripleCount, 4);
    if (tripleCount) {
        triples = new PyroKey12[tripleCount];
        for (int i = 0; i < tripleCount; ++i) {
            ar.Read(&triples[i].v[0], 4);
            ar.Read(&triples[i].v[1], 4);
            ar.Read(&triples[i].v[2], 4);
        }
    }
}

// EBX=this, ESI=ar. The second layer is gated on the (still null) member,
// not on the flag just read, so the original never loads it.
// FUNCTION: VC1 0x00453180
void PyroTexture::Read(PyroArchive& ar)
{
    int present;

    for (int i = 0; i < 14; ++i) {
        ar.Read(&fields[i], 4);
    }
    ar.Read(&present, 4);
    if (present) {
        PyroResources* const resources = owner;
        PyroTextureLayer* const layer = new PyroTextureLayer;
        layer->owner = resources;
        first = layer;
        layer->Read(ar);
    }
    ar.Read(&present, 4);
    if (second) {
        PyroResources* const resources = owner;
        PyroTextureLayer* const layer = new PyroTextureLayer;
        layer->owner = resources;
        second = layer;
        layer->Read(ar);
    }
}

// ESI=this.
// FUNCTION: VC1 0x004532d0
PyroResources::~PyroResources()
{
    if (graphics) {
        for (int i = 0; i < graphicCount; ++i) {
            if (graphics[i]) {
                delete graphics[i];
            }
        }
        free(graphics);
    }
    if (textureData) {
        delete[] textureData;
    }
    if (textures) {
        delete[] textures;
    }
}

// FUNCTION: VC1 0x00453320
PyroFile::~PyroFile()
{
    if (shapes) {
        delete[] shapes;
    }
    if (emitters) {
        delete[] emitters;
    }
}

// RET 4. Instantiates the named prototype.
// FUNCTION: VC1 0x00453630
PyroEmitter* PyroFile::CreateEmitter(const char* name)
{
    PyroEmitter* const prototype = FindEmitter(name);
    return new PyroEmitter(prototype);
}

// ECX=this, EAX=ar.
// FUNCTION: VC1 0x00453840
void PyroFrameTable::Read(PyroArchive& ar)
{
    int frameCount;

    ar.Read(&keyCountA, 4);
    if (keyCountA) {
        keysA = new PyroKey12[keyCountA];
        for (int i = 0; i < keyCountA; ++i) {
            ar.Read(&keysA[i].v[0], 4);
            ar.Read(&keysA[i].v[1], 4);
            ar.Read(&keysA[i].v[2], 4);
        }
    }
    ar.Read(&keyCountB, 4);
    if (keyCountB) {
        keysB = new PyroKey12[keyCountB];
        for (int i = 0; i < keyCountB; ++i) {
            ar.Read(&keysB[i].v[0], 4);
            ar.Read(&keysB[i].v[1], 4);
            ar.Read(&keysB[i].v[2], 4);
        }
    }
    ar.Read(&frameCount, 4);
    for (int i = 0; i < frameCount; ++i) {
        frames[i].Read(ar);
    }
}

// EBX=this, ESI=ar. Frame count, name (dropped), id, then per frame its
// image bytes and two floats; only frame 0 is kept, later images are
// skipped. Two trailing dwords and two bytes are read and dropped.
// FUNCTION: VC1 0x00454e40
void PyroShape::Read(PyroArchive& ar)
{
    PyroString name;
    int count;
    int size;
    float width;
    float height;
    int unused[2];
    unsigned char flag;

    ar.Read(&count, 4);
    name.data = g_pyroStringEmptyData;
    ar >> name;
    ar.Read(&field00, 4);
    for (int i = 0; i < count; ++i) {
        ar.Read(&size, 4);
        if (size) {
            if (i > 0) {
                ar.stream->Skip(size);
            }
            else {
                data = malloc(size);
                ar.Read(data, size);
            }
        }
        ar.Read(&width, 4);
        ar.Read(&height, 4);
        if (i == 0) {
            field10 = width;
            dataSize = size;
            field14 = height;
        }
    }
    ar.Read(&unused[0], 4);
    ar.Read(&unused[1], 4);
    ar.Read(&flag, 1);
    ar.Read(&flag, 1);
    name.Release();
}

// RET 8. The name is read and dropped.
// FUNCTION: VC1 0x004558d0
void PyroGraphicBase::Deserialize(PyroArchive& ar, int version)
{
    PyroString name;

    (void)version;
    ar.Read(&id, 4);
    name.data = g_pyroStringEmptyData;
    ar >> name;
    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 4; ++column) {
            ar.Read(&matrix[row][column], 4);
        }
    }
    name.Release();
}

// RET 8. The texture is looked up by id.
// FUNCTION: VC1 0x00455960
void PyroGraphic::Deserialize(PyroArchive& ar, int version)
{
    int textureId;

    PyroGraphicBase::Deserialize(ar, version);
    frames.Read(ar);
    ar.Read(&textureId, 4);
    if (textureId) {
        PyroTexture* found = 0;
        for (int i = 0; i < owner->textureCount; ++i) {
            if (owner->textures[i].fields[0] == textureId) {
                found = &owner->textures[i];
                break;
            }
        }
        texture = found;
    }
}

// EBX=this, EAX=ar. The name is read and dropped.
// FUNCTION: VC1 0x004559e0
void PyroTextureData::Read(PyroArchive& ar)
{
    PyroString name;

    ar.Read(&id, 4);
    name.data = g_pyroStringEmptyData;
    ar >> name;
    ar.Read(&size, 4);
    if (size) {
        data = malloc(size);
        ar.Read(data, size);
    }
    name.Release();
}

// Stack shape, RET 4. Decodes frame 0, has the device wrap it in an image
// (adopting the pixels) and build the texture, then frees the image.
// FUNCTION: VC1 0x00455a60
bool PyroShape::CreateTexture()
{
    if (object) {
        return true;
    }
    PyroMemoryStream memory(data, dataSize);
    PyroImageStreamReader reader(&memory);
    PyroImage decoded;
    if (!PyroDecodeImage(&reader, &decoded)) {
        return false;
    }
    library->device->CreateImage(&image, decoded.pixels, decoded.width, decoded.height, 0);
    library->device->CreateImageTexture(&object, image, 0);
    decoded.pixels = 0;
    if (image && image->pixels) {
        free(image->pixels);
    }
    if (image) {
        delete image;
        image = 0;
    }
    return true;
}

// EDI=this, stack ar and version. Graphic type 1 is the only known type; any
// other type leaves a null object that the original then dereferences.
// FUNCTION: VC1 0x00455f80
void PyroResources::Read(PyroArchive& ar, int version)
{
    ar.Read(&textureDataCount, 4);
    if (textureDataCount) {
        textureData = new PyroTextureData[textureDataCount];
        for (int i = 0; i < textureDataCount; ++i) {
            textureData[i].owner = this;
            textureData[i].Read(ar);
        }
    }
    ar.Read(&textureCount, 4);
    if (textureCount) {
        textures = new PyroTexture[textureCount];
        for (int i = 0; i < textureCount; ++i) {
            textures[i].owner = this;
            textures[i].Read(ar);
        }
    }
    ar.Read(&graphicCount, 4);
    if (graphicCount) {
        graphics = static_cast<PyroGraphicBase**>(malloc(graphicCount * sizeof(PyroGraphicBase*)));
        for (int i = 0; i < graphicCount; ++i) {
            int type;
            PyroGraphicBase* graphic;

            ar.Read(&type, 4);
            graphic = 0;
            if (type == 1) {
                graphic = new PyroGraphic;
            }
            graphic->owner = this;
            graphics[i] = graphic;
            graphic->Deserialize(ar, version);
        }
    }
}

// RET 8 (texture data, library). Only the device image is made (it adopts
// the decoded pixels); the decode result is not checked.
// FUNCTION: VC1 0x004561b0
void PyroTextureData::CreateTexture(PyroLibrary* library)
{
    if (object) {
        return;
    }
    PyroMemoryStream memory(data, size);
    PyroImageStreamReader reader(&memory);
    PyroImage decoded;
    PyroDecodeImage(&reader, &decoded);
    library->device->CreateImage(&image, decoded.pixels, decoded.width, decoded.height, 0);
    decoded.pixels = 0;
    object = 0;
}

// ECX=this, EAX=version, stack ar.
// FUNCTION: VC1 0x00456270
void PyroHeader::Read(PyroArchive& ar, int version)
{
    PyroResources::Read(ar, version);
    ar.Read(&refCount, 4);
    if (refCount) {
        refs = new PyroGraphicRef[refCount];
        for (int i = 0; i < refCount; ++i) {
            int value;
            int graphicId;
            PyroGraphicBase* found;
            PyroGraphicRef* ref;

            ar.Read(&value, 4);
            ar.Read(&graphicId, 4);
            found = 0;
            for (int j = 0; j < graphicCount; ++j) {
                if (graphics[j]->id == graphicId) {
                    found = graphics[j];
                    break;
                }
            }
            ref = &refs[i];
            ref->graphic = found;
            ref->value = value;
            ar.Read(&ref->scale[0], 4);
            ar.Read(&ref->scale[1], 4);
            ar.Read(&ref->scale[2], 4);
        }
    }
}

// EAX=this, stack ar. Magic and version, the header, shapes, emitter
// prototypes, then device buffers for every referenced graphic.
// FUNCTION: VC1 0x004563e0
void PyroFile::Deserialize(PyroArchive& ar)
{
    int magic;
    int version;
    IPyroDevice* device;

    ar.Read(&magic, 4);
    ar.Read(&version, 4);
    header.Read(ar, version);

    ar.Read(&shapeCount, 4);
    if (shapeCount) {
        shapes = new PyroShape[shapeCount];
        for (int i = 0; i < shapeCount; ++i) {
            shapes[i].library = library;
            shapes[i].Read(ar);
        }
    }

    ar.Read(&emitterCount, 4);
    emitters = new PyroEmitter[emitterCount];
    for (int i = 0; i < emitterCount; ++i) {
        emitters[i].library = library;
        emitters[i].file = this;
        emitters[i].Deserialize(ar, version);
    }

    device = library->device;
    for (int i = 0; i < header.refCount; ++i) {
        if (header.refs[i].graphic) {
            PyroCreateFrameBuffers(device, &static_cast<PyroGraphic*>(header.refs[i].graphic)->frames);
        }
    }

    if (version >= 0x16000) {
        int unused;
        ar.Read(&unused, 4);
    }
    if (version >= 0x23000) {
        int unusedA[4];
        int unusedB[4];
        int unusedC[4];
        for (int i = 0; i < 4; ++i) {
            ar.Read(&unusedA[i], 4);
        }
        for (int i = 0; i < 4; ++i) {
            ar.Read(&unusedB[i], 4);
        }
        for (int i = 0; i < 4; ++i) {
            ar.Read(&unusedC[i], 4);
        }
    }
}

// Shapes first, then the texture blobs.
// FUNCTION: VC1 0x00456650
void PyroFile::CreateTextures()
{
    for (int i = 0; i < shapeCount; ++i) {
        shapes[i].CreateTexture();
    }
    PyroLibrary* const owner = library;
    for (int i = 0; i < header.textureDataCount; ++i) {
        header.textureData[i].CreateTexture(owner);
    }
}

}
