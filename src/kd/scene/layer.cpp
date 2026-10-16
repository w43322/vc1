#include "kd/scene/layer.h"

#include "bindings/component.h"
#include "bindings/sqplus.h"
#include "kd/display/graphics.h"
#include "kd/iface/icomponent_faces.h"
#include "kd/iface/iid.h"

// <windows.h> maps DrawText to DrawTextA.
#undef DrawText

#include <new>
#include <stdlib.h>
#include <string.h>

namespace vc1 {

// STRING: VC1 0x004d4c20
static const char kLayer[] = "Layer";
// GLOBAL: VC1 0x004d0640
const char* g_kdNameLayer = kLayer;

// IScriptObject+0x14.
// FUNCTION: VC1 0x00418890
const char* KdLayer::GetTypeName()
{
    return kLayer;
}

// RET 4. Tail call through IGraphics+0x20.
// FUNCTION: VC1 0x004188c0
void LayerSetClip::Execute(LayerRenderContext* context)
{
    context->graphics->ptr->SetClipRect(rect);
}

// RET 4.
// FUNCTION: VC1 0x004188e0
void LayerSetColor::Execute(LayerRenderContext* context)
{
    IGraphics* const graphics = context->graphics->ptr;
    graphics->SetColor((color * reinterpret_cast<const Color&>(context->saved.color)).argb);
}

// RET 4. Tail call through IGraphics+0x30.
// FUNCTION: VC1 0x00418920
void LayerSetMode::Execute(LayerRenderContext* context)
{
    context->graphics->ptr->SetMode(mode);
}

// RET 4. Tail call through IGraphics+0x38.
// FUNCTION: VC1 0x00418940
void LayerSetFont::Execute(LayerRenderContext* context)
{
    context->graphics->ptr->SetFont(font.AsPtr());
}

// RET 4.
// FUNCTION: VC1 0x00418960
void LayerSetTransform::Execute(LayerRenderContext* context)
{
    IGraphics* const graphics = context->graphics->ptr;
    graphics->SetTransform(context->saved.transform * matrix);
}

// RET 4.
// FUNCTION: VC1 0x00418990
void LayerFillRect::Execute(LayerRenderContext* context)
{
    context->graphics->ptr->FillRect(x, y, width, height);
}

// RET 4.
// FUNCTION: VC1 0x004189d0
void LayerDrawRect::Execute(LayerRenderContext* context)
{
    context->graphics->ptr->DrawRect(x, y, width, height);
}

// RET 4.
// FUNCTION: VC1 0x00418a10
void LayerDrawImage::Execute(LayerRenderContext* context)
{
    context->graphics->ptr->DrawImage(texture.AsPtr(), x, y, u, v, width, height);
}

// RET 4.
// FUNCTION: VC1 0x00418a60
void LayerDrawQuads::Execute(LayerRenderContext* context)
{
    context->graphics->ptr->DrawQuads(texture.AsPtr(), quads, count);
}

// FUNCTION: VC1 0x00418a90
unsigned LayerDrawQuads::Size()
{
    return count * 0x18 + 0x0c;
}

// RET 4.
// FUNCTION: VC1 0x00418aa0
void LayerDrawString::Execute(LayerRenderContext* context)
{
    context->graphics->ptr->DrawString(text, x, y);
}

// FUNCTION: VC1 0x00418ad0
unsigned LayerDrawString::Size()
{
    return static_cast<unsigned>(strlen(text)) + 0x0d;
}

// RET 4.
// FUNCTION: VC1 0x00418af0
void LayerDrawLine::Execute(LayerRenderContext* context)
{
    context->graphics->ptr->DrawLine(x0, y0, x1, y1);
}

// RET 4.
// FUNCTION: VC1 0x00418b30
void LayerDrawEllipse::Execute(LayerRenderContext* context)
{
    context->graphics->ptr->DrawEllipse(x, y, width, height);
}

// FUNCTION: VC1 0x00418c40 FOLDED
unsigned LayerSetColor::Size()
{
    return 8;
}

// FUNCTION: VC1 0x00418c40 FOLDED
unsigned LayerSetMode::Size()
{
    return 8;
}

// FUNCTION: VC1 0x00418c40 FOLDED
unsigned LayerSetFont::Size()
{
    return 8;
}

// FUNCTION: VC1 0x00418c50
unsigned LayerSetTransform::Size()
{
    return 0x28;
}

// FUNCTION: VC1 0x00418c60 FOLDED
unsigned LayerSetClip::Size()
{
    return 0x14;
}

// FUNCTION: VC1 0x00418c60 FOLDED
unsigned LayerFillRect::Size()
{
    return 0x14;
}

// FUNCTION: VC1 0x00418c60 FOLDED
unsigned LayerDrawRect::Size()
{
    return 0x14;
}

// FUNCTION: VC1 0x00418c60 FOLDED
unsigned LayerDrawLine::Size()
{
    return 0x14;
}

// FUNCTION: VC1 0x00418c60 FOLDED
unsigned LayerDrawEllipse::Size()
{
    return 0x14;
}

// FUNCTION: VC1 0x00418c70
unsigned LayerDrawImage::Size()
{
    return 0x20;
}

// IRenderable+0x14.
// FUNCTION: VC1 0x0041adb0 FOLDED
int KdLayer::GetRenderOrder()
{
    return 0;
}

// IScriptObject+0x10. RET 8.
// FUNCTION: VC1 0x004688d0
unsigned char KdLayer::SetMember(void*, void*)
{
    return 0;
}

// FUNCTION: VC1 0x004688e0
Matrix3& KdLayer::GetTransform()
{
    return state.transform;
}

// FUNCTION: VC1 0x004688f0
IUnknown** KdLayer::GetFont()
{
    return &state.font;
}

// FUNCTION: VC1 0x00468900
unsigned KdLayer::GetMode()
{
    return state.mode;
}

// FUNCTION: VC1 0x00468910
unsigned KdLayer::GetColor()
{
    return state.color;
}

// FUNCTION: VC1 0x00468920
Rect& KdLayer::GetClipRect()
{
    return state.clip;
}

// A layer has no view of its own: an unbounded one.
// FUNCTION: VC1 0x00468930
Rect* KdLayer::GetView()
{
    // GLOBAL: VC1 0x004ee28c
    static Rect view(0, 0, 0x7fffffff, 0x7fffffff);
    return &view;
}

// No camera: a null slot.
// FUNCTION: VC1 0x00468970
IUnknown** KdLayer::GetCamera()
{
    // GLOBAL: VC1 0x004ee2a0
    static KdRef<IUnknown> camera;
    return &camera.ptr;
}

// ECX=this. Deleting destructor through IAbstract on zero.
// FUNCTION: VC1 0x004689f0
unsigned KdLayer::Release()
{
    IAbstract* const self = static_cast<IAbstract*>(this);
    unsigned value = --self->refs;
    if (!value) {
        delete self;
        return 0;
    }
    return value;
}

// FUNCTION: VC1 0x00468a10
unsigned KdLayer::AddRef()
{
    unsigned value = refs;
    if (value != 0) {
        ++value;
        refs = value;
    }
    return value;
}

// JMP 0x00418830.
// FUNCTION: VC1 0x00468a20
void KdLayer::Translate(const Vector2& offset)
{
    IGraphics::Translate(offset);
}

// RET 4. IRenderable+0x10: replays the recording onto the target, then puts
// the target's state back.
// FUNCTION: VC1 0x00468a30
int KdLayer::Render(void* graphics)
{
    LayerRenderContext context(static_cast<const KdPtr<IGraphics>*>(graphics));
    unsigned offset;

    for (offset = 0; offset < commands.size;) {
        LayerCommand* const command = reinterpret_cast<LayerCommand*>(
            reinterpret_cast<unsigned char*>(commands.begin) + offset);
        offset += command->Size();
        command->Execute(&context);
    }
    context.graphics->ptr->SetState(&context.saved);
    return 0;
}

// The origin row of the transform (through the virtual getter).
// FUNCTION: VC1 0x00468af0
Vector2& KdLayer::GetOrigin()
{
    return *reinterpret_cast<Vector2*>(&GetTransform()._31);
}

// RET 4. The color is ignored: drops the recording and resets the state.
// FUNCTION: VC1 0x00468b00
void KdLayer::Clear(unsigned)
{
    unsigned offset;

    state = GraphicsState();
    for (offset = 0; offset < commands.size;) {
        LayerCommand* const command = reinterpret_cast<LayerCommand*>(
            reinterpret_cast<unsigned char*>(commands.begin) + offset);
        offset += command->Size();
        command->~LayerCommand();
    }
    commands.ResizeBytes(0);
}

// ESI=this.
// FUNCTION: VC1 0x00468bd0
KdLayer::~KdLayer() throw()
{
    Clear(0);
}

// ESI=this. Reserves 0x40 bytes of commands.
// FUNCTION: VC1 0x00468c40
KdLayer::KdLayer() throw()
{
    if (commands.capacity < commands.Count() + 0x40) {
        GrowCountedBuffer(&commands, 0x40, 1);
    }
}

// FUNCTION: VC1 0x00468ce0
void** CreateKdLayer(void** out)
{
    void* const memory = malloc(sizeof(KdLayer));
    IAbstract* object = 0;
    IUnknown* queried;

    if (memory) {
        object = new (memory) KdLayer();
    }
    queried = object ? static_cast<IUnknown*>(object->QueryInterface(&g_iidAbstract)) : 0;
    *out = queried;
    if (queried) {
        queried->AddRef();
    }
    static_cast<IUnknown*>(*out)->Release();
    return out;
}

// RET 0x10.
// FUNCTION: VC1 0x00468d30
void KdLayer::DrawEllipse(float x, float y, float width, float height)
{
    LayerDrawEllipse* const command = new (AppendCommand(sizeof(LayerDrawEllipse))) LayerDrawEllipse;
    command->x = x;
    command->y = y;
    command->width = width;
    command->height = height;
}

// RET 0x10.
// FUNCTION: VC1 0x00468d90
void KdLayer::DrawLine(float x0, float y0, float x1, float y1)
{
    LayerDrawLine* const command = new (AppendCommand(sizeof(LayerDrawLine))) LayerDrawLine;
    command->x0 = x0;
    command->y0 = y0;
    command->x1 = x1;
    command->y1 = y1;
}

// RET 0xc. The text is copied after the command.
// FUNCTION: VC1 0x00468df0
void KdLayer::DrawString(const char* text, float x, float y)
{
    const unsigned length = static_cast<unsigned>(strlen(text)) + 1;
    LayerDrawString* const command = new (AppendCommand(length + 0x0c)) LayerDrawString;
    command->x = x;
    command->y = y;
    memcpy(command->text, text, length);
}

// RET 0xc. The quads are copied after the command.
// FUNCTION: VC1 0x00468e70
void KdLayer::DrawQuads(const KdPtr<ITexture>& texture, const void* quads, unsigned count)
{
    LayerDrawQuads* const command = new (AppendCommand(count * 0x18 + 0x0c)) LayerDrawQuads;
    command->texture.Assign(texture);
    command->count = count;
    memcpy(command->quads, quads, count * 0x18);
}

// RET 0x1c.
// FUNCTION: VC1 0x00468f00
void KdLayer::DrawImage(const KdPtr<ITexture>& texture, float x, float y, float u, float v,
    float width, float height)
{
    LayerDrawImage* const command = new (AppendCommand(sizeof(LayerDrawImage))) LayerDrawImage;
    command->texture.Assign(texture);
    command->x = x;
    command->y = y;
    command->u = u;
    command->v = v;
    command->width = width;
    command->height = height;
}

// RET 0x10.
// FUNCTION: VC1 0x00468f90
void KdLayer::DrawRect(float x, float y, float width, float height)
{
    LayerDrawRect* const command = new (AppendCommand(sizeof(LayerDrawRect))) LayerDrawRect;
    command->x = x;
    command->y = y;
    command->width = width;
    command->height = height;
}

// RET 0x10.
// FUNCTION: VC1 0x00468ff0
void KdLayer::FillRect(float x, float y, float width, float height)
{
    LayerFillRect* const command = new (AppendCommand(sizeof(LayerFillRect))) LayerFillRect;
    command->x = x;
    command->y = y;
    command->width = width;
    command->height = height;
}

// RET 4. The setters keep the layer's own state too (for the getters).
// FUNCTION: VC1 0x00469050
void KdLayer::SetTransform(const Matrix3& matrix)
{
    state.transform = matrix;
    LayerSetTransform* const command =
        new (AppendCommand(sizeof(LayerSetTransform))) LayerSetTransform;
    command->matrix = matrix;
}

// RET 4.
// FUNCTION: VC1 0x004690e0
void KdLayer::SetFont(const KdPtr<IFont>& font)
{
    if (font.ptr) {
        font.ptr->AddRef();
    }
    if (state.font) {
        state.font->Release();
    }
    state.font = font.ptr;
    LayerSetFont* const command = new (AppendCommand(sizeof(LayerSetFont))) LayerSetFont;
    command->font.Assign(font);
}

// RET 4.
// FUNCTION: VC1 0x00469180
void KdLayer::SetMode(unsigned mode)
{
    state.mode = mode;
    LayerSetMode* const command = new (AppendCommand(sizeof(LayerSetMode))) LayerSetMode;
    command->mode = mode;
}

// RET 4.
// FUNCTION: VC1 0x004691d0
void KdLayer::SetColor(unsigned color)
{
    state.color = color;
    LayerSetColor* const command = new (AppendCommand(sizeof(LayerSetColor))) LayerSetColor;
    command->color.argb = color;
}

// RET 4.
// FUNCTION: VC1 0x00469230
void KdLayer::SetClipRect(const Rect& rect)
{
    state.clip = rect;
    LayerSetClip* const command = new (AppendCommand(sizeof(LayerSetClip))) LayerSetClip;
    command->rect = rect;
}

// RET 8. this = IScriptObject (+0x48); the same script names as KdGraphics,
// every closure calls through the IGraphics vtable (vcall thunks).
// FUNCTION: VC1 0x00469e10
unsigned char KdLayer::GetMember(const char* name, SquirrelObject* out)
{
    // GLOBAL: VC1 0x004ee704
    static ScriptMethodTable methods;

    if (methods.Empty()) {
        methods[kScriptGetClip] = CreateScriptMethod<KdLayer>(&KdLayer::GetClipRect);
        methods[kScriptSetClip] = CreateScriptMethod<KdLayer>(&KdLayer::SetClipRect);
        methods[kScriptGetColor] = CreateScriptMethod<KdLayer>(&KdLayer::GetColor);
        methods[kScriptSetColor] = CreateScriptMethod<KdLayer>(&KdLayer::SetColor);
        methods[kScriptGetBlendMode] = CreateScriptMethod<KdLayer>(&KdLayer::GetMode);
        methods[kScriptSetBlendMode] = CreateScriptMethod<KdLayer>(&KdLayer::SetMode);
        methods[kScriptGetFont] = CreateScriptMethod<KdLayer>(&KdLayer::GetFont);
        methods[kScriptSetFont] = CreateScriptMethod<KdLayer>(&KdLayer::SetFont);
        methods[kScriptClear] = CreateScriptMethod<KdLayer>(&KdLayer::Clear);
        methods[kScriptGetCamera] = CreateScriptMethod<KdLayer>(&KdLayer::GetCamera);
        methods[kScriptGetTransform] = CreateScriptMethod<KdLayer>(&KdLayer::GetTransform);
        methods[kScriptSetTransform] = CreateScriptMethod<KdLayer>(&KdLayer::SetTransform);
        methods[kScriptGetTranslate] = CreateScriptMethod<KdLayer>(&KdLayer::GetOrigin);
        methods[kScriptTranslate] = CreateScriptMethod<KdLayer>(&KdLayer::Translate);
        methods[kScriptDrawRect] = CreateScriptMethod<KdLayer>(&KdLayer::DrawRect);
        methods[kScriptFillRect] = CreateScriptMethod<KdLayer>(&KdLayer::FillRect);
        methods[kScriptDrawTexture] = CreateScriptMethod<KdLayer>(&KdLayer::DrawImage);
        methods[kScriptDrawString] = CreateScriptMethod<KdLayer>(&KdLayer::DrawString);
        methods[kScriptDrawLine] = CreateScriptMethod<KdLayer>(&KdLayer::DrawLine);
        methods[kScriptDrawEllipse] = CreateScriptMethod<KdLayer>(&KdLayer::DrawEllipse);
    }
    return methods.Find(name, out);
}

}
