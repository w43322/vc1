#include "pyro/pyro_emitter.h"

#include "kd/display/pyro.h"
#include "pyro/pyro_matrix.h"
#include "pyro/pyro_random.h"

#include <math.h>
#include <stdlib.h>
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
bool PyroEmitter::Is3D()
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
void PyroEmitter::SetEmitCallback(PyroEmitCallback callback, void* data)
{
    emitCallback = callback;
    emitCallbackData = data;
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

// EAX=this, ECX=out (3x3).
// FUNCTION: VC1 0x0044ea70
void PyroEmitter::GetOrientation(float* out)
{
    for (int row = 0; row < 3; ++row) {
        out[row * 3 + 0] = orientation[row][0];
        out[row * 3 + 1] = orientation[row][1];
        out[row * 3 + 2] = orientation[row][2];
    }
}

// EAX=this, ECX=out (3x3).
// FUNCTION: VC1 0x0044eaa0
void PyroEmitter::GetRotation(float* out)
{
    for (int row = 0; row < 3; ++row) {
        out[row * 3 + 0] = rotation[row][0];
        out[row * 3 + 1] = rotation[row][1];
        out[row * 3 + 2] = rotation[row][2];
    }
}

// RET 8. Update with the stored move position.
// FUNCTION: VC1 0x0044ead0
void PyroEmitter::Update(float time, float dt)
{
    Move(time, dt, movePosition[0], movePosition[1], movePosition[2], emitOnMove);
}

// FUNCTION: VC1 0x0044eb20
void PyroEmitter::Render()
{
    RenderLimited(-1, 0);
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

// EAX=this, EBX=source. Passes the rotation on to every child emitter.
// FUNCTION: VC1 0x0044ec40
void PyroLayer::SetChildRotation(const float* source)
{
    for (PyroParticle* particle = particles; particle; particle = particle->next) {
        for (int i = 0; i < particle->childCount; ++i) {
            if (particle->children[i].emitter) {
                particle->children[i].emitter->SetRotation(source);
            }
        }
    }
}

// EAX=this. Particles of the child emitters.
// FUNCTION: VC1 0x0044eca0
int PyroLayer::CountParticles()
{
    int total = 0;

    if (subEmitterCount) {
        for (PyroParticle* particle = particles; particle; particle = particle->next) {
            if (particle->children) {
                for (int i = 0; i < particle->childCount; ++i) {
                    total += particle->children[i].emitter->GetParticleCount();
                }
            }
        }
    }
    return total;
}

// ESI=this, t on the stack. Piecewise linear, optionally smoothstepped.
// FUNCTION: VC1 0x0044ed10
float PyroCurve::Evaluate(float t)
{
    if (count != 1 && keys[0].time < t) {
        PyroCurveKey* key = keys;
        float f;

        if (keys[count - 1].time <= t) {
            return keys[count - 1].value;
        }
        while (key[1].time < t) {
            ++key;
        }
        f = (t - key->time) / (key[1].time - key->time);
        if (smooth) {
            f = (3.0f - (f + f)) * f * f;
        }
        return (key[1].value - key->value) * f + key->value;
    }
    return keys->value;
}

// EAX=this. Instances share the prototype's curve keys and name.
// FUNCTION: VC1 0x0044ef50
void PyroEmitter::MarkInstance()
{
    curve00.shared = true;
    curve01.shared = true;
    curve02.shared = true;
    curve03.shared = true;
    curve04.shared = true;
    curve05.shared = true;
    curve06.shared = true;
    curve07.shared = true;
    curve08.shared = true;
    curve09.shared = true;
    curve10.shared = true;
    curve11.shared = true;
    curve12.shared = true;
    curve13.shared = true;
    curve14.shared = true;
    curve15.shared = true;
    curve16.shared = true;
    curve17.shared = true;
    curve18.shared = true;
    isInstance = true;
}

// ESI=this, EDI=&direction. Yaw/pitch frame (no roll) along direction.
// FUNCTION: VC1 0x0044f0d0
void PyroEmitter::UpdateOrientation()
{
    float length = sqrtf(direction[0] * direction[0] + direction[1] * direction[1] +
        direction[2] * direction[2]);
    const float roll = 0.0f;
    float yaw;
    float pitch;
    float sy;
    float cy;
    float sp;
    float cp;
    float sr;
    float cr;

    if (fabsf(length) < 0.0001f) {
        direction[0] = 1.0f;
        direction[1] = 0.0f;
        direction[2] = 0.0f;
        length = 1.0f;
    }
    yaw = -atan2f(direction[0], direction[1]);
    pitch = asinf(direction[2] / length);
    sy = sinf(yaw);
    cy = cosf(yaw);
    sp = sinf(pitch);
    cp = cosf(pitch);
    sr = sinf(roll);
    cr = cosf(roll);
    orientation[0][0] = sr * sy * sp + cy * cr;
    orientation[1][0] = sp * cr * sy - sr * cy;
    orientation[2][0] = cp * sy;
    orientation[0][1] = cr * sy - sp * cy * sr;
    orientation[1][1] = -cy * sp * cr - sr * sy;
    orientation[2][1] = cp * -cy;
    orientation[0][2] = cp * sr;
    orientation[1][2] = cp * cr;
    orientation[2][2] = -sp;
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
        layers[i].SetChildRotation(source);
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
    curve00.shared = true;
    curve01.shared = true;
    curve02.shared = true;
    curve04.shared = true;
    curve05.shared = true;
    curve06.shared = true;
    curve03.shared = true;
    curve07.shared = true;
    curve09.shared = true;
    curve10.shared = true;
    curve11.shared = true;
    curve12.shared = true;
    curve13.shared = true;
    curve14.shared = true;
    curve15.shared = true;
    curve16.shared = true;
    curve17.shared = true;
    curve18.shared = true;
    curve19.shared = true;
    curve21.shared = true;
    curve22.shared = true;
    curve23.shared = true;
    curve20.shared = true;
    curve25.shared = true;
    curve26.shared = true;
    curve27.shared = true;
    curve28.shared = true;
    curve29.shared = true;
    curve30.shared = true;
    curve31.shared = true;
    curve32.shared = true;
    curve08.shared = true;
    curve24.shared = true;
    curve33.shared = true;
    curve34.shared = true;
    curve35.shared = true;
    curve36.shared = true;
    curve37.shared = true;
    active = false;
    isInstance = true;
}

// ESI=this. Rotation from the three layer angles (radians).
// FUNCTION: VC1 0x0044f4e0
void PyroLayer::UpdateAngles()
{
    float sa;
    float ca;
    float sb;
    float cb;
    float sc;
    float cc;

    if (angles[0] == 0.0f && angles[1] == 0.0f && angles[2] == 0.0f) {
        hasAngles = false;
        return;
    }
    hasAngles = true;
    sa = sinf(angles[0]);
    ca = cosf(angles[0]);
    sb = sinf(angles[1]);
    cb = cosf(angles[1]);
    sc = sinf(angles[2]);
    cc = cosf(angles[2]);
    rotation[0][0] = cb * cc;
    rotation[1][0] = sc * cb;
    rotation[2][0] = -sb;
    rotation[0][1] = sa * sb * cc - ca * sc;
    rotation[1][1] = sc * sa * sb + cc * ca;
    rotation[2][1] = cb * sa;
    rotation[0][2] = cc * sb * ca + sc * sa;
    rotation[1][2] = sb * ca * sc - cc * sa;
    rotation[2][2] = cb * ca;
}

// ESI=this. Texture coordinates from the flips, quad size from the shape and
// the pivot BuildQuad uses.
// FUNCTION: VC1 0x0044f650
void PyroLayer::PrepareQuads()
{
    float u0;
    float u1;
    float v0;
    float v1;
    float width;
    float height;

    if (!flipU) {
        u0 = 0.0f;
        u1 = 1.0f;
    }
    else {
        u0 = 1.0f;
        u1 = 0.0f;
    }
    if (flipV == emitter->Is3D()) {
        v0 = 0.0f;
        v1 = 1.0f;
    }
    else {
        v0 = 1.0f;
        v1 = 0.0f;
    }
    uv[0] = u0;
    uv[1] = v0;
    uv[2] = u1;
    uv[3] = v1;
    if (!shape) {
        height = 64.0f;
        width = 64.0f;
    }
    else {
        width = shape->field10;
        height = shape->field14;
    }
    size[0] = width;
    size[1] = height;
    pivot[0] = pivotSource[0];
    pivot[1] = pivotSource[1];
}

// ECX=this, EBX=b, EDI=a, t/r/g on the stack; RET 0xc. With repeat > 1 the
// key range wraps repeat times over t.
// FUNCTION: VC1 0x0044f780
void PyroKeyList::Sample(float t, float* r, float* g, float* b, float* a)
{
    if (count != 1 && keys[0].time < t) {
        PyroColorKey* key = keys;
        float f;

        if (repeat > 1 && count > 1) {
            t = fmodf(t * repeat, keys[count - 1].time - keys[0].time) + keys[0].time;
        }
        else if (keys[count - 1].time <= t) {
            *r = keys[count - 1].r;
            *g = keys[count - 1].g;
            *b = keys[count - 1].b;
            *a = keys[count - 1].a;
            return;
        }
        while (key[1].time < t) {
            ++key;
        }
        f = (t - key->time) / (key[1].time - key->time);
        *r = key->r + f * (key[1].r - key->r);
        *g = (key[1].g - key->g) * f + key->g;
        *b = (key[1].b - key->b) * f + key->b;
        *a = (key[1].a - key->a) * f + key->a;
        return;
    }
    *r = keys->r;
    *g = keys->g;
    *b = keys->b;
    *a = keys->a;
}

// EAX=this; RET 0xc. Weighted pick; choices with flag set also get a random
// unit axis and angle.
// FUNCTION: VC1 0x0044fa90
PyroGraphicRef* PyroGraphicChoices::Pick(float* axis, float* angle, float* scale)
{
    PyroGraphicChoice* choice;
    float lengthSq;
    float inverse;

    if (count == 0) {
        axis[0] = 0.0f;
        axis[1] = 0.0f;
        axis[2] = 1.0f;
        *angle = 0.0f;
        *scale = 1.0f;
        return 0;
    }
    if (count == 1) {
        choice = choices;
    }
    else if (total == 0) {
        choice = &choices[PyroGetRandom()->Next() % count];
    }
    else {
        unsigned value = PyroGetRandom()->Next() % total;
        choice = &choices[count - 1];
        for (int i = 0; i < count - 1; ++i) {
            if (value < static_cast<unsigned>(choices[i].weight)) {
                choice = &choices[i];
                break;
            }
            value -= choices[i].weight;
        }
    }
    *scale = choice->value;
    if (!choice->flag) {
        axis[0] = 0.0f;
        axis[1] = 0.0f;
        axis[2] = 1.0f;
        *angle = 0.0f;
        return choice->ref;
    }
    do {
        axis[0] = static_cast<float>(PyroRandomSigned());
        axis[1] = static_cast<float>(PyroRandomSigned());
        axis[2] = static_cast<float>(PyroRandomSigned());
        lengthSq = axis[0] * axis[0] + axis[1] * axis[1] + axis[2] * axis[2];
    } while (lengthSq < 0.1f);
    inverse = 1.0f / sqrtf(lengthSq);
    axis[0] = inverse * axis[0];
    axis[1] = axis[1] * inverse;
    axis[2] = inverse * axis[2];
    *angle = static_cast<float>(PyroRandomUnit() * 6.283185307179586);
    return choice->ref;
}

// FUNCTION: VC1 0x0044fd60
PyroEmitter::PyroEmitter()
{
    direction[0] = 0.0f;
    direction[1] = 0.0f;
    direction[2] = 0.0f;
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
    emitCallback = 0;
    emitCallbackData = 0;
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

// ECX=this, EBX=angle, eleven stack arguments; RET 0x2c. Spawn point on the
// emitter shape and launch direction; *angle ends up as the particle angle,
// the direction uses the shape angle before angleMode replaces it.
// FUNCTION: VC1 0x00450160
void PyroEmitter::GetSpawn(float* angle, float* x, float* y, float* z, float* dx, float* dy,
    float* dz, int angleMode, float angleDegrees, int index, float minPitch, float maxPitch)
{
    bool flip = field194;
    float base;

    if (flip && field195) {
        if (field196) {
            flip = (index & 1) != 0;
            index /= 2;
        }
        else {
            flip = (PyroGetRandom()->Next() & 1) != 0;
        }
    }
    {
        const float r = static_cast<float>(PyroRandomSigned());
        *angle = values[9] * r * 0.5f + values[8];
    }
    if (field19c == 0) {
        *x = movePosition[0];
        *y = movePosition[1];
        *z = movePosition[2];
    }
    else if (field19c == 1) {
        float t;

        if (field196) {
            if (field198 > 1) {
                t = static_cast<float>(index) / (field198 - 1);
            }
            else {
                t = 0.0f;
            }
        }
        else {
            t = static_cast<float>(PyroRandomUnit());
        }
        t = (t - 0.5f) * 2.0f;
        *x = cosf(values[14]) * values[15] * t + movePosition[0];
        *y = sinf(values[14]) * values[15] * t + movePosition[1];
        *z = movePosition[2];
        *angle = values[14] + 1.57075f + *angle;
    }
    else {
        float t;
        float spread;

        if (field196) {
            t = static_cast<float>(index) / field198;
        }
        else {
            t = static_cast<float>(PyroRandomUnit());
        }
        spread = t * 6.283f;
        t = spread + values[14];
        *x = cosf(t) * values[15] + movePosition[0];
        *y = sinf(t) * values[15] + movePosition[1];
        *z = movePosition[2];
        *angle = *angle + t;
    }
    base = *angle;
    if (angleMode == 2) {
        *angle = angleDegrees * 0.0174533f;
    }
    else if (angleMode == 0) {
        *angle = angleDegrees * 0.0174533f + *angle;
    }
    else {
        *angle = static_cast<float>(PyroRandomUnit() * 6.283185307179586);
    }
    if (Is3D()) {
        float low = minPitch;
        float high = maxPitch;
        float pitch;
        float r;
        float s;

        if (high < low) {
            low = maxPitch;
            high = minPitch;
        }
        r = static_cast<float>(PyroRandomUnit());
        pitch = (high - low) * r + low;
        pitch = 3.1415f - pitch;
        s = sinf(pitch);
        *dx = cosf(base) * s;
        *dy = sinf(base) * s;
        *dz = -cosf(pitch);
    }
    else {
        *dx = cosf(base);
        *dy = sinf(base);
        *dz = 0.0f;
    }
    if (flip) {
        *dx = -*dx;
        *dy = -*dy;
        *dz = -*dz;
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
    keyList.random = false;
    keyList.shared = false;
    time = 0.0f;
    particles = 0;
    pivotSource[1] = 0;
    shape = 0;
    pivotSource[0] = 0;
    library = 0;
    active = false;
    subEmitterCount = 0;
    subEmitters = 0;
    isInstance = false;
    field2b4 = 0;
    particleCount = 0;
    hasAngles = false;
}

// ESI=particle, stack this; RET 4. Unlinks the particle, deletes its child
// emitters and returns it to the pool.
// FUNCTION: VC1 0x00450910
void PyroLayer::FreeParticle(PyroParticle* particle)
{
    PyroParticle* const prev = particle->prev;
    PyroParticle* const next = particle->next;

    if (prev) {
        prev->next = next;
    }
    if (next) {
        next->prev = prev;
    }
    if (!prev) {
        particles = next;
    }
    if (particle->children) {
        for (int i = 0; i < particle->childCount; ++i) {
            if (particle->children[i].emitter) {
                delete particle->children[i].emitter;
            }
        }
        free(particle->children);
        particle->children = 0;
        particle->childCount = 0;
    }
    PyroParticleRelease(particle);
}

// RET 8. value scaled by a random factor in [1 - variation, 1 + variation];
// above 1 the range is shifted up so the result stays positive.
// FUNCTION: VC1 0x00450a00
float PyroVary(float value, float variation)
{
    const float r = static_cast<float>(PyroRandomSigned());

    if (variation > 1.0f) {
        return value + value * (r * variation + (variation - 1.0f));
    }
    return value + value * r * variation;
}

// Sum of every layer's live and pending particles.
// FUNCTION: VC1 0x00450a90
int PyroEmitter::GetParticleCount()
{
    int total = 0;
    for (int i = 0; i < layerCount; ++i) {
        PyroLayer* const layer = &layers[i];
        total += layer->CountParticles() + layer->particleCount;
    }
    return total;
}

// Stack this, now, dt; RET 0xc. Moves every particle: velocity, emitter
// drift and turbulence; with attached colliders the step is clipped against
// them (the emitter's and then the prototype's list); spin and drag act
// around the spawn origin.
// FUNCTION: VC1 0x00450af0
void PyroLayer::Prepare(float now, float dt)
{
    const float t = emitter->GetConstant() * now;
    const float d = emitter->GetConstant() * dt;
    int index = 0;

    for (PyroParticle* particle = particles; particle; particle = particle->next, ++index) {
        float noiseX;
        float noiseY;
        float noiseZ;
        float vx;
        float vy;
        float vz;
        float x;
        float y;
        float z;

        if (particle->cur50 != 0.0f) {
            const float amplitude = particle->cur50 * 0.01f;
            const float base = t * 0.05f + particle->phase;
            const float base450 = 450.0f + base;
            const float span = d * 0.05f;

            noiseX = PyroNoise(base, span + base) * amplitude;
            noiseY = PyroNoise(base450, base450 + span) * amplitude;
            if (emitter->Is3D()) {
                const float base940 = base + 940.0f;
                noiseZ = PyroNoise(base940, base940 + span) * amplitude;
            }
            else {
                noiseZ = 0.0f;
            }
        }
        else {
            noiseZ = 0.0f;
            noiseY = 0.0f;
            noiseX = 0.0f;
        }
        vx = particle->cur3c * particle->velocity[0] + emitter->position[0] * particle->cur54;
        vy = particle->cur3c * particle->velocity[1] + emitter->position[1] * particle->cur54;
        vz = particle->velocity[2] * particle->cur3c + emitter->position[2] * particle->cur54;
        if (!emitter->children) {
            const float speed = particle->base4c;

            x = particle->position[0] + speed * vx * (dt + noiseX);
            y = speed * vy * (noiseY + dt) + particle->position[1];
            z = particle->position[2] + (noiseZ + dt) * vz * speed;
        }
        else {
            float position[3];
            float step[3];
            float velocity[3];
            float time = now;
            float left = dt;
            IPyroCollider* last = 0;
            bool hit;

            position[0] = particle->position[0];
            position[1] = particle->position[1];
            position[2] = particle->position[2];
            step[0] = particle->base4c * vx;
            step[1] = particle->base4c * vy;
            step[2] = particle->base4c * vz;
            velocity[0] = particle->velocity[0];
            velocity[1] = particle->velocity[1];
            velocity[2] = particle->velocity[2];
            do {
                IPyroCollider* best = 0;
                float bestTime = 0.0f;
                float savedPosition[3];
                float savedStep[3];
                float savedVelocity[3];

                hit = false;
                for (int pass = 0; pass < 2; ++pass) {
                    PyroEmitter* source;

                    if (pass == 0) {
                        source = emitter;
                    }
                    else {
                        source = emitter->prototype;
                        if (source->children == emitter->children) {
                            break;
                        }
                    }
                    for (int k = 0; k < source->childCount; ++k) {
                        IPyroCollider* const collider = static_cast<IPyroCollider*>(source->children[k]);
                        float hitTime;
                        float tryPosition[3];
                        float tryStep[3];
                        float tryVelocity[3];

                        if (collider == last) {
                            continue;
                        }
                        hitTime = left;
                        tryPosition[0] = position[0];
                        tryPosition[1] = position[1];
                        tryPosition[2] = position[2];
                        tryStep[0] = step[0];
                        tryStep[1] = step[1];
                        tryStep[2] = step[2];
                        tryVelocity[0] = velocity[0];
                        tryVelocity[1] = velocity[1];
                        tryVelocity[2] = velocity[2];
                        if (collider->Collide(time, tryPosition, tryStep, tryVelocity, left,
                                particle->cur40, particle->field34, index, &hitTime, 0)) {
                            if (!hit || hitTime < bestTime) {
                                bestTime = hitTime;
                            }
                            best = collider;
                            hit = true;
                            savedPosition[0] = tryPosition[0];
                            savedPosition[1] = tryPosition[1];
                            savedPosition[2] = tryPosition[2];
                            savedStep[0] = tryStep[0];
                            savedStep[1] = tryStep[1];
                            savedStep[2] = tryStep[2];
                            savedVelocity[0] = tryVelocity[0];
                            savedVelocity[1] = tryVelocity[1];
                            savedVelocity[2] = tryVelocity[2];
                        }
                    }
                }
                if (hit) {
                    position[0] = savedPosition[0];
                    position[1] = savedPosition[1];
                    position[2] = savedPosition[2];
                    step[0] = savedStep[0];
                    step[1] = savedStep[1];
                    step[2] = savedStep[2];
                    velocity[0] = savedVelocity[0];
                    velocity[1] = savedVelocity[1];
                    velocity[2] = savedVelocity[2];
                    time = bestTime + time;
                    left = left - bestTime;
                    if (left <= 0.0f) {
                        break;
                    }
                }
                last = best;
            } while (hit);
            if (left > 0.0f) {
                position[0] = left * step[0] + position[0];
                position[1] = left * step[1] + position[1];
                position[2] = left * step[2] + position[2];
            }
            x = position[0];
            y = position[1];
            z = position[2];
            particle->velocity[0] = velocity[0];
            particle->velocity[1] = velocity[1];
            particle->velocity[2] = velocity[2];
        }
        if (particle->cur58 != 0.0f || particle->cur5c != 0.0f) {
            float dx = x - particle->origin[0];
            float dy = y - particle->origin[1];

            if (particle->cur58 != 0.0f) {
                const float spin = dt * particle->cur58;
                const float s = sinf(spin);
                const float c = cosf(spin);
                const float rx = dx * c - dy * s;

                dy = c * dy + s * dx;
                dx = rx;
                if (matrixMode != 1) {
                    particle->spawnAngle += spin;
                }
            }
            if (particle->cur5c != 0.0f) {
                const float drag = 1.0f - particle->cur5c * dt;
                dx = drag * dx;
                dy = drag * dy;
            }
            x = dx + particle->origin[0];
            y = dy + particle->origin[1];
        }
        particle->position[0] = x;
        particle->position[1] = y;
        particle->position[2] = z;
        particle->spawnAngle = particle->spawnAngle + particle->cur44 * dt;
        particle->angle = dt * particle->cur48 + particle->angle;
        if (particle->children) {
            for (int i = 0; i < particle->childCount; ++i) {
                PyroParticle::Child* const child = &particle->children[i];
                if (child->emitter) {
                    child->emitter->Prepare(now - child->time, dt);
                }
            }
        }
    }
}

// ESI=this. matrix = rotation * the root emitter's rotation (mode 1) or
// orientation (mode 2); without angles the root rotation itself.
// FUNCTION: VC1 0x00451220
void PyroLayer::UpdateMatrix()
{
    PyroEmitter* root = emitter;
    float local[3][3];

    while (root->parent) {
        root = root->parent;
    }
    if (!hasAngles) {
        root->GetRotation(&matrix[0][0]);
        return;
    }
    if (matrixMode == 1) {
        root->GetRotation(&local[0][0]);
    }
    else if (matrixMode == 2) {
        root->GetOrientation(&local[0][0]);
    }
    else {
        for (int row = 0; row < 3; ++row) {
            matrix[row][0] = rotation[row][0];
            matrix[row][1] = rotation[row][1];
            matrix[row][2] = rotation[row][2];
        }
        return;
    }
    for (int row = 0; row < 3; ++row) {
        matrix[row][0] = local[2][0] * rotation[row][2] + local[1][0] * rotation[row][1] +
            local[0][0] * rotation[row][0];
        matrix[row][1] = local[2][1] * rotation[row][2] + local[1][1] * rotation[row][1] +
            local[0][1] * rotation[row][0];
        matrix[row][2] = local[2][2] * rotation[row][2] + local[1][2] * rotation[row][1] +
            local[0][2] * rotation[row][0];
    }
}

// EAX=out, EBX=this, stack particle; RET 4. Four vertices for one particle:
// colour between the key colour and the spawn colour, size from the shape,
// position between the particle and the emitter (attach), rotated by the
// spawn angle around the pivot, scaled and offset by the root emitter. 3D
// layers rotate the layer axes around the layer normal instead and also hand
// the particle's graphic object transform to the device (slot +0x1c); 2D
// quads go through the root emitter matrix. The 3D path applies the pivot a
// second time on top of the 2D one, and its object transform starts from a
// zero matrix rather than identity.
// FUNCTION: VC1 0x004513d0
void PyroLayer::BuildQuad(PyroVertex* out, PyroParticle* particle)
{
    float r;
    float g;
    float b;
    float a;
    float x;
    float y;
    float z;
    float offset[3];
    unsigned color;

    const float blend = particle->fieldbc;
    if (blend == 0.0f) {
        r = particle->keyColor[0];
        g = particle->keyColor[1];
        b = particle->keyColor[2];
        a = particle->keyColor[3];
    }
    else if (blend == 1.0f) {
        r = particle->color[0];
        g = particle->color[1];
        b = particle->color[2];
        a = particle->color[3];
    }
    else {
        const float keep = 1.0f - blend;
        r = particle->keyColor[0] * keep + particle->color[0] * blend;
        g = particle->keyColor[1] * keep + particle->color[1] * blend;
        b = particle->keyColor[2] * keep + particle->color[2] * blend;
        a = keep * particle->keyColor[3] + particle->color[3] * blend;
    }
    const float particleSize = particle->cur40;
    const float width = size[0] * particleSize;
    const float height = particleSize * size[1];
    const float pivotX = pivot[0];
    const float pivotY = pivot[1];
    if (attach == 0.0f) {
        x = particle->position[0];
        y = particle->position[1];
        z = particle->position[2];
    }
    else if (attach == 1.0f) {
        x = emitter->movePosition[0] + particle->offset[0];
        y = emitter->movePosition[1] + particle->offset[1];
        z = emitter->movePosition[2] + particle->offset[2];
    }
    else {
        x = (emitter->movePosition[0] + particle->offset[0] - particle->position[0]) * attach +
            particle->position[0];
        y = (emitter->movePosition[1] + particle->offset[1] - particle->position[1]) * attach +
            particle->position[1];
        z = (emitter->movePosition[2] + particle->offset[2] - particle->position[2]) * attach +
            particle->position[2];
    }
    const float s = sinf(-particle->spawnAngle);
    const float c = cosf(particle->spawnAngle);
    const float ax = c * width;
    const float ay = -width * s;
    const float bx = s * height;
    const float by = c * height;
    float cx = x - ax * pivotX - bx * pivotY;
    float cy = y - ay * pivotX - by * pivotY;

    const int red = static_cast<int>(r * 255.0f + 0.5f);
    const int green = static_cast<int>(g * 255.0f + 0.5f);
    const int blue = static_cast<int>(b * 255.0f + 0.5f);
    const int alpha = static_cast<int>(particle->cur60 * a * 255.0f + 0.5f);
    if (library->device->IsRgba()) {
        color = ((((alpha << 8) | blue) << 8 | green) << 8) | red;
    }
    else {
        color = ((((alpha << 8) | red) << 8 | green) << 8) | blue;
    }

    PyroEmitter* root = emitter;
    while (root->parent) {
        root = root->parent;
    }
    const float scale = emitter->GetScale();
    root->GetOffsetVec(offset);

    if (emitter->Is3D() && (matrixMode == 1 || hasAngles)) {
        float ux;
        float uy;
        float uz;
        float vx;
        float vy;
        float vz;

        if (particle->spawnAngle == 0.0f) {
            ux = matrix[0][0];
            uy = matrix[0][1];
            uz = matrix[0][2];
            vx = matrix[1][0];
            vy = matrix[1][1];
            vz = matrix[1][2];
        }
        else {
            float spin[3][3];
            const float half = particle->spawnAngle * 0.5f;
            const float hc = cosf(half);
            const float hs = sinf(half);
            const float qx = matrix[2][0] * hs;
            const float x2 = qx + qx;
            const float xx = x2 * qx;
            const float qy = matrix[2][1] * hs;
            const float y2 = qy + qy;
            const float yy = qy * y2;
            const float qz = matrix[2][2] * hs;
            const float z2 = qz + qz;
            const float zz = z2 * qz;
            const float wx = x2 * hc;
            const float wy = y2 * hc;
            const float wz = hc * z2;
            const float xy = y2 * qx;
            const float xz = qx * z2;
            const float yz = qz * y2;

            spin[0][0] = 1.0f - yy - zz;
            spin[1][1] = 1.0f - xx - zz;
            spin[2][2] = 1.0f - xx - yy;
            spin[0][1] = xy - wz;
            spin[1][0] = xy + wz;
            spin[0][2] = xz + wy;
            spin[2][0] = xz - wy;
            spin[1][2] = yz - wx;
            spin[2][1] = yz + wx;
            ux = matrix[0][1] * spin[1][0] + matrix[0][0] * spin[0][0] + matrix[0][2] * spin[2][0];
            uy = matrix[0][1] * spin[1][1] + matrix[0][0] * spin[0][1] + matrix[0][2] * spin[2][1];
            uz = matrix[0][1] * spin[1][2] + matrix[0][0] * spin[0][2] + matrix[0][2] * spin[2][2];
            vx = matrix[1][0] * spin[0][0] + matrix[1][1] * spin[1][0] + matrix[1][2] * spin[2][0];
            vy = matrix[1][0] * spin[0][1] + matrix[1][1] * spin[1][1] + matrix[1][2] * spin[2][1];
            vz = matrix[1][0] * spin[0][2] + spin[1][2] * matrix[1][1] + matrix[1][2] * spin[2][2];
        }
        ux *= width;
        uy *= width;
        uz *= width;
        vx *= height;
        vy *= height;
        vz *= height;
        cx = cx - ux * pivotX - vx * pivotY;
        cy = cy - uy * pivotX - vy * pivotY;
        const float cz = z - uz * pivotX - vz * pivotY;

        out[0].x = offset[0] + (cx - ux - vx) * scale;
        out[0].y = offset[1] + (cy - uy - vy) * scale;
        out[0].z = offset[2] + (cz - uz - vz) * scale;
        out[0].u = uv[0];
        out[0].color = color;
        out[0].v = uv[1];
        out[1].x = offset[0] + (cx + ux - vx) * scale;
        out[1].y = offset[1] + (cy + uy - vy) * scale;
        out[1].z = offset[2] + (cz + uz - vz) * scale;
        out[1].u = uv[2];
        out[1].color = color;
        out[1].v = uv[1];
        out[2].x = offset[0] + (vx + (cx + ux)) * scale;
        out[2].y = offset[1] + (vy + (cy + uy)) * scale;
        out[2].z = offset[2] + (vz + (cz + uz)) * scale;
        out[2].u = uv[2];
        out[2].color = color;
        out[2].v = uv[3];
        out[3].x = offset[0] + (vx + (cx - ux)) * scale;
        out[3].y = offset[1] + (vy + (cy - uy)) * scale;
        out[3].z = offset[2] + (vz + (cz - uz)) * scale;
        out[3].u = uv[0];
        out[3].color = color;
        out[3].v = uv[3];

        if (particle->graphic) {
            PyroMatrix34 zero;
            PyroMatrix34 world;
            PyroAxisAngle turn;
            PyroQuat q;
            PyroMatrix34 rotation;
            PyroMatrix34 product;

            memset(&zero, 0, sizeof(zero));
            memcpy(&world, &zero, sizeof(world));
            turn.angle = particle->angle;
            turn.axis[0] = particle->axis[0];
            turn.axis[1] = particle->axis[1];
            turn.axis[2] = particle->axis[2];
            q.FromAxisAngle(&turn);
            rotation.SetRotation(&q);
            const float objectScale = particle->scale;
            rotation.Scale(objectScale * particle->graphic->scale[0],
                particle->graphic->scale[1] * objectScale,
                objectScale * particle->graphic->scale[2], 1.0f);
            world.m[0][3] += cx;
            world.m[1][3] += cy;
            world.m[2][3] += cz;
            world = *rotation.Multiply(&product, &world);
            library->device->SetWorldMatrix(&world);
            library->device->SetWorldMatrix(&zero);
        }
        return;
    }

    out[0].u = uv[0];
    out[0].color = color;
    out[0].v = uv[1];
    out[1].u = uv[2];
    out[1].color = color;
    out[1].v = uv[1];
    out[2].u = uv[2];
    out[2].color = color;
    out[2].v = uv[3];
    out[3].u = uv[0];
    out[3].color = color;
    out[3].v = uv[3];
    const float depth = scale * z + offset[2];
    const float x0 = offset[0] + (cx - ax - bx) * scale;
    const float y0 = offset[1] + (cy - ay - by) * scale;
    const float x1 = offset[0] + (cx + ax - bx) * scale;
    const float y1 = offset[1] + (cy + ay - by) * scale;
    const float x2 = offset[0] + (bx + (cx + ax)) * scale;
    const float y2 = offset[1] + ((cy + ay) + by) * scale;
    const float x3 = offset[0] + (bx + (cx - ax)) * scale;
    const float y3 = offset[1] + ((cy - ay) + by) * scale;
    if (root->matrixIdentity) {
        out[0].x = x0;
        out[0].y = y0;
        out[0].z = depth;
        out[1].x = x1;
        out[1].y = y1;
        out[1].z = depth;
        out[2].x = x2;
        out[2].y = y2;
        out[2].z = depth;
        out[3].x = x3;
        out[3].y = y3;
        out[3].z = depth;
        return;
    }
    const float (*const m)[4] = root->matrix;
    out[0].x = y0 * m[0][1] + x0 * m[0][0] + depth * m[0][2] + m[0][3];
    out[0].y = x0 * m[1][0] + y0 * m[1][1] + depth * m[1][2] + m[1][3];
    out[0].z = y0 * m[2][1] + x0 * m[2][0] + depth * m[2][2] + m[2][3];
    out[1].x = y1 * m[0][1] + x1 * m[0][0] + depth * m[0][2] + m[0][3];
    out[1].y = x1 * m[1][0] + y1 * m[1][1] + depth * m[1][2] + m[1][3];
    out[1].z = y1 * m[2][1] + x1 * m[2][0] + depth * m[2][2] + m[2][3];
    out[2].x = y2 * m[0][1] + x2 * m[0][0] + depth * m[0][2] + m[0][3];
    out[2].y = x2 * m[1][0] + y2 * m[1][1] + depth * m[1][2] + m[1][3];
    out[2].z = y2 * m[2][1] + x2 * m[2][0] + depth * m[2][2] + m[2][3];
    out[3].x = y3 * m[0][1] + x3 * m[0][0] + depth * m[0][2] + m[0][3];
    out[3].y = x3 * m[1][0] + y3 * m[1][1] + depth * m[1][2] + m[1][3];
    out[3].z = y3 * m[2][1] + x3 * m[2][0] + depth * m[2][2] + m[2][3];
}

// Stack this, limit, randomPick; RET 0xc. Child emitters first (limit split
// by their particle share), then this layer's particles as quads batched into
// the library scratch vertices and uploaded into the device buffer ring. A
// positive limit caps the drawn count (random subset with randomPick). Returns
// the number drawn.
// FUNCTION: VC1 0x00452140
int PyroLayer::Render(int limit, int randomPick)
{
    if (!particles || !limit) {
        return 0;
    }
    int rendered = 0;
    const int live = particleCount;
    int seen = 0;
    int total = 0;
    if (limit > 0) {
        total = CountParticles() + particleCount;
    }
    if (subEmitterCount) {
        for (PyroParticle* particle = particles; particle; particle = particle->next) {
            if (!particle->children) {
                continue;
            }
            for (int i = 0; i < particle->childCount; ++i) {
                int share;
                if (limit < 0) {
                    share = -1;
                }
                else {
                    share = particle->children[i].emitter->GetParticleCount() * limit / total;
                    if (!share) {
                        continue;
                    }
                }
                rendered += particle->children[i].emitter->RenderLimited(share, randomPick);
            }
        }
    }

    PrepareQuads();
    library->device->SetRenderState(0, 1);
    library->device->SetBlendMode(field14 ? 4 : 1, field0c ? 1 : 5);
    library->device->SetTexture(0, shape ? shape->object : 0);

    PyroParticle* particle = particles;
    PyroVertexBuffer* const buffer = library->vertexBuffer;
    const int capacity = library->capacity;
    int start = library->vertexCursor;
    PyroVertex* const scratch = library->vertices;
    int batch = start / 4;
    PyroVertex* write = scratch;
    bool draw;

    if (!field0c && field13) {
        for (; particle; particle = particle->next) {
            if (limit < 0) {
                draw = true;
            }
            else if (limit == 0) {
                draw = false;
            }
            else if (randomPick) {
                draw = live <= limit ||
                    static_cast<int>(PyroGetRandom()->Next() % static_cast<unsigned>(live)) < limit;
            }
            else {
                draw = seen < limit;
            }
            if (!draw) {
                continue;
            }
            ++seen;
            if (0.01f < particle->cur60 && particle->alive) {
                BuildQuad(write, particle);
                ++batch;
                write += 4;
                if (batch == capacity) {
                    const int count = write - scratch;
                    buffer->Upload(scratch, start * sizeof(PyroVertex), count * sizeof(PyroVertex),
                        start == 0);
                    library->device->DrawQuads(buffer, start / 4, count / 4, 0);
                    start = 0;
                    write = scratch;
                    batch = 0;
                }
                ++rendered;
            }
        }
    }
    else {
        if (limit > 0 && !randomPick) {
            int skip = live < limit ? live : limit;
            while (skip > 0) {
                PyroParticle* const next = particle->next;
                if (!next) {
                    break;
                }
                --skip;
                particle = next;
            }
        }
        else {
            while (particle->next) {
                particle = particle->next;
            }
        }
        for (; particle; particle = particle->prev) {
            if (limit < 0) {
                draw = true;
            }
            else if (limit == 0) {
                draw = false;
            }
            else if (randomPick) {
                draw = live <= limit ||
                    static_cast<int>(PyroGetRandom()->Next() % static_cast<unsigned>(live)) < limit;
            }
            else {
                draw = seen < limit;
            }
            if (!draw) {
                continue;
            }
            ++seen;
            if (0.01f < particle->cur60 && particle->alive) {
                BuildQuad(write, particle);
                ++batch;
                write += 4;
                if (batch == capacity) {
                    const int count = write - scratch;
                    buffer->Upload(scratch, start * sizeof(PyroVertex), count * sizeof(PyroVertex),
                        start == 0);
                    library->device->DrawQuads(buffer, start / 4, count / 4, 0);
                    start = 0;
                    write = scratch;
                    batch = 0;
                }
                ++rendered;
            }
        }
    }
    if (write != scratch) {
        const int count = write - scratch;
        buffer->Upload(scratch, start * sizeof(PyroVertex), count * sizeof(PyroVertex), start == 0);
        library->device->DrawQuads(buffer, start / 4, count / 4, 0);
        start += count;
    }
    library->vertexCursor = start;
    return rendered;
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
    keys = new PyroColorKey[count];
    for (int i = 0; i < count; ++i) {
        ar.Read(&keys[i].time, 4);
        ar.Read(&keys[i].r, 4);
        ar.Read(&keys[i].g, 4);
        ar.Read(&keys[i].b, 4);
        ar.Read(&keys[i].a, 4);
    }
    ar.Read(&repeat, 4);
    ar.Read(&flag, 4);
    random = flag != 0;
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
    smooth = flag != 0;
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
int PyroEmitter::RenderLimited(int a, int b)
{
    int total = 0;
    for (int i = 0; i < layerCount; ++i) {
        total += layers[i].Render(a, b);
    }
    return total;
}

// ESI=this. New particles go to the head of the list.
// FUNCTION: VC1 0x00453760
PyroParticle* PyroLayer::NewParticle()
{
    PyroParticle* const particle = static_cast<PyroParticle*>(PyroParticleAlloc());

    if (!particles) {
        particles = particle;
        particle->next = 0;
        particles->prev = 0;
        return particles;
    }
    particle->prev = 0;
    particle->next = particles;
    particles->prev = particle;
    particles = particle;
    return particle;
}

// RET 8. PyroVary on the magnitude, sign kept.
// FUNCTION: VC1 0x004537d0
float PyroVarySigned(float value, float variation)
{
    if (variation == 0.0f) {
        return value;
    }
    if (value < 0.0f) {
        return -PyroVary(-value, variation);
    }
    return PyroVary(value, variation);
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

// Stack this, now, dt; RET 0xc. Ages every particle: over-life curves, key
// colour, gravity and child emitters. Expired particles stop their children,
// spawn the end-of-life sub-emitters (field1ac == 1) and are freed once the
// children are at least 0.1 s old and empty.
// FUNCTION: VC1 0x00453bc0
void PyroLayer::Update(float now, float dt)
{
    const float t = emitter->GetConstant() * now;
    PyroParticle* particle = particles;

    while (particle) {
        PyroParticle* const next = particle->next;
        const float age = t - particle->birth;
        float life;

        if (flag2) {
            life = 0.0f;
        }
        else {
            life = age / particle->lifetime;
            if (life >= 1.0f) {
                if (particle->alive) {
                    for (int i = 0; i < particle->childCount; ++i) {
                        PyroEmitter* const child = particle->children[i].emitter;
                        if (child->field1ac == 0) {
                            child->field1a9 = false;
                        }
                    }
                    for (int i = 0; i < subEmitterCount; ++i) {
                        if (subEmitters[i].field1ac == 1) {
                            PyroEmitter* const child = new PyroEmitter(&subEmitters[i]);
                            PyroEmitter* const owner = emitter;

                            ++particle->childCount;
                            particle->children = static_cast<PyroParticle::Child*>(
                                realloc(particle->children, particle->childCount * sizeof(PyroParticle::Child)));
                            particle->children[particle->childCount - 1].emitter = child;
                            particle->children[particle->childCount - 1].time = now;
                            child->parent = owner;
                        }
                    }
                    particle->alive = false;
                }
                else {
                    bool busy = false;

                    for (int i = 0; i < particle->childCount; ++i) {
                        PyroEmitter* const child = particle->children[i].emitter;
                        if (child && (now - particle->children[i].time < 0.1f || child->HasParticles())) {
                            busy = true;
                            break;
                        }
                    }
                    if (!busy) {
                        FreeParticle(particle);
                        --particleCount;
                        particle = next;
                        continue;
                    }
                }
            }
        }
        {
            const float value31 = curve31.Get(life);
            const float value27 = curve27.Get(life);
            const float value29 = curve29.Get(life);
            const float value30 = curve30.Get(life);
            const float value28 = curve28.Get(life);
            const float value33 = curve33.Get(life);
            const float value32 = curve32.Get(life);
            const float value35 = curve35.Get(life);
            const float value36 = curve36.Get(life);
            const float value37 = curve37.Get(life);
            float scaled;
            float gravity;

            if (!keyList.random || !particle->keyed) {
                particle->keyed = true;
                keyList.Sample(keyList.random ? static_cast<float>(PyroRandomUnit()) : age / particle->lifetime,
                    &particle->keyColor[0], &particle->keyColor[1], &particle->keyColor[2],
                    &particle->keyColor[3]);
            }
            particle->cur3c = particle->start6c * value28;
            particle->cur44 = particle->start70 * value29;
            particle->cur48 = particle->start74 * value30;
            particle->cur50 = particle->start78 * value31;
            particle->cur54 = particle->start84 * value32;
            particle->cur60 = particle->start64 * value35;
            particle->cur58 = particle->start88 * value36;
            particle->cur5c = particle->start8c * value37;
            scaled = particle->start7c * value27;
            particle->cur40 = scaled * particle->base4c;
            gravity = particle->gravity * value33;
            if (!emitter->Is3D()) {
                particle->velocity[1] = (t - particle->birth) * gravity + particle->velocity[1];
            }
            else {
                particle->velocity[2] = particle->velocity[2] - (t - particle->birth) * gravity;
            }
            for (int i = 0; i < particle->childCount; ++i) {
                PyroParticle::Child* const child = &particle->children[i];
                if (child->emitter) {
                    child->emitter->Move(now - child->time, dt, particle->position[0],
                        particle->position[1], particle->position[2], true);
                }
            }
        }
        particle = next;
    }
}

// Stack this, now, dt, emit; RET 0x10. Evaluates the layer curves (scaled by
// the emitter values), spawns this frame's particles and ages them.
// FUNCTION: VC1 0x00454140
void PyroLayer::Emit(float now, float dt, bool emit)
{
    if (emitter->Is3D()) {
        UpdateAngles();
        UpdateMatrix();
    }
    if (emitter->field1a9 && (!flag1 || !active)) {
        const float t = emitter->GetConstant() * now;
        float life = curve00.Get(t);
        float rate = curve01.Get(t);
        float start64 = curve02.Get(t);
        float start90 = curve03.Get(t);
        float start70 = curve04.Get(t);
        float start74 = curve05.Get(t);
        float start6c = curve06.Get(t);
        float start7c = curve07.Get(t);
        float start78 = curve09.Get(t);
        float gravity = curve08.Get(t);
        float start88 = curve13.Get(t);
        float start8c = curve14.Get(t);
        float start84 = curve10.Get(t);
        const float minPitch = curve11.Get(t);
        const float maxPitch = curve12.Get(t);
        const float lifeVariation = curve15.Get(t);
        const float start7cVariation = curve16.Get(t);
        const float rateVariation = curve17.Get(t);
        const float start6cVariation = curve18.Get(t);
        const float start78Variation = curve21.Get(t);
        const float gravityVariation = curve24.Get(t);
        const float start70Variation = curve25.Get(t);
        const float start74Variation = curve26.Get(t);
        const float start90Variation = curve19.Get(t);
        const float start84Variation = curve20.Get(t);
        const float start88Variation = curve13.Get(t);
        const float start8cVariation = curve14.Get(t);
        float emitterValue10;
        int count;

        if (flag4 && start70 != 0.0f) {
            if (PyroGetRandom()->Next() & 1) {
                start70 = -start70;
            }
        }
        life = emitter->values[0] * life;
        start70 = emitter->values[3] * start70;
        start74 = emitter->values[4] * start74;
        start7c = emitter->values[6] * start7c;
        start6c = emitter->values[5] * start6c;
        start78 = emitter->values[7] * start78;
        start64 = emitter->values[2] * start64;
        rate = emitter->values[1] * rate;
        gravity = emitter->values[13] * gravity;
        start90 = emitter->values[11] * start90;
        start84 = emitter->values[16] * start84;
        start88 = emitter->values[17] * start88;
        start8c = emitter->values[18] * start8c;
        emitterValue10 = emitter->values[10];
        if (!flag1) {
            float add;

            if (rateVariation != 0.0f) {
                rate = PyroVary(rate, rateVariation);
            }
            add = rate * dt;
            rate = add + time;
            count = static_cast<int>(rate);
            time = rate - count;
        }
        else {
            count = burstCount;
            active = true;
        }
        if (emit && (life != 0.0f || lifeVariation != 0.0f) && count > 0) {
            do {
                for (int i = 0;; ++i) {
                    PyroParticle* particle;
                    float x;
                    float y;
                    float z;
                    float dx;
                    float dy;
                    float dz;
                    float angle;
                    int points;

                    if (emitter->field19c == 0 || !emitter->field196) {
                        points = 1;
                    }
                    else if (emitter->field194 && emitter->field195) {
                        points = emitter->field198 * 2;
                    }
                    else {
                        points = emitter->field198;
                    }
                    if (i >= points) {
                        break;
                    }
                    particle = NewParticle();
                    emitter->GetSpawn(&angle, &x, &y, &z, &dx, &dy, &dz, field04,
                        static_cast<float>(field08), i, minPitch, maxPitch);
                    if (emitter->emitCallback) {
                        float spawn[7];

                        spawn[0] = x;
                        spawn[1] = y;
                        spawn[2] = z;
                        spawn[3] = dx;
                        spawn[4] = dy;
                        spawn[5] = dz;
                        spawn[6] = angle;
                        emitter->emitCallback(spawn, emitter->emitCallbackData);
                        x = spawn[0];
                        y = spawn[1];
                        z = spawn[2];
                        dx = spawn[3];
                        dy = spawn[4];
                        dz = spawn[5];
                        angle = spawn[6];
                    }
                    ++particleCount;
                    particle->position[0] = x;
                    particle->alive = true;
                    particle->position[1] = y;
                    particle->keyed = false;
                    particle->childCount = 0;
                    particle->position[2] = z;
                    particle->children = 0;
                    particle->offset[0] = x - emitter->movePosition[0];
                    particle->offset[1] = y - emitter->movePosition[1];
                    particle->offset[2] = z - emitter->movePosition[2];
                    particle->origin[0] = emitter->movePosition[0];
                    particle->origin[1] = emitter->movePosition[1];
                    particle->origin[2] = emitter->movePosition[2];
                    particle->velocity[0] = dx;
                    particle->velocity[1] = dy;
                    particle->velocity[2] = dz;
                    particle->spawnAngle = angle;
                    particle->base4c = emitter->values[12];
                    particle->phase = static_cast<float>(rand() & 0x7fff) * 0.1432f;
                    emitter->GetColor(&particle->color[0], &particle->color[1], &particle->color[2],
                        &particle->color[3]);
                    particle->fieldbc = emitterValue10;
                    particle->lifetime = lifeVariation == 0.0f ? life : PyroVary(life, lifeVariation);
                    particle->birth = t;
                    if (!flag4) {
                        particle->start70 = start70Variation != 0.0f ? PyroVary(start70, start70Variation) : start70;
                    }
                    else {
                        particle->start70 = start70Variation != 0.0f ? PyroVarySigned(start70, start70Variation) : start70;
                    }
                    particle->start74 = start74Variation == 0.0f ? start74 : PyroVary(start74, start74Variation);
                    particle->start64 = start64;
                    particle->start7c = start7cVariation == 0.0f ? start7c : PyroVary(start7c, start7cVariation);
                    particle->start6c = start6cVariation == 0.0f ? start6c : PyroVary(start6c, start6cVariation);
                    particle->start78 = start78Variation == 0.0f ? start78 : PyroVary(start78, start78Variation);
                    particle->gravity = gravityVariation == 0.0f ? gravity : PyroVary(gravity, gravityVariation);
                    particle->start84 = start84Variation == 0.0f ? start84 : PyroVary(start84, start84Variation);
                    particle->start88 = start88Variation == 0.0f ? start88 : PyroVary(start88, start88Variation);
                    particle->start8c = start8cVariation == 0.0f ? start8c : PyroVary(start8c, start8cVariation);
                    particle->start90 = start90Variation == 0.0f ? start90 : PyroVary(start90, start90Variation);
                    particle->graphic = graphics.Pick(particle->axis, &particle->angle, &particle->scale);
                    particle->childCount = 0;
                    particle->children = 0;
                    for (int j = 0; j < subEmitterCount; ++j) {
                        if (subEmitters[j].field1ac == 0) {
                            PyroEmitter* const child = new PyroEmitter(&subEmitters[j]);
                            PyroEmitter* const owner = emitter;

                            ++particle->childCount;
                            particle->children = static_cast<PyroParticle::Child*>(
                                realloc(particle->children, particle->childCount * sizeof(PyroParticle::Child)));
                            particle->children[particle->childCount - 1].emitter = child;
                            particle->children[particle->childCount - 1].time = now;
                            child->parent = owner;
                        }
                    }
                }
            } while (--count);
        }
    }
    Update(now, dt);
}

// EBX=this, five floats and a flag on the stack; RET 0x18. Evaluates the
// emitter curves into values[] and emits/ages every layer.
// FUNCTION: VC1 0x00454f70
void PyroEmitter::MoveTo(float time, float dt, float x, float y, float z, bool emit)
{
    float t;

    movePosition[0] = x;
    movePosition[1] = y;
    movePosition[2] = z;
    UpdateOrientation();
    t = GetConstant() * time;
    values[0] = curve00.Get(t);
    values[1] = curve01.Get(t);
    values[2] = curve02.Get(t);
    values[3] = curve03.Get(t);
    values[4] = curve04.Get(t);
    values[5] = curve05.Get(t);
    values[6] = curve10.Get(t);
    values[7] = curve11.Get(t);
    values[13] = curve06.Get(t);
    values[8] = curve12.Get(t);
    values[9] = curve13.Get(t);
    values[14] = curve14.Get(t);
    values[12] = curve08.Get(t);
    values[15] = curve15.Get(t);
    values[10] = curve09.Get(t);
    values[16] = curve16.Get(t);
    values[17] = curve17.Get(t);
    values[18] = curve18.Get(t);
    values[11] = curve07.Get(t);
    for (int i = 0; i < layerCount; ++i) {
        layers[i].Emit(time, dt, emit);
    }
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
    flipU = value != 0;
    ar.Read(&value, 4);
    flipV = value != 0;
    ar.Read(&value, 4);
    flag1 = (value & 1) != 0;
    flag4 = (value & 4) != 0;
    flag2 = (value & 2) != 0;
    keyList.Read(ar);
    curve00.Read(ar, version);   // 0x20
    curve07.Read(ar, version);   // 0x74
    curve04.Read(ar, version);   // 0x50
    curve06.Read(ar, version);   // 0x68
    curve01.Read(ar, version);   // 0x2c
    curve02.Read(ar, version);   // 0x38
    curve09.Read(ar, version);   // 0x8c
    curve15.Read(ar, version);  // 0xd4
    curve16.Read(ar, version);  // 0xe0
    curve17.Read(ar, version);  // 0xec
    curve21.Read(ar, version);  // 0x11c
    curve18.Read(ar, version);  // 0xf8
    curve27.Read(ar, version);  // 0x164
    curve31.Read(ar, version);  // 0x194
    curve28.Read(ar, version);  // 0x170
    curve29.Read(ar, version);  // 0x17c
    curve08.Read(ar, version);   // 0x80
    curve33.Read(ar, version);  // 0x1ac
    curve24.Read(ar, version);  // 0x140
    curve10.Read(ar, version);  // 0x98
    curve20.Read(ar, version);  // 0x110
    curve32.Read(ar, version);  // 0x1a0
    curve35.Read(ar, version);  // 0x1c4
    ar.Read(&attach, 4);
    ar.Read(&value, 4);
    field13 = value != 0;
    curve03.Read(ar, version);   // 0x44
    curve19.Read(ar, version);  // 0x104
    curve34.Read(ar, version);  // 0x1b8
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
    ar.Read(&matrixMode, 4);
    ar.Read(&angles[0], 4);
    ar.Read(&angles[1], 4);
    ar.Read(&angles[2], 4);
    curve11.Read(ar, version);  // 0xa4
    curve12.Read(ar, version);  // 0xb0
    curve13.Read(ar, version);  // 0xbc
    curve14.Read(ar, version);  // 0xc8
    curve22.Read(ar, version);  // 0x128
    curve23.Read(ar, version);  // 0x134
    curve36.Read(ar, version);  // 0x1d0
    curve37.Read(ar, version);  // 0x1dc
    curve25.Read(ar, version);  // 0x14c
    graphics.Read(ar);
    ar.Read(&value, 4);
    field14 = value != 0;
    ar.Read(&burstCount, 4);
    curve05.Read(ar, version);   // 0x5c
    curve26.Read(ar, version);  // 0x158
    curve30.Read(ar, version);  // 0x188
    ar.Read(&pivotSource[0], 4);
    ar.Read(&pivotSource[1], 4);
    name.Release();
}

// RET 8 (ar, version).
// FUNCTION: VC1 0x00455bc0
void PyroEmitter::Deserialize(PyroArchive& ar, int version)
{
    PyroString text;
    int value;

    curve00.Read(ar, version);   // 0x04
    curve10.Read(ar, version);  // 0x7c
    curve01.Read(ar, version);   // 0x10
    curve03.Read(ar, version);   // 0x28
    curve05.Read(ar, version);   // 0x40
    curve02.Read(ar, version);   // 0x1c
    curve11.Read(ar, version);  // 0x88
    curve12.Read(ar, version);  // 0x94
    curve13.Read(ar, version);  // 0xa0
    curve06.Read(ar, version);   // 0x4c
    curve08.Read(ar, version);   // 0x64
    curve14.Read(ar, version);  // 0xac
    curve15.Read(ar, version);  // 0xb8
    curve09.Read(ar, version);   // 0x70
    curve16.Read(ar, version);  // 0xc4
    ar.Read(&position[0], 4);
    ar.Read(&position[1], 4);
    position[2] = 0.0f;
    curve07.Read(ar, version);   // 0x58
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
    curve17.Read(ar, version);  // 0xd0
    curve18.Read(ar, version);  // 0xdc
    curve04.Read(ar, version);   // 0x34
    if (version >= 0x16000) {
        ar.Read(&field1ac, 4);
    }
    if (version >= 0x24000) {
        ar.Read(&field210, 4);
    }
    text.Release();
}

}
