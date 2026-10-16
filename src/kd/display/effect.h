#ifndef VC1_KD_EFFECT_H
#define VC1_KD_EFFECT_H

#include "kd/iface/ieffect.h"
#include "kd/iface/iloadable.h"
#include "kd/iface/icomponent_faces.h"
#include "kd/iface/iscript_object.h"
#include "kd/iface/iabstract.h"
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

// Inferred name. Wraps a new emitter instance of effect (malloc(0xa0),
// constructor 0x0045b410) and returns its IAbstract face.
KdPtr<IAbstract> CreateEffectEmitter(IPyroEmitter* emitter, IEffect* effect);

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

// TEMPLATE: VC1 0x0041d7c0
// ??$GetScriptValue@URectF@vc1@@@vc1@@YA?AURectF@0@ABVSquirrelObject@@@Z

}

#endif
