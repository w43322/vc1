#ifndef VC1_PYRO_EMITTER_H
#define VC1_PYRO_EMITTER_H

#include "pyro/pyro_file.h"

namespace vc1 {

// Inferred names throughout (Pyro particle library, no symbols).

// SIZE 0x1c
struct PyroCurveKey
{
    VC1_PYRO_HEAP

    float time;   // 0x00
    float value;  // 0x04
    int type;     // 0x08  0x16000+ files
    float c0;     // 0x0c
    float c1;     // 0x10
    float c2;     // 0x14
    float c3;     // 0x18
};

// Keyframed parameter. Instances share the prototype's keys (shared = true).
// SIZE 0x0c
struct PyroCurve
{
    int count;          // 0x00
    PyroCurveKey* keys; // 0x04
    bool smooth;        // 0x08  smoothstep between keys
    bool shared;        // 0x09

    PyroCurve() : count(0), keys(0), shared(false), smooth(false) {}
    ~PyroCurve()
    {
        if (!shared && keys) {
            delete[] keys;
        }
    }

    void Read(PyroArchive& ar, int version);
    float Evaluate(float t);

    // Every caller tests the single-key case before calling Evaluate.
    float Get(float t)
    {
        return count == 1 ? keys->value : Evaluate(t);
    }
};

// SIZE 0x14
struct PyroColorKey
{
    VC1_PYRO_HEAP

    float time; // 0x00
    float r;    // 0x04
    float g;    // 0x08
    float b;    // 0x0c
    float a;    // 0x10
};

// SIZE 0x10
struct PyroKeyList
{
    int count;          // 0x00
    PyroColorKey* keys; // 0x04
    int repeat;         // 0x08  key range repeats this often over the life
    bool random;        // 0x0c  one random sample per particle
    bool shared;        // 0x0d

    ~PyroKeyList()
    {
        if (!shared && keys) {
            delete[] keys;
        }
    }

    void Read(PyroArchive& ar);
    void Sample(float t, float* r, float* g, float* b, float* a);
};

// SIZE 0x10
struct PyroGraphicChoice
{
    VC1_PYRO_HEAP

    PyroGraphicRef* ref; // 0x00
    int weight;          // 0x04
    float value;         // 0x08
    bool flag;           // 0x0c
};

// Weighted list of graphic references; total is the sum of the weights.
// SIZE 0x14
struct PyroGraphicChoices
{
    int total;                  // 0x00
    PyroGraphicChoice* choices; // 0x04
    int count;                  // 0x08
    PyroHeader* header;         // 0x0c
    bool shared;                // 0x10

    ~PyroGraphicChoices()
    {
        if (!shared && choices) {
            delete[] choices;
        }
    }

    void Read(PyroArchive& ar);
    PyroGraphicRef* Pick(float* axis, float* angle, float* scale);
};

// Pool element; the first dword links free elements. Field meanings past the
// motion state are inferred from the emitter that fills them.
// SIZE 0xec
struct PyroParticle
{
    union {
        PyroParticle* poolNext;
        float position[3];   // 0x00
    };
    float offset[3];         // 0x0c  position - emitter position at spawn
    float origin[3];         // 0x18  emitter position at spawn
    float velocity[3];       // 0x24
    float phase;             // 0x30  random, (rand() & 0x7fff) * 0.1432
    float field34;           // 0x34
    float spawnAngle;        // 0x38
    float cur3c;             // 0x3c  start * over-life curve, per update
    float cur40;             // 0x40
    float cur44;             // 0x44
    float cur48;             // 0x48
    float base4c;            // 0x4c  emitter value 12 at spawn
    float cur50;             // 0x50
    float cur54;             // 0x54
    float cur58;             // 0x58
    float cur5c;             // 0x5c
    float cur60;             // 0x60
    float start64;           // 0x64
    float birth;             // 0x68  emitter time at spawn
    float start6c;           // 0x6c
    float start70;           // 0x70
    float start74;           // 0x74
    float start78;           // 0x78
    float start7c;           // 0x7c
    float gravity;           // 0x80
    float start84;           // 0x84
    float start88;           // 0x88
    float start8c;           // 0x8c
    float start90;           // 0x90
    float lifetime;          // 0x94
    float field98;           // 0x98
    float keyColor[4];       // 0x9c
    float color[4];          // 0xac  emitter colour at spawn
    float fieldbc;           // 0xbc  emitter value 10 at spawn
    PyroGraphicRef* graphic; // 0xc0
    float axis[3];           // 0xc4
    float angle;             // 0xd0
    float scale;             // 0xd4
    bool alive;              // 0xd8
    bool keyed;              // 0xd9
    int childCount;          // 0xdc
    struct Child
    {
        float time;          // emitter time when the child was made
        PyroEmitter* emitter;
    }* children;             // 0xe0
    PyroParticle* prev;      // 0xe4
    PyroParticle* next;      // 0xe8
};

// Fixed-size block allocator for particles (0xec bytes, 64 per block).
// SIZE 0x10
struct PyroParticlePool
{
    int elementSize; // 0x00
    int blockCount;  // 0x04
    void* blocks;    // 0x08
    void* freeList;  // 0x0c
};

extern PyroParticlePool g_pyroParticlePool;

void* PyroParticleAlloc();
void PyroParticleFreeAll();

// SIZE 0x0c
struct PyroVec3
{
    float x;
    float y;
    float z;

    PyroVec3() : x(0.0f), y(0.0f), z(0.0f) {}
};

// One particle layer of an emitter.
// SIZE 0x2b8
struct PyroLayer
{
    VC1_PYRO_HEAP

    bool isInstance;            // 0x000
    int field04;                // 0x004
    int field08;                // 0x008
    bool field0c;               // 0x00c
    bool field0d;               // 0x00d
    bool field0e;               // 0x00e
    bool flag1;                 // 0x00f
    bool flag2;                 // 0x010
    bool flag4;                 // 0x011
    bool active;                // 0x012
    bool field13;               // 0x013
    bool field14;               // 0x014
    int field18;                // 0x018
    float time;                 // 0x01c  fractional emission carried over
    PyroCurve curves[38];       // 0x020
    int matrixMode;             // 0x1e8  1: root rotation, 2: root orientation
    bool hasAngles;             // 0x1ec
    float rotation[3][3];       // 0x1f0  from angles
    float matrix[3][3];         // 0x214
    float angles[3];            // 0x238
    int subEmitterCount;        // 0x244
    PyroEmitter* subEmitters;   // 0x248
    int burstCount;             // 0x24c
    int particleCount;          // 0x250
    PyroParticle* particles;    // 0x254
    PyroEmitter* emitter;       // 0x258
    PyroGraphicChoices graphics; // 0x25c
    PyroKeyList keyList;        // 0x270
    PyroShape* shape;           // 0x280
    PyroLibrary* library;       // 0x284
    PyroFile* file;             // 0x288
    int field28c[8];            // 0x28c
    int field2ac;               // 0x2ac
    int field2b0;               // 0x2b0
    int field2b4;               // 0x2b4

    PyroLayer();
    ~PyroLayer();

    void MarkInstance();
    void Read(PyroArchive& ar, int version);

    void SetChildRotation(const float* source);
    int CountParticles();
    void UpdateAngles();
    void Prepare(float a, float b);
    void FreeParticle(PyroParticle* particle);
    void UpdateMatrix();
    int Render(int a, int b);
    PyroParticle* NewParticle();
    void Update(float time, float dt);
    void Emit(float time, float dt, bool emit);
};

float PyroVary(float value, float variation);
float PyroVarySigned(float value, float variation);

// Objects attached with PyroEmitter::AddChild. Slot 0 clips one particle
// step: position/step/velocity and hitTime are updated in place on a hit.
// SIZE 0x04
struct IPyroCollider
{
    virtual int Collide(float time, float* position, float* step, float* velocity, float dt,
        float size, float field34, int index, float* hitTime, int flags) = 0; // vtable+0x00
};

typedef void (*PyroEmitCallback)(float* spawn, void* data);

// SYNTHETIC: VC1 0x0042ecf0
// vc1::PyroLayer::`vector deleting destructor'

// Public emitter interface; only the destructor has a body.
// SIZE 0x04
// VTABLE: VC1 0x004d7c10
class IPyroEmitter
{
public:
    VC1_PYRO_HEAP

    virtual void Reset() = 0;                                     // vtable+0x00
    virtual int Emitter04(int a, int b) = 0;                      // vtable+0x04
    virtual void Emitter08() = 0;                                 // vtable+0x08
    virtual void Move(float time, float dt, float x, float y, float z, bool emit) = 0; // vtable+0x0c
    virtual void Update(float time, float dt) = 0;                // vtable+0x10
    virtual void Prepare(float a, float b) = 0;                   // vtable+0x14
    virtual bool Is3D() = 0;                                      // vtable+0x18
    virtual const char* GetName() = 0;                            // vtable+0x1c
    virtual void SetRotation(const float* matrix) = 0;            // vtable+0x20
    virtual void SetMatrix(const float* matrix) = 0;              // vtable+0x24
    virtual void SetPosition(float x, float y, float z) = 0;      // vtable+0x28
    virtual void GetPosition(float* x, float* y, float* z) = 0;   // vtable+0x2c
    virtual void SetColor(float r, float g, float b, float a) = 0; // vtable+0x30
    virtual void GetColor(float* r, float* g, float* b, float* a) = 0; // vtable+0x34
    virtual bool HasParticles() = 0;                              // vtable+0x38
    virtual int GetParticleCount() = 0;                           // vtable+0x3c
    virtual void AddChild(void* child) = 0;                       // vtable+0x40
    virtual void SetOffset(float x, float y, float z) = 0;        // vtable+0x44
    virtual void GetOffset(float* x, float* y, float* z) = 0;     // vtable+0x48
    virtual void SetScale(float scale) = 0;                       // vtable+0x4c
    virtual float GetScale() = 0;                                 // vtable+0x50
    virtual void SetEmitCallback(PyroEmitCallback callback, void* data) = 0; // vtable+0x54
    virtual void SetUserData(void* data) = 0;                     // vtable+0x58
    virtual void* GetUserData() = 0;                              // vtable+0x5c
    virtual float GetField210() = 0;                              // vtable+0x60
    virtual bool HasActiveLayers() = 0;                           // vtable+0x64
    virtual ~IPyroEmitter() {}                                    // vtable+0x68
};

// SYNTHETIC: VC1 0x0042e7e0
// vc1::IPyroEmitter::`scalar deleting destructor'

// Emitter prototype (owned by PyroFile) or instance (PyroFile::CreateEmitter
// copies the prototype; isInstance then marks the shared curves and name).
// SIZE 0x248
// VTABLE: VC1 0x004d7d08
class PyroEmitter : public IPyroEmitter
{
public:
    PyroCurve curves[19];       // 0x004
    float scale;                // 0x0e8
    PyroVec3 offset;            // 0x0ec
    float values[19];           // 0x0f8  curves evaluated by MoveTo
    float orientation[3][3];    // 0x144  from direction
    float direction[3];         // 0x168
    float movePosition[3];      // 0x174
    bool emitOnMove;            // 0x180
    PyroLibrary* library;       // 0x184
    PyroLayer* parentLayer;     // 0x188
    PyroFile* file;             // 0x18c
    char* name;                 // 0x190
    bool field194;              // 0x194  flip spawn directions
    bool field195;              // 0x195  ... alternating (with field194)
    bool field196;              // 0x196  even spacing instead of random
    int field198;               // 0x198  spawn points per emission
    int field19c;               // 0x19c  shape: 0 point, 1 line, else circle
    int layerCount;             // 0x1a0
    PyroLayer* layers;          // 0x1a4
    bool isInstance;            // 0x1a8
    bool field1a9;              // 0x1a9
    bool field1aa;              // 0x1aa
    bool field1ab;              // 0x1ab
    int field1ac;               // 0x1ac
    float color[4];             // 0x1b0
    float position[3];          // 0x1c0
    float rotation[3][3];       // 0x1cc
    void** children;            // 0x1f0
    int childCount;             // 0x1f4
    int childCapacity;          // 0x1f8
    PyroEmitter* parent;        // 0x1fc
    PyroEmitter* prototype;     // 0x200
    PyroEmitCallback emitCallback; // 0x204
    void* emitCallbackData;     // 0x208
    void* userData;             // 0x20c
    float field210;             // 0x210
    float matrix[3][4];         // 0x214
    bool matrixIdentity;        // 0x244

    PyroEmitter();
    explicit PyroEmitter(PyroEmitter* source);
    ~PyroEmitter();

    void MarkInstance();
    void Deserialize(PyroArchive& ar, int version);
    void MoveTo(float time, float dt, float x, float y, float z, bool emit);
    void GetRotation(float* out);
    void GetOrientation(float* out);
    void UpdateOrientation();
    void GetSpawn(float* angle, float* x, float* y, float* z, float* dx, float* dy, float* dz,
        int angleMode, float angleDegrees, int index, float minPitch, float maxPitch);

    void Reset();
    int Emitter04(int a, int b);
    void Emitter08();
    void Move(float time, float dt, float x, float y, float z, bool emit);
    void Update(float time, float dt);
    void Prepare(float a, float b);
    bool Is3D();
    const char* GetName();
    void SetRotation(const float* matrix);
    void SetMatrix(const float* matrix);
    void SetPosition(float x, float y, float z);
    void GetPosition(float* x, float* y, float* z);
    void SetColor(float r, float g, float b, float a);
    void GetColor(float* r, float* g, float* b, float* a);
    bool HasParticles();
    int GetParticleCount();
    void AddChild(void* child);
    void SetOffset(float x, float y, float z);
    void GetOffset(float* x, float* y, float* z);
    void SetScale(float scale);
    float GetScale();
    void SetEmitCallback(PyroEmitCallback callback, void* data);
    void SetUserData(void* data);
    void* GetUserData();
    float GetField210();
    bool HasActiveLayers();

    virtual void GetOffsetVec(float* out); // vtable+0x6c
    virtual float GetConstant();           // vtable+0x70
};

// SYNTHETIC: VC1 0x0042ec80
// vc1::PyroEmitter::`vector deleting destructor'

typedef char Vc1PyroLayerSizeCheck[sizeof(PyroLayer) == 0x2b8 ? 1 : -1];
typedef char Vc1PyroEmitterSizeCheck[sizeof(PyroEmitter) == 0x248 ? 1 : -1];

}

#endif
