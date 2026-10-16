#ifndef VC1_KD_RENDER_CLASSIFICATED_RENDERER_H
#define VC1_KD_RENDER_CLASSIFICATED_RENDERER_H

#include "kd/scene/scripted_object.h"

namespace vc1 {

// Inferred name. Factory "ClassificatedRenderer": a ScriptedObject that only
// renders when its classificator bits pass the render mask (every bit of
// mask.lo set, none of mask.hi). The HUD script classes behind it
// (CUIObjectsRenderer, CUIHintsRenderer) implement SetObjectsArray and the
// drawing; no fields of its own (malloc 0xd8 like ScriptedObject).
// SIZE 0xd8
// VTABLE: VC1 0x004cfbac vc1::IUpdateable
// VTABLE: VC1 0x004cfbc4 vc1::IRenderable
// VTABLE: VC1 0x004cfbdc vc1::IInputHandler
// VTABLE: VC1 0x004cfbf0 vc1::IStateProvider
// VTABLE: VC1 0x004cfc04 vc1::IScriptObject
// VTABLE: VC1 0x004cfc1c vc1::IScriptLoadable
// VTABLE: VC1 0x004cfc2c vc1::IPositionable
// VTABLE: VC1 0x004cfc40 vc1::IIdentifiable
// VTABLE: VC1 0x004cfc58 vc1::IClassifiable
// VTABLE: VC1 0x004cfc74 vc1::KdComponentGroup
// VTABLE: VC1 0x004cfc9c vc1::IAbstract
class KdClassificatedRenderer : public KdScriptedObject
{
public:
    KdClassificatedRenderer() throw();
    void* QueryInterface(const char** iid);
    unsigned AddRef();
    unsigned Release();
    unsigned char GetMember(const char* name, SquirrelObject* out);
    unsigned char SetMember(void* name, void* value);
    const char* GetTypeName();
    int RenderMasked(void* graphics, void* mask);
};

typedef char Vc1KdClassificatedRendererSizeCheck[sizeof(KdClassificatedRenderer) == 0xd8 ? 1 : -1];

// SYNTHETIC: VC1 0x00402050
// ?QueryInterface@KdClassificatedRenderer@vc1@@WBM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00402060
// ?Release@KdClassificatedRenderer@vc1@@WBI@AEIXZ
// SYNTHETIC: VC1 0x00402070
// ?AddRef@KdClassificatedRenderer@vc1@@WBA@AEIXZ
// SYNTHETIC: VC1 0x00402080
// ?Release@KdClassificatedRenderer@vc1@@WDI@AEIXZ
// SYNTHETIC: VC1 0x00402090
// ?AddRef@KdClassificatedRenderer@vc1@@WDA@AEIXZ
// SYNTHETIC: VC1 0x004020a0
// ?QueryInterface@KdClassificatedRenderer@vc1@@WFM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x004020b0
// ?Release@KdClassificatedRenderer@vc1@@WBM@AEIXZ
// SYNTHETIC: VC1 0x004020c0
// ?AddRef@KdClassificatedRenderer@vc1@@WBE@AEIXZ
// SYNTHETIC: VC1 0x004020d0
// ?Release@KdClassificatedRenderer@vc1@@WFM@AEIXZ
// SYNTHETIC: VC1 0x004020e0
// ?QueryInterface@KdClassificatedRenderer@vc1@@W3AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x004020f0
// ?AddRef@KdClassificatedRenderer@vc1@@WBI@AEIXZ
// SYNTHETIC: VC1 0x00402100
// ?AddRef@KdClassificatedRenderer@vc1@@WDI@AEIXZ
// SYNTHETIC: VC1 0x00402110
// ?QueryInterface@KdClassificatedRenderer@vc1@@W7AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00402120
// ?Release@KdClassificatedRenderer@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x00402130
// ?AddRef@KdClassificatedRenderer@vc1@@WBM@AEIXZ
// SYNTHETIC: VC1 0x00402140
// ?AddRef@KdClassificatedRenderer@vc1@@WFM@AEIXZ
// SYNTHETIC: VC1 0x00402150
// ?QueryInterface@KdClassificatedRenderer@vc1@@WM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00402160
// ?Release@KdClassificatedRenderer@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x00402170
// ?QueryInterface@KdClassificatedRenderer@vc1@@WBA@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00402180
// ?Release@KdClassificatedRenderer@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x00402190
// ?AddRef@KdClassificatedRenderer@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x004021a0
// ?QueryInterface@KdClassificatedRenderer@vc1@@WDA@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x004021b0
// ?QueryInterface@KdClassificatedRenderer@vc1@@WBE@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x004021c0
// ?Release@KdClassificatedRenderer@vc1@@WBA@AEIXZ
// SYNTHETIC: VC1 0x004021d0
// ?AddRef@KdClassificatedRenderer@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x004021e0
// ?Release@KdClassificatedRenderer@vc1@@WDA@AEIXZ
// SYNTHETIC: VC1 0x004021f0
// ?QueryInterface@KdClassificatedRenderer@vc1@@WBI@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00402200
// ?Release@KdClassificatedRenderer@vc1@@WBE@AEIXZ
// SYNTHETIC: VC1 0x00402210
// ?AddRef@KdClassificatedRenderer@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x00402220
// ?QueryInterface@KdClassificatedRenderer@vc1@@WDI@AEPAXPAPBD@Z

}

#endif
