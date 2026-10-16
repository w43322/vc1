#include "pyro/pyro_emitter.h"

#include <string.h>

namespace vc1 {

// GLOBAL: VC1 0x004e99c0
PyroParticlePool g_pyroParticlePool = { 0xec, 0x40, 0, 0 };

// GLOBAL: VC1 0x004d89dc
extern const float g_pyroEmitterConstant = 50.0f;

static void PyroParticleRelease(PyroParticle* particle)
{
    particle->poolNext = static_cast<PyroParticle*>(g_pyroParticlePool.freeList);
    g_pyroParticlePool.freeList = particle;
}

// Returns every live particle of every layer to the pool.
// FUNCTION: VC1 0x0042e910
void PyroEmitter::Reset()
{
    for (int i = 0; i < layerCount; ++i) {
        PyroLayer* const layer = &layers[i];
        PyroParticle* particle = layer->particles;
        layer->active = false;
        while (particle) {
            PyroParticle* const next = particle->next;
            PyroParticleRelease(particle);
            particle = next;
        }
        layer->time = 0.0f;
        layer->particles = 0;
    }
}

// FUNCTION: VC1 0x0042e980
bool PyroEmitter::IsActive()
{
    PyroEmitter* root = this;
    while (root->parent) {
        root = root->parent;
    }
    return root->field1ab;
}

// FUNCTION: VC1 0x0042e9b0
const char* PyroEmitter::GetName()
{
    return name;
}

// RET 0xc.
// FUNCTION: VC1 0x0042e9c0
void PyroEmitter::SetPosition(float x, float y, float z)
{
    position[0] = x;
    position[1] = y;
    position[2] = z;
}

// RET 0xc.
// FUNCTION: VC1 0x0042e9f0
void PyroEmitter::GetPosition(float* x, float* y, float* z)
{
    *x = position[0];
    *y = position[1];
    *z = position[2];
}

// RET 0x10.
// FUNCTION: VC1 0x0042ea20
void PyroEmitter::SetColor(float r, float g, float b, float a)
{
    color[0] = r;
    color[1] = g;
    color[2] = b;
    color[3] = a;
}

// RET 0x10.
// FUNCTION: VC1 0x0042ea50
void PyroEmitter::GetColor(float* r, float* g, float* b, float* a)
{
    *r = color[0];
    *g = color[1];
    *b = color[2];
    *a = color[3];
}

// RET 0xc.
// FUNCTION: VC1 0x0042ea90
void PyroEmitter::SetOffset(float x, float y, float z)
{
    offset.x = x;
    offset.y = y;
    offset.z = z;
}

// RET 0xc.
// FUNCTION: VC1 0x0042eac0
void PyroEmitter::GetOffset(float* x, float* y, float* z)
{
    *x = offset.x;
    *y = offset.y;
    *z = offset.z;
}

// RET 4.
// FUNCTION: VC1 0x0042eaf0
void PyroEmitter::SetScale(float value)
{
    scale = value;
}

// FUNCTION: VC1 0x0042eb00
float PyroEmitter::GetScale()
{
    return scale;
}

// FUNCTION: VC1 0x0042eb10
float PyroEmitter::GetField210()
{
    return field210;
}

// FUNCTION: VC1 0x0042eb20
bool PyroEmitter::HasActiveLayers()
{
    for (int i = 0; i < layerCount; ++i) {
        if (layers[i].flag2) {
            return true;
        }
    }
    return false;
}

// RET 4.
// FUNCTION: VC1 0x0042eb50
void PyroEmitter::GetOffsetVec(float* out)
{
    out[0] = offset.x;
    out[1] = offset.y;
    out[2] = offset.z;
}

// RET 4.
// FUNCTION: VC1 0x0042eb80
void PyroEmitter::SetUserData(void* data)
{
    userData = data;
}

// FUNCTION: VC1 0x0042eb90
void* PyroEmitter::GetUserData()
{
    return userData;
}

// FUNCTION: VC1 0x0042eba0
float PyroEmitter::GetConstant()
{
    return g_pyroEmitterConstant;
}

// RET 8.
// FUNCTION: VC1 0x0042ebb0
void PyroEmitter::SetRange(int a, int b)
{
    rangeA = a;
    rangeB = b;
}

// Pops a particle off the free list, carving a new block of 64 when empty.
// FUNCTION: VC1 0x0042ee10
void* PyroParticleAlloc()
{
    void** element = static_cast<void**>(g_pyroParticlePool.freeList);

    if (!element) {
        void** const block = static_cast<void**>(malloc(g_pyroParticlePool.blockCount * g_pyroParticlePool.elementSize + 4));
        int i = g_pyroParticlePool.blockCount - 1;
        char* cursor;

        *block = g_pyroParticlePool.blocks;
        g_pyroParticlePool.blocks = block;
        cursor = reinterpret_cast<char*>(block) + i * g_pyroParticlePool.elementSize + 4;
        element = static_cast<void**>(g_pyroParticlePool.freeList);
        for (; i >= 0; --i) {
            *reinterpret_cast<void**>(cursor) = element;
            element = reinterpret_cast<void**>(cursor);
            cursor -= g_pyroParticlePool.elementSize;
        }
        g_pyroParticlePool.freeList = element;
    }
    g_pyroParticlePool.freeList = *element;
    return element;
}

// Frees every pool block.
// FUNCTION: VC1 0x0042ee80
void PyroParticleFreeAll()
{
    void** block = static_cast<void**>(g_pyroParticlePool.blocks);
    while (block) {
        void** const next = static_cast<void**>(*block);
        free(block);
        block = next;
    }
    g_pyroParticlePool.freeList = 0;
    g_pyroParticlePool.blocks = 0;
}

// RET 8. Update with the stored move position.
// FUNCTION: VC1 0x0044ead0
void PyroEmitter::Update(float time, float dt)
{
    Move(time, dt, movePosition[0], movePosition[1], movePosition[2], emitOnMove);
}

// FUNCTION: VC1 0x0044eb20
void PyroEmitter::Emitter08()
{
    Emitter04(-1, 0);
}

// RET 4. Copies a 3x4 matrix and records whether it is the identity.
// FUNCTION: VC1 0x0044eb40
void PyroEmitter::SetMatrix(const float* source)
{
    bool identity = true;

    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 4; ++column) {
            const float value = source[row * 4 + column];
            matrix[row][column] = value;
            if (value != (row == column ? 1.0f : 0.0f)) {
                identity = false;
            }
        }
    }
    matrixIdentity = identity;
}

// EAX=this. Body not recovered yet.
// STUB: VC1 0x0044ec40
void PyroLayer::UpdateRotation()
{
}

// EAX=this. Body not recovered yet.
// STUB: VC1 0x0044eca0
int PyroLayer::CountParticles()
{
    return 0;
}

// EAX=this. Instances share the prototype's curve keys and name.
// FUNCTION: VC1 0x0044ef50
void PyroEmitter::MarkInstance()
{
    for (int i = 0; i < 19; ++i) {
        curves[i].shared = true;
    }
    isInstance = true;
}

// RET 4. Copies the rotation part of a 4x4 matrix.
// FUNCTION: VC1 0x0044f2a0
void PyroEmitter::SetRotation(const float* source)
{
    for (int row = 0; row < 3; ++row) {
        rotation[row][0] = source[row * 4 + 0];
        rotation[row][1] = source[row * 4 + 1];
        rotation[row][2] = source[row * 4 + 2];
    }
    for (int i = 0; i < layerCount; ++i) {
        layers[i].UpdateRotation();
    }
}

// FUNCTION: VC1 0x0044f310
bool PyroEmitter::HasParticles()
{
    for (int i = 0; i < layerCount; ++i) {
        if (layers[i].particles) {
            return true;
        }
    }
    return false;
}

// RET 4. Appends to the child vector, doubling its capacity (initially 4).
// FUNCTION: VC1 0x0044f350
void PyroEmitter::AddChild(void* child)
{
    if (isInstance) {
        children = 0;
        childCount = 0;
        childCapacity = 0;
    }
    if (childCount == childCapacity) {
        if (childCapacity == 0) {
            childCapacity = 4;
        }
        childCapacity *= 2;
        children = static_cast<void**>(realloc(children, childCapacity * sizeof(void*)));
    }
    children[childCount] = child;
    ++childCount;
}

// EAX=this. Instances share the prototype's curve keys and key lists.
// FUNCTION: VC1 0x0044f3d0
void PyroLayer::MarkInstance()
{
    graphics.shared = true;
    keyList.shared = true;
    for (int i = 0; i < 38; ++i) {
        curves[i].shared = true;
    }
    active = false;
    isInstance = true;
}

// FUNCTION: VC1 0x0044fd60
PyroEmitter::PyroEmitter()
{
    field168 = 0;
    field16c = 0;
    field170 = 0;
    movePosition[0] = 0.0f;
    movePosition[1] = 0.0f;
    movePosition[2] = 0.0f;
    emitOnMove = true;
    parentLayer = 0;
    name = 0;
    layerCount = 0;
    layers = 0;
    isInstance = false;
    field1a9 = true;
    field1aa = false;
    field1ab = false;
    field1ac = 0;
    scale = 1.0f;
    children = 0;
    childCount = 0;
    childCapacity = 0;
    parent = 0;
    prototype = 0;
    rangeA = 0;
    rangeB = 0;
    userData = 0;
    field210 = 100.0f;
    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 4; ++column) {
            matrix[row][column] = row == column ? 1.0f : 0.0f;
        }
    }
    matrixIdentity = true;
    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 3; ++column) {
            rotation[row][column] = row == column ? 1.0f : 0.0f;
        }
    }
}

// ECX=this.
// FUNCTION: VC1 0x00450550
PyroLayer::PyroLayer()
{
    graphics.choices = 0;
    graphics.count = 0;
    graphics.header = 0;
    graphics.shared = false;
    graphics.total = 0;
    keyList.count = 0;
    keyList.keys = 0;
    keyList.loop = false;
    keyList.shared = false;
    time = 0.0f;
    particles = 0;
    field2b0 = 0;
    shape = 0;
    field2ac = 0;
    library = 0;
    active = false;
    subEmitterCount = 0;
    subEmitters = 0;
    isInstance = false;
    field2b4 = 0;
    field250 = 0;
    field1ec = false;
}

// Sum of every layer's live and pending particles.
// FUNCTION: VC1 0x00450a90
int PyroEmitter::GetParticleCount()
{
    int total = 0;
    for (int i = 0; i < layerCount; ++i) {
        PyroLayer* const layer = &layers[i];
        total += layer->CountParticles() + layer->field250;
    }
    return total;
}

// Stack this, a, b. Body not recovered yet.
// STUB: VC1 0x00450af0
void PyroLayer::Prepare(float a, float b)
{
    (void)a;
    (void)b;
}

// Stack this, a, b. Body not recovered yet.
// STUB: VC1 0x00452140
int PyroLayer::Render(int a, int b)
{
    (void)a;
    (void)b;
    return 0;
}

// ECX=this. Particles go back to the pool after their child emitters are
// deleted.
// FUNCTION: VC1 0x00452620
PyroLayer::~PyroLayer()
{
    PyroParticle* particle = particles;
    while (particle) {
        PyroParticle* const next = particle->next;
        if (particle->children) {
            for (int i = 0; i < particle->childCount; ++i) {
                if (particle->children[i].emitter) {
                    delete particle->children[i].emitter;
                }
            }
            free(particle->children);
            particle->children = 0;
        }
        PyroParticleRelease(particle);
        particle = next;
    }
    if (!isInstance && subEmitters) {
        delete[] subEmitters;
    }
}

// EAX=ar, stack this.
// FUNCTION: VC1 0x00452b00
void PyroKeyList::Read(PyroArchive& ar)
{
    int flag;

    ar.Read(&count, 4);
    keys = new PyroKey20[count];
    for (int i = 0; i < count; ++i) {
        ar.Read(&keys[i].v[0], 4);
        ar.Read(&keys[i].v[1], 4);
        ar.Read(&keys[i].v[2], 4);
        ar.Read(&keys[i].v[3], 4);
        ar.Read(&keys[i].v[4], 4);
    }
    ar.Read(&field08, 4);
    ar.Read(&flag, 4);
    loop = flag != 0;
}

// EAX=ar, stack this and version. Keys gained their interpolation fields in
// version 0x16000.
// FUNCTION: VC1 0x00452bd0
void PyroCurve::Read(PyroArchive& ar, int version)
{
    int flag;

    ar.Read(&count, 4);
    if (count) {
        keys = new PyroCurveKey[count];
        for (int i = 0; i < count; ++i) {
            PyroCurveKey* const key = &keys[i];
            ar.Read(&key->time, 4);
            ar.Read(&key->value, 4);
            if (version >= 0x16000) {
                ar.Read(&key->type, 4);
                ar.Read(&key->c0, 4);
                ar.Read(&key->c2, 4);
                ar.Read(&key->c1, 4);
                ar.Read(&key->c3, 4);
            }
            else {
                key->type = 0;
                key->c0 = 0.0f;
                key->c2 = 0.0f;
                key->c1 = 0.0f;
                key->c3 = 0.0f;
            }
        }
    }
    ar.Read(&flag, 4);
    loop = flag != 0;
}

// ESI=this, EDI=ar.
// FUNCTION: VC1 0x00452cf0
void PyroGraphicChoices::Read(PyroArchive& ar)
{
    ar.Read(&count, 4);
    if (count) {
        choices = new PyroGraphicChoice[count];
        for (int i = 0; i < count; ++i) {
            int refValue;
            int weight;
            float value;
            int flag;
            bool set;
            PyroGraphicRef* found;
            PyroGraphicChoice* choice;

            ar.Read(&refValue, 4);
            ar.Read(&weight, 4);
            total += weight;
            ar.Read(&value, 4);
            ar.Read(&flag, 4);
            set = flag != 0;
            found = 0;
            for (int j = 0; j < header->refCount; ++j) {
                if (header->refs[j].value == refValue) {
                    found = &header->refs[j];
                    break;
                }
            }
            choice = &choices[i];
            choice->ref = found;
            choice->weight = weight;
            choice->value = value;
            choice->flag = set;
        }
    }
}

// Stack this and prototype. A byte copy of the prototype that shares its
// curve keys; layers are copied the same way.
// FUNCTION: VC1 0x004533b0
PyroEmitter::PyroEmitter(PyroEmitter* source)
{
    memcpy(this, source, sizeof(PyroEmitter));
    name = 0;
    prototype = source;
    MarkInstance();
    if (layerCount) {
        layers = new PyroLayer[layerCount];
        for (int i = 0; i < layerCount; ++i) {
            memcpy(&layers[i], &source->layers[i], sizeof(PyroLayer));
            layers[i].emitter = this;
            layers[i].MarkInstance();
        }
    }
    field1aa = false;
    field1a9 = true;
    emitOnMove = true;
}

// RET 8.
// FUNCTION: VC1 0x004536b0
void PyroEmitter::Prepare(float a, float b)
{
    for (int i = 0; i < layerCount; ++i) {
        layers[i].Prepare(a, b);
    }
}

// RET 8.
// FUNCTION: VC1 0x00453700
int PyroEmitter::Emitter04(int a, int b)
{
    int total = 0;
    for (int i = 0; i < layerCount; ++i) {
        total += layers[i].Render(a, b);
    }
    return total;
}

// FUNCTION: VC1 0x00453980
PyroEmitter::~PyroEmitter()
{
    if (layers) {
        delete[] layers;
    }
    if (children && (!isInstance || prototype->children != children)) {
        free(children);
    }
    if (!isInstance && name) {
        free(name);
    }
}

// EBX=this, five floats and a flag on the stack. Body not recovered yet.
// STUB: VC1 0x00454f70
void PyroEmitter::MoveTo(float time, float dt, float x, float y, float z, bool emit)
{
    (void)time;
    (void)dt;
    (void)x;
    (void)y;
    (void)z;
    (void)emit;
}

// RET 0x18.
// FUNCTION: VC1 0x00455350
void PyroEmitter::Move(float time, float dt, float x, float y, float z, bool emit)
{
    MoveTo(time, dt, x, y, z, emit);
}

// EAX=ar, EBX=version, stack this.
// FUNCTION: VC1 0x00455390
void PyroLayer::Read(PyroArchive& ar, int version)
{
    PyroString name;
    int value;

    ar.Read(&value, 4);
    if (value) {
        PyroShape* found;

        ar.Read(&value, 4);
        found = 0;
        for (int i = 0; i < file->shapeCount; ++i) {
            if (file->shapes[i].field00 == value) {
                found = &file->shapes[i];
                break;
            }
        }
        shape = found;
    }
    name.data = g_pyroStringEmptyData;
    ar >> name;
    ar.Read(&value, 4);
    field0c = value != 0;
    ar.Read(&field04, 4);
    ar.Read(&field08, 4);
    ar.Read(&value, 4);
    field0d = value != 0;
    ar.Read(&value, 4);
    field0e = value != 0;
    ar.Read(&value, 4);
    flag1 = (value & 1) != 0;
    flag4 = (value & 4) != 0;
    flag2 = (value & 2) != 0;
    keyList.Read(ar);
    curves[0].Read(ar, version);   // 0x20
    curves[7].Read(ar, version);   // 0x74
    curves[4].Read(ar, version);   // 0x50
    curves[6].Read(ar, version);   // 0x68
    curves[1].Read(ar, version);   // 0x2c
    curves[2].Read(ar, version);   // 0x38
    curves[9].Read(ar, version);   // 0x8c
    curves[15].Read(ar, version);  // 0xd4
    curves[16].Read(ar, version);  // 0xe0
    curves[17].Read(ar, version);  // 0xec
    curves[21].Read(ar, version);  // 0x11c
    curves[18].Read(ar, version);  // 0xf8
    curves[27].Read(ar, version);  // 0x164
    curves[31].Read(ar, version);  // 0x194
    curves[28].Read(ar, version);  // 0x170
    curves[29].Read(ar, version);  // 0x17c
    curves[8].Read(ar, version);   // 0x80
    curves[33].Read(ar, version);  // 0x1ac
    curves[24].Read(ar, version);  // 0x140
    curves[10].Read(ar, version);  // 0x98
    curves[20].Read(ar, version);  // 0x110
    curves[32].Read(ar, version);  // 0x1a0
    curves[35].Read(ar, version);  // 0x1c4
    ar.Read(&field18, 4);
    ar.Read(&value, 4);
    field13 = value != 0;
    curves[3].Read(ar, version);   // 0x44
    curves[19].Read(ar, version);  // 0x104
    curves[34].Read(ar, version);  // 0x1b8
    ar.Read(&subEmitterCount, 4);
    if (subEmitterCount) {
        subEmitters = new PyroEmitter[subEmitterCount];
        for (int i = 0; i < subEmitterCount; ++i) {
            subEmitters[i].library = library;
            subEmitters[i].parentLayer = this;
            subEmitters[i].file = file;
            subEmitters[i].Deserialize(ar, version);
        }
    }
    ar.Read(&field1e8, 4);
    ar.Read(&field238, 4);
    ar.Read(&field23c, 4);
    ar.Read(&field240, 4);
    curves[11].Read(ar, version);  // 0xa4
    curves[12].Read(ar, version);  // 0xb0
    curves[13].Read(ar, version);  // 0xbc
    curves[14].Read(ar, version);  // 0xc8
    curves[22].Read(ar, version);  // 0x128
    curves[23].Read(ar, version);  // 0x134
    curves[36].Read(ar, version);  // 0x1d0
    curves[37].Read(ar, version);  // 0x1dc
    curves[25].Read(ar, version);  // 0x14c
    graphics.Read(ar);
    ar.Read(&value, 4);
    field14 = value != 0;
    ar.Read(&field24c, 4);
    curves[5].Read(ar, version);   // 0x5c
    curves[26].Read(ar, version);  // 0x158
    curves[30].Read(ar, version);  // 0x188
    ar.Read(&field2ac, 4);
    ar.Read(&field2b0, 4);
    name.Release();
}

// RET 8 (ar, version).
// FUNCTION: VC1 0x00455bc0
void PyroEmitter::Deserialize(PyroArchive& ar, int version)
{
    PyroString text;
    int value;

    curves[0].Read(ar, version);   // 0x04
    curves[10].Read(ar, version);  // 0x7c
    curves[1].Read(ar, version);   // 0x10
    curves[3].Read(ar, version);   // 0x28
    curves[5].Read(ar, version);   // 0x40
    curves[2].Read(ar, version);   // 0x1c
    curves[11].Read(ar, version);  // 0x88
    curves[12].Read(ar, version);  // 0x94
    curves[13].Read(ar, version);  // 0xa0
    curves[6].Read(ar, version);   // 0x4c
    curves[8].Read(ar, version);   // 0x64
    curves[14].Read(ar, version);  // 0xac
    curves[15].Read(ar, version);  // 0xb8
    curves[9].Read(ar, version);   // 0x70
    curves[16].Read(ar, version);  // 0xc4
    ar.Read(&position[0], 4);
    ar.Read(&position[1], 4);
    position[2] = 0.0f;
    curves[7].Read(ar, version);   // 0x58
    ar.Read(&layerCount, 4);
    if (layerCount) {
        layers = new PyroLayer[layerCount];
        for (int i = 0; i < layerCount; ++i) {
            layers[i].library = library;
            layers[i].file = file;
            layers[i].graphics.header = &file->header;
            layers[i].emitter = this;
            layers[i].Read(ar, version);
        }
    }
    text.data = g_pyroStringEmptyData;
    ar >> text;
    if (text.Header() != &g_pyroStringEmpty.header && text.data) {
        name = PyroStrDup(text.data);
    }
    else {
        name = 0;
    }
    ar.Read(&color[0], 4);
    ar.Read(&color[1], 4);
    ar.Read(&color[2], 4);
    ar.Read(&color[3], 4);
    ar.Read(&value, 4);
    field196 = value != 0;
    ar.Read(&field198, 4);
    ar.Read(&value, 4);
    field194 = value != 0;
    ar.Read(&value, 4);
    field195 = value != 0;
    ar.Read(&field19c, 4);
    ar.Read(&value, 4);
    field1ab = value != 0;
    curves[17].Read(ar, version);  // 0xd0
    curves[18].Read(ar, version);  // 0xdc
    curves[4].Read(ar, version);   // 0x34
    if (version >= 0x16000) {
        ar.Read(&field1ac, 4);
    }
    if (version >= 0x24000) {
        ar.Read(&field210, 4);
    }
    text.Release();
}

}
