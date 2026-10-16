#ifndef VC1_KD_EFFECT_H
#define VC1_KD_EFFECT_H

#include "kd/iface/ieffect.h"
#include "kd/iface/iloadable.h"
#include "kd/iface/icomponent_faces.h"
#include "kd/iface/iscript_object.h"
#include "kd/iface/iabstract.h"
#include "bindings/matrix3.h"
#include "bindings/rectf.h"
#include "pyro/pyro_stream.h"

namespace vc1 {

class PyroFile;
class IPyroEmitter;
struct KdFileHandle;

// Inferred name. IPyroStream over an OpenKdFile handle, built on the stack
// by KdEffect::Load. Read, Write and Tell are the SQFile bodies (same
// member layout: vptr, handle).
// SIZE 0x08
// VTABLE: VC1 0x004d69a8
class KdPyroStream : public IPyroStream
{
public:
    KdFileHandle* file; // 0x04

    explicit KdPyroStream(KdFileHandle* handle) : file(handle) {}

    int Read(void* buffer, int size);
    int Write(const void* buffer, int size);
    int Seek(int offset, int origin);
    int Tell();
};

typedef char Vc1KdPyroStreamSizeCheck[sizeof(KdPyroStream) == 0x08 ? 1 : -1];

// Inferred name. Script class name "Effect" (0x004d4c18). A loaded .pyro
// file: IDisplay::Display3c creates it, ResourceManager GetEffect loads it
// and applies the PyroEffects.nut entry (BoundingRect). QI at 0x0045b650:
// IEffect this, ILoadable+4, IScriptLoadable+8, IScriptObject+0x0c,
// IAbstract+0x10.
// SIZE 0x2c
// VTABLE: VC1 0x004d6950 vc1::IEffect
// VTABLE: VC1 0x004d6960 vc1::ILoadable
// VTABLE: VC1 0x004d6970 vc1::IScriptLoadable
// VTABLE: VC1 0x004d6980 vc1::IScriptObject
// VTABLE: VC1 0x004d6998 vc1::IAbstract
class KdEffect : public IEffect,
                 public ILoadable,
                 public IScriptLoadable,
                 public IScriptObject,
                 public IAbstract
{
public:
    PyroFile* file; // 0x18
    RectF bounds;   // 0x1c  BoundingRect, default (0, 0, 1, 1)

    KdEffect();
    ~KdEffect();
    void* QueryInterface(const char** iid);                       // vtable+0x00
    unsigned AddRef();                                            // vtable+0x04
    unsigned Release();                                           // vtable+0x08
    KdPtr<IEffectEmitter> CreateEmitter(const char* name);        // IEffect vtable+0x0c
    unsigned char Load(const char* name);                         // ILoadable vtable+0x0c
    bool LoadFromScript(SquirrelObject& table);                   // IScriptLoadable vtable+0x0c
    unsigned char Script0c(const char* name, SquirrelObject* out); // IScriptObject vtable+0x0c
    unsigned char Script10(void* a, void* b);                     // IScriptObject vtable+0x10
    const char* Script14();                                       // IScriptObject vtable+0x14

    KdPtr<IEffectEmitter> CreateFirstEmitter();
    void FreeFile();

    static void operator delete(void* p) { free(p); }
};

typedef char Vc1KdEffectSizeCheck[sizeof(KdEffect) == 0x2c ? 1 : -1];

// Inferred name. Script class name "EffectEmitter" (0x004d4c08). One Pyro
// emitter instance of an effect, placed in the scene: it follows its
// positionable (or its own position), advances the emitter on Update and
// draws it on Render when the effect's BoundingRect is on screen. QI at
// 0x0045a9c0: IEffectEmitter/IRenderable this, IWithLifeTime/IUpdateable +4,
// IPositionableUser +8, IPositionable +0x0c, IVisible +0x10, IScriptObject
// +0x14, IShutdownable +0x18, IIdentifiable +0x1c, IAbstract +0x30.
// SIZE 0xa0
// VTABLE: VC1 0x004d7aa8 vc1::IEffectEmitter
// VTABLE: VC1 0x004d7ad4 vc1::IWithLifeTime
// VTABLE: VC1 0x004d7af4 vc1::IPositionableUser
// VTABLE: VC1 0x004d7b04 vc1::IPositionable
// VTABLE: VC1 0x004d7b18 vc1::IVisible
// VTABLE: VC1 0x004d7b2c vc1::IScriptObject
// VTABLE: VC1 0x004d7b44 vc1::IShutdownable
// VTABLE: VC1 0x004d7b54 vc1::IIdentifiable
// VTABLE: VC1 0x004d7b6c vc1::IAbstract
class KdEffectEmitter : public IEffectEmitter,
                        public IWithLifeTime,
                        public IPositionableUser,
                        public IPositionable,
                        public IVisible,
                        public IScriptObject,
                        public IShutdownable,
                        public IIdentifiable,
                        public IAbstract
{
public:
    IPyroEmitter* emitter;               // 0x38  owned instance
    IEffect* effect;                     // 0x3c  counted
    int lifeTime;                        // 0x40  ms since creation
    KdRef<IPositionable> positionable;   // 0x44  followed when set
    Matrix3 position;                    // 0x48
    Matrix3 localMatrix;                 // 0x6c  applied before position
    int renderLayer;                     // 0x90
    bool visible;                        // 0x94
    bool enabled;                        // 0x95  emits new particles
    bool localMode;                      // 0x96  emitter rotates with the matrix
    bool started;                        // 0x97  has had particles
    float emitterX;                      // 0x98  emitter position at creation
    float emitterY;                      // 0x9c

    KdEffectEmitter(IPyroEmitter* source, IEffect* owner);
    ~KdEffectEmitter();
    void* QueryInterface(const char** iid);                         // vtable+0x00
    unsigned AddRef();                                              // vtable+0x04
    unsigned Release();                                             // vtable+0x08
    int Render(void* graphics);                                     // IRenderable vtable+0x10
    int Render14();                                                 // IRenderable vtable+0x14
    void Reset();
    void SetRenderLayer(int layer);
    void SetEnabled(bool value);
    void SetLocalMode(bool local);
    void SetLocalMatrix(const Matrix3& matrix);
    int Unmapped0c();                                               // IUpdateable vtable+0x0c
    int SetLayer(unsigned layer);                                   // IUpdateable vtable+0x10
    void Update(unsigned sliceBit, int dt);                         // IUpdateable vtable+0x14
    int GetLifeTime();
    bool IsExpired();
    void SetPositionable(const KdPtr<IPositionable>& source);
    void SetPosition(const Matrix3& value);
    const Matrix3& GetPosition();
    void SetVisible(bool value);
    bool IsVisible();
    unsigned char Script0c(const char* name, SquirrelObject* out);
    unsigned char Script10(void* a, void* b);
    const char* Script14();
    int Shutdown();

    static void operator delete(void* p) { free(p); }
};

typedef char Vc1KdEffectEmitterSizeCheck[sizeof(KdEffectEmitter) == 0xa0 ? 1 : -1];

// Inferred name. Wraps a new emitter instance of effect (malloc(0xa0),
// constructor 0x0045b410) and returns its IAbstract face.
KdPtr<IAbstract> CreateEffectEmitter(IPyroEmitter* emitter, IEffect* effect);

// SYNTHETIC: VC1 0x0042dfb0
// vc1::KdEffectEmitter::`scalar deleting destructor'
// SYNTHETIC: VC1 0x0042e1d0
// ?QueryInterface@KdEffectEmitter@vc1@@WDA@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0042e1e0
// ??_EKdEffectEmitter@vc1@@WDA@AEPAXI@Z
// SYNTHETIC: VC1 0x0042e1f0
// ?QueryInterface@KdEffectEmitter@vc1@@WBE@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0042e200
// ?Release@KdEffectEmitter@vc1@@WBA@AEIXZ
// SYNTHETIC: VC1 0x0042e210
// ?AddRef@KdEffectEmitter@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x0042e220
// ?Release@KdEffectEmitter@vc1@@WDA@AEIXZ
// SYNTHETIC: VC1 0x0042e230
// ?QueryInterface@KdEffectEmitter@vc1@@WBI@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0042e240
// ?Release@KdEffectEmitter@vc1@@WBE@AEIXZ
// SYNTHETIC: VC1 0x0042e250
// ?AddRef@KdEffectEmitter@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x0042e260
// ?QueryInterface@KdEffectEmitter@vc1@@WBM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0042e270
// ?Release@KdEffectEmitter@vc1@@WBI@AEIXZ
// SYNTHETIC: VC1 0x0042e280
// ?AddRef@KdEffectEmitter@vc1@@WBA@AEIXZ
// SYNTHETIC: VC1 0x0042e290
// ?AddRef@KdEffectEmitter@vc1@@WDA@AEIXZ
// SYNTHETIC: VC1 0x0042e2a0
// ??_EKdEffectEmitter@vc1@@WBM@AEPAXI@Z
// SYNTHETIC: VC1 0x0042e2c0
// ?Release@KdEffectEmitter@vc1@@WBM@AEIXZ
// SYNTHETIC: VC1 0x0042e2d0
// ?AddRef@KdEffectEmitter@vc1@@WBE@AEIXZ
// SYNTHETIC: VC1 0x0042e2e0
// ?QueryInterface@KdEffectEmitter@vc1@@W3AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0042e2f0
// ?AddRef@KdEffectEmitter@vc1@@WBI@AEIXZ
// SYNTHETIC: VC1 0x0042e300
// ?QueryInterface@KdEffectEmitter@vc1@@W7AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0042e310
// ?Release@KdEffectEmitter@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x0042e320
// ?AddRef@KdEffectEmitter@vc1@@WBM@AEIXZ
// SYNTHETIC: VC1 0x0042e330
// ?QueryInterface@KdEffectEmitter@vc1@@WM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0042e340
// ?Release@KdEffectEmitter@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x0042e350
// ?QueryInterface@KdEffectEmitter@vc1@@WBA@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0042e360
// ?Release@KdEffectEmitter@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x0042e370
// ?AddRef@KdEffectEmitter@vc1@@W3AEIXZ

// SYNTHETIC: VC1 0x0041d660
// vc1::KdEffect::`scalar deleting destructor'
// SYNTHETIC: VC1 0x0041d850
// ?Release@KdEffect@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x0041d860
// ?QueryInterface@KdEffect@vc1@@W7AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0041d870
// ?Release@KdEffect@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x0041d880
// ?QueryInterface@KdEffect@vc1@@WM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0041d890
// ?AddRef@KdEffect@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x0041d8a0
// ?Release@KdEffect@vc1@@WBA@AEIXZ
// SYNTHETIC: VC1 0x0041d8b0
// ?QueryInterface@KdEffect@vc1@@WBA@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0041d8c0
// ?AddRef@KdEffect@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x0041d8d0
// ??_EKdEffect@vc1@@WBA@AEPAXI@Z
// SYNTHETIC: VC1 0x0041d8e0
// ?AddRef@KdEffect@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x0041d8f0
// ?AddRef@KdEffect@vc1@@WBA@AEIXZ
// SYNTHETIC: VC1 0x0041d900
// ?Release@KdEffect@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x0041d910
// ?QueryInterface@KdEffect@vc1@@W3AEPAXPAPBD@Z

// TEMPLATE: VC1 0x0041d750
// ??$CreateScriptMethod@VKdEffect@vc1@@P812@AE?AV?$KdPtr@UIEffectEmitter@vc1@@@2@XZ@vc1@@YA?AVSquirrelObject@@P8KdEffect@0@AE?AV?$KdPtr@UIEffectEmitter@vc1@@@0@XZ@Z

// TEMPLATE: VC1 0x0041d7c0
// ??$GetScriptValue@URectF@vc1@@@vc1@@YA?AURectF@0@ABVSquirrelObject@@@Z

// TEMPLATE: VC1 0x0042e010
// ??$CreateScriptMethod@VKdEffectEmitter@vc1@@P812@AEXXZ@vc1@@YA?AVSquirrelObject@@P8KdEffectEmitter@0@AEXXZ@Z

// TEMPLATE: VC1 0x0042e080
// ??$CreateScriptMethod@VKdEffectEmitter@vc1@@P812@AEXH@Z@vc1@@YA?AVSquirrelObject@@P8KdEffectEmitter@0@AEXH@Z@Z

// TEMPLATE: VC1 0x0042e0f0
// ??$CreateScriptMethod@VKdEffectEmitter@vc1@@P812@AEX_N@Z@vc1@@YA?AVSquirrelObject@@P8KdEffectEmitter@0@AEX_N@Z@Z

// TEMPLATE: VC1 0x0042e160
// ??$CreateScriptMethod@VKdEffectEmitter@vc1@@P812@AEXABUMatrix3@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdEffectEmitter@0@AEXABUMatrix3@0@@Z@Z

}

#endif
