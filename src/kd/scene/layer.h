#ifndef VC1_KD_SCENE_LAYER_H
#define VC1_KD_SCENE_LAYER_H

#include "bindings/color.h"
#include "kd/factory.h"
#include "kd/iface/iabstract.h"
#include "kd/iface/ifont.h"
#include "kd/iface/igraphics.h"
#include "kd/iface/irenderable.h"
#include "kd/iface/iscript_object.h"

class SquirrelObject;

namespace vc1 {

// Inferred name. What a recorded command draws on: the IGraphics the layer is
// rendered to and its state on entry (colors and transforms are relative to it).
// SIZE 0x44
struct LayerRenderContext
{
    const KdPtr<IGraphics>* graphics; // 0x00
    GraphicsState saved;              // 0x04

    explicit __forceinline LayerRenderContext(const KdPtr<IGraphics>* target)
        : graphics(target), saved(*target->ptr->GetState())
    {
    }
};

// Inferred name. One recorded IGraphics call, stored inline in the layer's
// byte buffer; Size() is the bytes it takes there (some are variable-length).
// VTABLE: VC1 0x004d5a4c
class LayerCommand
{
public:
    virtual ~LayerCommand() {}                                 // vtable+0x00
    virtual void Execute(LayerRenderContext* context) = 0;     // vtable+0x04
    virtual unsigned Size() = 0;                               // vtable+0x08
};

// VTABLE: VC1 0x004d5a58
class LayerSetClip : public LayerCommand
{
public:
    Rect rect; // 0x04
    void Execute(LayerRenderContext* context);
    unsigned Size();
};

// The color is modulated by the target's color on entry.
// VTABLE: VC1 0x004d5a64
class LayerSetColor : public LayerCommand
{
public:
    Color color; // 0x04
    void Execute(LayerRenderContext* context);
    unsigned Size();
};

// VTABLE: VC1 0x004d5a70
class LayerSetMode : public LayerCommand
{
public:
    unsigned mode; // 0x04
    void Execute(LayerRenderContext* context);
    unsigned Size();
};

// VTABLE: VC1 0x004d5a7c
class LayerSetFont : public LayerCommand
{
public:
    KdRef<IFont> font; // 0x04
    void Execute(LayerRenderContext* context);
    unsigned Size();
};

// The matrix is applied after the target's transform on entry.
// VTABLE: VC1 0x004d5a88
class LayerSetTransform : public LayerCommand
{
public:
    Matrix3 matrix; // 0x04
    void Execute(LayerRenderContext* context);
    unsigned Size();
};

// VTABLE: VC1 0x004d5a94
class LayerFillRect : public LayerCommand
{
public:
    float x, y, width, height; // 0x04
    void Execute(LayerRenderContext* context);
    unsigned Size();
};

// VTABLE: VC1 0x004d5aa0
class LayerDrawRect : public LayerCommand
{
public:
    float x, y, width, height; // 0x04
    void Execute(LayerRenderContext* context);
    unsigned Size();
};

// VTABLE: VC1 0x004d5aac
class LayerDrawImage : public LayerCommand
{
public:
    KdRef<ITexture> texture;          // 0x04
    float x, y, u, v, width, height; // 0x08
    void Execute(LayerRenderContext* context);
    unsigned Size();
};

// Followed by count 0x18-byte quads.
// VTABLE: VC1 0x004d5ab8
class LayerDrawQuads : public LayerCommand
{
public:
    KdRef<ITexture> texture; // 0x04
    unsigned count;          // 0x08
    unsigned char quads[1];  // 0x0c
    void Execute(LayerRenderContext* context);
    unsigned Size();
};

// Followed by the NUL-terminated text.
// VTABLE: VC1 0x004d5ac4
class LayerDrawString : public LayerCommand
{
public:
    float x, y;   // 0x04
    char text[1]; // 0x0c
    void Execute(LayerRenderContext* context);
    unsigned Size();
};

// VTABLE: VC1 0x004d5ad0
class LayerDrawLine : public LayerCommand
{
public:
    float x0, y0, x1, y1; // 0x04
    void Execute(LayerRenderContext* context);
    unsigned Size();
};

// VTABLE: VC1 0x004d5adc
class LayerDrawEllipse : public LayerCommand
{
public:
    float x, y, width, height; // 0x04
    void Execute(LayerRenderContext* context);
    unsigned Size();
};

// Inferred name. Factory "Layer" (menus' getComponent("Layer"), e.g.
// HintBase.nut): an IGraphics that records every call as a LayerCommand and
// replays them onto another IGraphics when rendered (script renderComponent).
// Clear drops the recording. QI 0x004689a0 establishes the face offsets.
// SIZE 0x64
// VTABLE: VC1 0x004d59a0 vc1::IGraphics
// VTABLE: VC1 0x004d5a0c vc1::IRenderable
// VTABLE: VC1 0x004d5a24 vc1::IScriptObject
// VTABLE: VC1 0x004d5a3c vc1::IAbstract
class KdLayer : public IGraphics, public IRenderable, public IScriptObject, public IAbstract
{
public:
    KdFactory commands; // 0x54 recorded LayerCommands, back to back

    KdLayer() throw();
    ~KdLayer() throw();
    void* QueryInterface(const char** iid);
    unsigned AddRef();
    unsigned Release();

    IUnknown** GetCamera();
    Rect* GetView();
    Rect& GetClipRect();
    void SetClipRect(const Rect& rect);
    unsigned GetColor();
    void SetColor(unsigned color);
    int GetMode();
    void SetMode(int mode);
    IFont** GetFont();
    void SetFont(const KdPtr<IFont>& font);
    void Clear(unsigned color);
    Matrix3& GetTransform();
    void SetTransform(const Matrix3& matrix);
    Vector2& GetOrigin();
    void Translate(const Vector2& offset);
    void FillRect(float x, float y, float width, float height);
    void DrawRect(float x, float y, float width, float height);
    void DrawImage(const KdPtr<ITexture>& texture, float x, float y, float u, float v,
        float width, float height);
    void DrawQuads(const KdPtr<ITexture>& texture, const void* quads, unsigned count);
    void DrawString(const char* text, float x, float y);
    void DrawLine(float x0, float y0, float x1, float y1);
    void DrawEllipse(float x, float y, float width, float height);

    int Render(void* graphics);
    int GetRenderOrder();

    unsigned char GetMember(const char* name, SquirrelObject* out);
    unsigned char SetMember(void* name, void* value);
    const char* GetTypeName();

private:
    // Inlined into every recorder: room for one command at the end.
    void* AppendCommand(unsigned bytes)
    {
        const unsigned size = commands.size + bytes;
        if (commands.capacity < size) {
            GrowCountedBuffer(&commands, static_cast<int>(size), 1);
        }
        commands.size = size;
        return reinterpret_cast<unsigned char*>(commands.begin) + size - bytes;
    }
};

typedef char Vc1KdLayerSizeCheck[sizeof(KdLayer) == 0x64 ? 1 : -1];

void** CreateKdLayer(void** out);

// TEMPLATE: VC1 0x00418c80
// ??$CreateScriptMethod@VKdLayer@vc1@@P812@AEAAURect@2@XZ@vc1@@YA?AVSquirrelObject@@P8KdLayer@0@AEAAURect@0@XZ@Z
// TEMPLATE: VC1 0x00418cf0
// ??$CreateScriptMethod@VKdLayer@vc1@@P812@AEXABURect@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdLayer@0@AEXABURect@0@@Z@Z
// TEMPLATE: VC1 0x00418d60
// ??$CreateScriptMethod@VKdLayer@vc1@@P812@AEIXZ@vc1@@YA?AVSquirrelObject@@P8KdLayer@0@AEIXZ@Z
// TEMPLATE: VC1 0x00418dd0
// ??$CreateScriptMethod@VKdLayer@vc1@@P812@AEXI@Z@vc1@@YA?AVSquirrelObject@@P8KdLayer@0@AEXI@Z@Z
// TEMPLATE: VC1 0x00418f20
// ??$CreateScriptMethod@VKdLayer@vc1@@P812@AEPAPAUIFont@2@XZ@vc1@@YA?AVSquirrelObject@@P8KdLayer@0@AEPAPAUIFont@0@XZ@Z
// TEMPLATE: VC1 0x00418f90
// ??$CreateScriptMethod@VKdLayer@vc1@@P812@AEXABV?$KdPtr@UIFont@vc1@@@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdLayer@0@AEXABV?$KdPtr@UIFont@vc1@@@0@@Z@Z
// TEMPLATE: VC1 0x00419000
// ??$CreateScriptMethod@VKdLayer@vc1@@P812@AEPAPAUIUnknown@2@XZ@vc1@@YA?AVSquirrelObject@@P8KdLayer@0@AEPAPAUIUnknown@0@XZ@Z
// TEMPLATE: VC1 0x00419070
// ??$CreateScriptMethod@VKdLayer@vc1@@P812@AEAAUMatrix3@2@XZ@vc1@@YA?AVSquirrelObject@@P8KdLayer@0@AEAAUMatrix3@0@XZ@Z
// TEMPLATE: VC1 0x004190e0
// ??$CreateScriptMethod@VKdLayer@vc1@@P812@AEXABUMatrix3@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdLayer@0@AEXABUMatrix3@0@@Z@Z
// TEMPLATE: VC1 0x00419150
// ??$CreateScriptMethod@VKdLayer@vc1@@P812@AEAAUVector2@2@XZ@vc1@@YA?AVSquirrelObject@@P8KdLayer@0@AEAAUVector2@0@XZ@Z
// TEMPLATE: VC1 0x004191c0
// ??$CreateScriptMethod@VKdLayer@vc1@@P812@AEXABUVector2@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdLayer@0@AEXABUVector2@0@@Z@Z
// TEMPLATE: VC1 0x00419230
// ??$CreateScriptMethod@VKdLayer@vc1@@P812@AEXMMMM@Z@vc1@@YA?AVSquirrelObject@@P8KdLayer@0@AEXMMMM@Z@Z
// TEMPLATE: VC1 0x004192a0
// ??$CreateScriptMethod@VKdLayer@vc1@@P812@AEXABV?$KdPtr@UITexture@vc1@@@2@MMMMMM@Z@vc1@@YA?AVSquirrelObject@@P8KdLayer@0@AEXABV?$KdPtr@UITexture@vc1@@@0@MMMMMM@Z@Z
// TEMPLATE: VC1 0x00419310
// ??$CreateScriptMethod@VKdLayer@vc1@@P812@AEXPBDMM@Z@vc1@@YA?AVSquirrelObject@@P8KdLayer@0@AEXPBDMM@Z@Z
// TEMPLATE: VC1 0x00419380 FOLDED
// ??$Call@VKdLayer@vc1@@MMMM@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdLayer@vc1@@P823@AEXMMMM@ZPAUSQVM@@H@Z
// TEMPLATE: VC1 0x00419380 FOLDED
// ??$Call@VKdGraphics@vc1@@MMMM@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdGraphics@vc1@@P823@AEXMMMM@ZPAUSQVM@@H@Z
// TEMPLATE: VC1 0x00419460 FOLDED
// ??$Call@VKdLayer@vc1@@ABV?$KdPtr@UITexture@vc1@@@2@MMMMMM@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdLayer@vc1@@P823@AEXABV?$KdPtr@UITexture@vc1@@@3@MMMMMM@ZPAUSQVM@@H@Z
// TEMPLATE: VC1 0x00419460 FOLDED
// ??$Call@VKdGraphics@vc1@@ABV?$KdPtr@UITexture@vc1@@@2@MMMMMM@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdGraphics@vc1@@P823@AEXABV?$KdPtr@UITexture@vc1@@@3@MMMMMM@ZPAUSQVM@@H@Z
// TEMPLATE: VC1 0x004692b0
// ?Dispatch@?$ScriptMethodCall@VKdLayer@vc1@@P812@AEXABV?$KdPtr@UITexture@vc1@@@2@MMMMMM@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x00469350
// ?Dispatch@?$ScriptMethodCall@VKdLayer@vc1@@P812@AEXMMMM@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004693f0
// ?Dispatch@?$ScriptMethodCall@VKdLayer@vc1@@P812@AEXABUVector2@2@@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004694b0
// ?Dispatch@?$ScriptMethodCall@VKdLayer@vc1@@P812@AEXABUMatrix3@2@@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x00469570
// ?Dispatch@?$ScriptMethodCall@VKdLayer@vc1@@P812@AEXABV?$KdPtr@UIFont@vc1@@@2@@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x00469610
// ?Dispatch@?$ScriptMethodCall@VKdLayer@vc1@@P812@AEXH@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x00469720
// ?Dispatch@?$ScriptMethodCall@VKdLayer@vc1@@P812@AEHXZ@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004697f0
// ?Dispatch@?$ScriptMethodCall@VKdLayer@vc1@@P812@AEXI@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x00469890
// ?Dispatch@?$ScriptMethodCall@VKdLayer@vc1@@P812@AEIXZ@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x00469960
// ?Dispatch@?$ScriptMethodCall@VKdLayer@vc1@@P812@AEXABURect@2@@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x00469a20
// ?Dispatch@?$ScriptMethodCall@VKdLayer@vc1@@P812@AEXPBDMM@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x00469ac0
// ?Dispatch@?$ScriptMethodCall@VKdLayer@vc1@@P812@AEPAPAUIFont@2@XZ@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x00469b60
// ?Dispatch@?$ScriptMethodCall@VKdLayer@vc1@@P812@AEPAPAUIUnknown@2@XZ@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x00469c00
// ?Dispatch@?$ScriptMethodCall@VKdLayer@vc1@@P812@AEAAUVector2@2@XZ@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x00469ca0
// ?Dispatch@?$ScriptMethodCall@VKdLayer@vc1@@P812@AEAAURect@2@XZ@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x00469d70
// ?Dispatch@?$ScriptMethodCall@VKdLayer@vc1@@P812@AEAAUMatrix3@2@XZ@vc1@@SAHPAUSQVM@@@Z

// SYNTHETIC: VC1 0x004188a0
// vc1::KdLayer::`scalar deleting destructor'
// SYNTHETIC: VC1 0x00418be0 FOLDED
// vc1::LayerSetClip::`scalar deleting destructor'
// SYNTHETIC: VC1 0x00418be0 FOLDED
// vc1::LayerSetColor::`scalar deleting destructor'
// SYNTHETIC: VC1 0x00418be0 FOLDED
// vc1::LayerSetMode::`scalar deleting destructor'
// SYNTHETIC: VC1 0x00418be0 FOLDED
// vc1::LayerSetTransform::`scalar deleting destructor'
// SYNTHETIC: VC1 0x00418be0 FOLDED
// vc1::LayerFillRect::`scalar deleting destructor'
// SYNTHETIC: VC1 0x00418be0 FOLDED
// vc1::LayerDrawRect::`scalar deleting destructor'
// SYNTHETIC: VC1 0x00418be0 FOLDED
// vc1::LayerDrawString::`scalar deleting destructor'
// SYNTHETIC: VC1 0x00418be0 FOLDED
// vc1::LayerDrawLine::`scalar deleting destructor'
// SYNTHETIC: VC1 0x00418be0 FOLDED
// vc1::LayerDrawEllipse::`scalar deleting destructor'
// SYNTHETIC: VC1 0x00418c00 FOLDED
// vc1::LayerSetFont::`scalar deleting destructor'
// SYNTHETIC: VC1 0x00418c00 FOLDED
// vc1::LayerDrawImage::`scalar deleting destructor'
// SYNTHETIC: VC1 0x00418c00 FOLDED
// vc1::LayerDrawQuads::`scalar deleting destructor'
// SYNTHETIC: VC1 0x00419640
// ?Release@KdLayer@vc1@@WEE@AEIXZ
// SYNTHETIC: VC1 0x00419650
// ?QueryInterface@KdLayer@vc1@@WEE@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00419670
// ?Release@KdLayer@vc1@@WEI@AEIXZ
// SYNTHETIC: VC1 0x00419680
// ?QueryInterface@KdLayer@vc1@@WEI@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x004196b0
// ?Release@KdLayer@vc1@@WEM@AEIXZ
// SYNTHETIC: VC1 0x004196c0
// ?QueryInterface@KdLayer@vc1@@WEM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x004196d0
// ?AddRef@KdLayer@vc1@@WEE@AEIXZ
// SYNTHETIC: VC1 0x004196f0
// ??_EKdLayer@vc1@@WEM@AEPAXI@Z
// SYNTHETIC: VC1 0x00419700
// ?AddRef@KdLayer@vc1@@WEI@AEIXZ
// SYNTHETIC: VC1 0x00419720
// ?AddRef@KdLayer@vc1@@WEM@AEIXZ

}

#endif
