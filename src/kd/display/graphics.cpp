#include "kd/display/graphics.h"
#include "kd/display/display.h"

#include "bindings/component.h"
#include "bindings/sqplus.h"
#include "kd/iface/icomponent_faces.h"

// <windows.h> (via host/runtime.h) maps DrawText to DrawTextA.
#undef DrawText

#include "kd/iface/iid.h"
#include "kd/iface/ifont.h"

#include <string.h>

namespace vc1 {

// STRING: VC1 0x004d4c40
static const char kGraphics[] = "Graphics";

// LEA EAX, [ECX+4]; RET. IGraphics vtable+0x0c.
// FUNCTION: VC1 0x00401000
GraphicsState* IGraphics::GetState()
{
    return &state;
}

// RET 4. Re-applies a whole state block through the virtual setters.
// FUNCTION: VC1 0x004187e0
void IGraphics::SetState(const GraphicsState* value)
{
    SetClipRect(value->clip);
    SetColor(value->color);
    SetMode(value->mode);
    SetTransform(value->transform);
    SetFont(*reinterpret_cast<const KdPtr<IFont>*>(&value->font));
}

// RET 4. Identity with origin = offset, folded into state.transform in place by
// 0x00473cd0 (EAX=transform, ECX=local), then SetTransform(transform).
// FUNCTION: VC1 0x00418830
void IGraphics::Translate(const Vector2& offset)
{
    Matrix3 move(offset);

    SetTransform(*state.transform.LeftMultiply(&move));
}

// MOV EAX, 0x004d4c40; RET. IScriptObject vtable+0x14.
// FUNCTION: VC1 0x0041c8d0
const char* KdGraphics::GetTypeName()
{
    return kGraphics;
}

KdGraphics::~KdGraphics()
{
    if (camera) {
        camera->Release();
    }
    camera = 0;
}

// RET 0x10. Script "DrawRect"; draws through vtable+0x64. Body not recovered.
// STUB: VC1 0x0045c610
void KdGraphics::DrawRect(float x, float y, float width, float height)
{
    (void)x;
    (void)y;
    (void)width;
    (void)height;
}

// LEA EAX, [ECX+0x54]; RET.
// FUNCTION: VC1 0x0045c6d0
Rect* KdGraphics::GetView()
{
    return &view;
}

// LEA EAX, [ECX+0x50]; RET.
// FUNCTION: VC1 0x0045c6e0
IUnknown** KdGraphics::GetCamera()
{
    return &camera;
}

// XOR AL, AL; RET 8. IScriptObject vtable+0x10.
// FUNCTION: VC1 0x0045c6f0
unsigned char KdGraphics::SetMember(void* a, void* b)
{
    (void)a;
    (void)b;
    return 0;
}

// RET 0x10. Script "DrawEllipse" (four floats, the same Call as DrawRect).
// Body not recovered.
// STUB: VC1 0x0045c700
void KdGraphics::DrawEllipse(float x, float y, float width, float height)
{
    (void)x;
    (void)y;
    (void)width;
    (void)height;
}

// RET 0x10. Script "DrawLine". Body not recovered.
// STUB: VC1 0x0045c810
void KdGraphics::DrawLine(float x0, float y0, float x1, float y1)
{
    (void)x0;
    (void)y0;
    (void)x1;
    (void)y1;
}

// RET 0xc. Inferred name. Without state.font (+0x40) nothing is drawn.
// Saves the transform, translates to (x, y), hands the IAbstract face to
// IFont::DrawText and restores the transform.
// FUNCTION: VC1 0x0045c850
void KdGraphics::DrawString(const char* text, float x, float y)
{
    Vector2 origin;
    IFont* font;
    IUnknown* face;
    IUnknown* self;

    if (!state.font) {
        return;
    }
    Matrix3 saved(GetTransform());
    origin.x = x;
    origin.y = y;
    Translate(origin);
    font = static_cast<IFont*>(state.font);
    face = static_cast<IUnknown*>(static_cast<IAbstract*>(this)->QueryInterface(
        &g_iidAbstract));
    if (!face) {
        self = 0;
    }
    else {
        self = static_cast<IUnknown*>(face->QueryInterface(&g_iidAbstract));
        if (self) {
            self->AddRef();
        }
    }
    font->DrawText(&self, text);
    if (self) {
        self->Release();
    }
    SetTransform(saved);
}

// RET 0xc. Inferred name. Copies count prebuilt quads (four 0x18-byte
// vertices each) into the vertex batch, starting a new batch whenever the
// current one is full. With count 0 the original still stores 1 into
// g_batchCount.
// FUNCTION: VC1 0x0045c910
void KdGraphics::DrawQuads(const KdPtr<ITexture>& texture, const void* quads, unsigned count)
{
    IUnknown* const native = texture.ptr ? texture.ptr->native : 0;
    const unsigned char* source = static_cast<const unsigned char*>(quads);
    unsigned char* target = 0;
    int capacity = 0;
    int written = 1;
    unsigned i;

    for (i = 0; i < count; i++) {
        if (written >= capacity) {
            if (i > 0) {
                g_batchCount = written;
            }
            target = static_cast<unsigned char*>(BeginQuadBatch(state.mode, native, &capacity));
            written = 0;
        }
        memcpy(target, source, 0x60);
        source += 0x60;
        target += 0x60;
        written++;
    }
    if (count == 0 || written != 0) {
        g_batchCount = written;
    }
}

// RET 4. REP MOVSD 9 dwords into state.transform, then 0x00464710 (EAX=matrix).
// FUNCTION: VC1 0x0045c9a0
void KdGraphics::SetTransform(const Matrix3& matrix)
{
    state.transform = matrix;
    DisplayD3dSetView(&matrix);
}

// LEA EAX, [ECX+0x1c]; RET.
// FUNCTION: VC1 0x0045c9c0
Matrix3& KdGraphics::GetTransform()
{
    return state.transform;
}

// RET 4. IDirect3DDevice8::Clear (vtable+0x90): 0 rects, D3DCLEAR_TARGET, Z=1.
// FUNCTION: VC1 0x0045c9d0
void KdGraphics::Clear(unsigned color)
{
    typedef int (__stdcall *FnClear)(IUnknown* device, unsigned count, void* rects,
        unsigned flags, unsigned value, float z, unsigned stencil);

    reinterpret_cast<FnClear>((*reinterpret_cast<void***>(g_d3dDevice))[0x24])(
        g_d3dDevice, 0, 0, 1, color, 1.0f, 0);
}

// RET 4. AddRef new, Release old, store at +0x40.
// FUNCTION: VC1 0x0045ca00
void KdGraphics::SetFont(const KdPtr<IFont>& font)
{
    if (font.ptr) {
        font.ptr->AddRef();
    }
    if (state.font) {
        state.font->Release();
    }
    state.font = font.ptr;
}

// LEA EAX, [ECX+0x40]; RET.
// FUNCTION: VC1 0x0045ca40
IUnknown** KdGraphics::GetFont()
{
    return &state.font;
}

// FUNCTION: VC1 0x0045ca50
void KdGraphics::SetMode(unsigned mode)
{
    state.mode = mode;
}

// FUNCTION: VC1 0x0045ca60
unsigned KdGraphics::GetMode()
{
    return state.mode;
}

// FUNCTION: VC1 0x0045ca70
void KdGraphics::SetColor(unsigned color)
{
    state.color = color;
}

// FUNCTION: VC1 0x0045ca80
unsigned KdGraphics::GetColor()
{
    return state.color;
}

// LEA EAX, [ECX+4]; RET. Same body as IGraphics::GetState, not folded.
// FUNCTION: VC1 0x0045ca90
Rect& KdGraphics::GetClipRect()
{
    return state.clip;
}

// RET 4. IAbstract has no null check in the original.
// FUNCTION: VC1 0x0045caa0
void* KdGraphics::QueryInterface(const char** iid)
{
    const char* name = *iid;
    if (name == g_iidScriptObject) {
        return static_cast<IScriptObject*>(this);
    }
    if (name == g_iidGraphics) {
        return static_cast<IGraphics*>(this);
    }
    if (name == g_iidAbstract) {
        return reinterpret_cast<char*>(this) + 0x48;
    }
    return 0;
}

// refs at +0x4c. Zero calls the deleting destructor through IAbstract.
// FUNCTION: VC1 0x0045cae0
unsigned KdGraphics::Release()
{
    unsigned value = --refs;
    if (value == 0) {
        delete static_cast<IAbstract*>(this);
        return 0;
    }
    return value;
}

// FUNCTION: VC1 0x0045cb00
unsigned KdGraphics::AddRef()
{
    unsigned value = refs;
    if (value != 0) {
        ++value;
        refs = value;
    }
    return value;
}

// RET 0x1c. Inferred name. Normalizes a negative width/height for the
// screen rectangle; texture coordinates use the unnormalized size scaled by
// 1/GetWidth and 1/GetHeight, so a negative size mirrors the image. The quad
// carries ITexture::native and state.mode into 0x004648b0 (EDI).
// FUNCTION: VC1 0x0045cb10
void KdGraphics::DrawImage(const KdPtr<ITexture>& texture, float x, float y, float u, float v,
    float width, float height)
{
    BatchQuad quad;
    float sourceWidth;
    float sourceHeight;
    float scaleU;
    float scaleV;
    float u0;
    float v0;
    float u1;
    float v1;

    sourceWidth = width;
    sourceHeight = height;
    if (width < 0.0f) {
        x += width;
        width = -width;
    }
    if (height < 0.0f) {
        y += height;
        height = -height;
    }
    quad.texture = texture->native;
    quad.flags = state.mode;
    scaleU = 1.0f / static_cast<float>(texture->GetWidth());
    scaleV = 1.0f / static_cast<float>(texture->GetHeight());
    u0 = u * scaleU;
    v0 = v * scaleV;
    u1 = (u + sourceWidth) * scaleU;
    v1 = (v + sourceHeight) * scaleV;
    quad.vertices[0].x = x;
    quad.vertices[0].y = y;
    quad.vertices[0].z = 0.0f;
    quad.vertices[0].color = state.color;
    quad.vertices[0].u = u0;
    quad.vertices[0].v = v0;
    quad.vertices[1].x = x + width;
    quad.vertices[1].y = y;
    quad.vertices[1].z = 0.0f;
    quad.vertices[1].color = state.color;
    quad.vertices[1].u = u1;
    quad.vertices[1].v = v0;
    quad.vertices[2].x = x + width;
    quad.vertices[2].y = y + height;
    quad.vertices[2].z = 0.0f;
    quad.vertices[2].color = state.color;
    quad.vertices[2].u = u1;
    quad.vertices[2].v = v1;
    quad.vertices[3].x = x;
    quad.vertices[3].y = y + height;
    quad.vertices[3].z = 0.0f;
    quad.vertices[3].color = state.color;
    quad.vertices[3].u = u0;
    quad.vertices[3].v = v1;
    PushQuad(&quad);
}

// RET 0x10. Inferred name. Normalizes a negative width/height, then emits an
// untextured quad in state.color / state.mode through 0x004648b0 (EDI). The
// texture coordinates are left unset.
// FUNCTION: VC1 0x0045ccb0
void KdGraphics::FillRect(float x, float y, float width, float height)
{
    BatchQuad quad;

    if (width < 0.0f) {
        x += width;
        width = -width;
    }
    if (height < 0.0f) {
        y += height;
        height = -height;
    }
    quad.vertices[0].x = x;
    quad.vertices[0].y = y;
    quad.vertices[0].z = 0.0f;
    quad.vertices[0].color = state.color;
    quad.vertices[1].x = x + width;
    quad.vertices[1].y = y;
    quad.vertices[1].z = 0.0f;
    quad.vertices[1].color = state.color;
    quad.vertices[2].x = x + width;
    quad.vertices[2].y = y + height;
    quad.vertices[2].z = 0.0f;
    quad.vertices[2].color = state.color;
    quad.vertices[3].x = x;
    quad.vertices[3].y = y + height;
    quad.vertices[3].z = 0.0f;
    quad.vertices[3].color = state.color;
    quad.texture = 0;
    quad.flags = state.mode;
    PushQuad(&quad);
}

// JMP 0x00418830.
// FUNCTION: VC1 0x0045cd90
void KdGraphics::Translate(const Vector2& offset)
{
    IGraphics::Translate(offset);
}

// ESI=this. Camera <- g_displayScene QI ICamera; font <- QI IFont via SetFont.
// FUNCTION: VC1 0x0045cda0
void KdGraphics::BindScene()
{
    IUnknown* font;

    if (camera) {
        camera->Release();
    }
    if (!g_displayScene) {
        camera = 0;
    }
    else {
        camera = static_cast<IUnknown*>(g_displayScene->QueryInterface(&g_iidCamera));
        if (camera) {
            camera->AddRef();
        }
    }
    if (!g_displayScene) {
        font = 0;
    }
    else {
        font = static_cast<IUnknown*>(g_displayScene->QueryInterface(&g_iidFont));
        if (font) {
            font->AddRef();
        }
    }
    SetFont(*reinterpret_cast<const KdPtr<IFont>*>(&font));
    if (font) {
        font->Release();
    }
}

// ESI=this, EAX=rect, ECX=camera slot. Resets to a default GraphicsState, clips
// to the view, then hands the IAbstract face and view to camera vtable+0x14.
// FUNCTION: VC1 0x0045ce30
unsigned char KdGraphics::SetView(const Rect* rect, IUnknown* const* newCamera)
{
    IUnknown* face;
    IUnknown* ident;
    typedef void (__thiscall *FnBindView)(IUnknown* object, IUnknown** ident, Rect* view);

    view = *rect;
    {
        GraphicsState reset;
        SetState(&reset);
    }
    SetClipRect(view);
    if (*newCamera) {
        (*newCamera)->AddRef();
    }
    if (camera) {
        camera->Release();
    }
    camera = *newCamera;
    if (camera) {
        face = static_cast<IUnknown*>(static_cast<IAbstract*>(this)->QueryInterface(
            &g_iidAbstract));
        if (!face) {
            ident = 0;
        }
        else {
            ident = static_cast<IUnknown*>(face->QueryInterface(&g_iidAbstract));
            if (ident) {
                ident->AddRef();
            }
        }
        reinterpret_cast<FnBindView>((*reinterpret_cast<void***>(camera))[5])(camera, &ident, &view);
        if (ident) {
            ident->Release();
        }
    }
    return 1;
}

// GetTransform() then origin at +0x18.
// FUNCTION: VC1 0x0045cfa0
Vector2& KdGraphics::GetOrigin()
{
    return GetTransform().GetOrigin();
}

// RET 4. Copies the clip rect, then 0x00464c10(x, y; ECX=width, EAX=height).
// FUNCTION: VC1 0x0045cfb0
void KdGraphics::SetClipRect(const Rect& rect)
{
    state.clip = rect;
    DisplayD3dSetClip(rect.x, rect.y, rect.width, rect.height);
}

// EAX=this (malloc(0x64) in IRenderTarget()). GraphicsState defaults, then the
// IScriptObject/IAbstract default vtables, refs=1, camera and view cleared.
// FUNCTION: VC1 0x0045cff0
KdGraphics::KdGraphics()
{
    camera = 0;
    view.height = 0;
    view.width = 0;
    view.y = 0;
    view.x = 0;
}

// STRING: VC1 0x004d58d4
static const char kScriptGetClip[] = "GetClip";
// STRING: VC1 0x004d2070
static const char kScriptSetClip[] = "SetClip";
// STRING: VC1 0x004d58dc
static const char kScriptGetColor[] = "GetColor";
// STRING: VC1 0x004d2034
static const char kScriptSetColor[] = "SetColor";
// STRING: VC1 0x004d58e8
static const char kScriptGetBlendMode[] = "GetBlendMode";
// STRING: VC1 0x004d58f8
static const char kScriptSetBlendMode[] = "SetBlendMode";
// STRING: VC1 0x004d5908
static const char kScriptSetFont[] = "SetFont";
// GLOBAL: VC1 0x004d591c
extern const char kScriptGetTransform[] = "GetTransform";
// GLOBAL: VC1 0x004d592c
extern const char kScriptSetTransform[] = "SetTransform";
// STRING: VC1 0x004d593c
static const char kScriptGetTranslate[] = "GetTranslate";
// STRING: VC1 0x004d594c
static const char kScriptTranslate[] = "Translate";
// STRING: VC1 0x004d5958
static const char kScriptDrawRect[] = "DrawRect";
// STRING: VC1 0x004d5964
static const char kScriptFillRect[] = "FillRect";
// STRING: VC1 0x004d5970
static const char kScriptDrawTexture[] = "DrawTexture";
// STRING: VC1 0x004d597c
static const char kScriptDrawString[] = "DrawString";
// STRING: VC1 0x004d5988
static const char kScriptDrawLine[] = "DrawLine";
// STRING: VC1 0x004d5994
static const char kScriptDrawEllipse[] = "DrawEllipse";

// RET 8. this = IScriptObject (+0x44); the dispatchers QI the instance to
// IScriptObject and step back to KdGraphics. Every closure calls through the
// IGraphics vtable (vcall thunks 0x00417bb0..0x0041d2d0).
// FUNCTION: VC1 0x0045dc20
unsigned char KdGraphics::GetMember(const char* name, SquirrelObject* out)
{
    static ScriptMethodTable methods;

    if (methods.Empty()) {
        methods[kScriptGetClip] = CreateScriptMethod<KdGraphics>(&KdGraphics::GetClipRect);
        methods[kScriptSetClip] = CreateScriptMethod<KdGraphics>(&KdGraphics::SetClipRect);
        methods[kScriptGetColor] = CreateScriptMethod<KdGraphics>(&KdGraphics::GetColor);
        methods[kScriptSetColor] = CreateScriptMethod<KdGraphics>(&KdGraphics::SetColor);
        methods[kScriptGetBlendMode] = CreateScriptMethod<KdGraphics>(&KdGraphics::GetMode);
        methods[kScriptSetBlendMode] = CreateScriptMethod<KdGraphics>(&KdGraphics::SetMode);
        methods[kScriptGetFont] = CreateScriptMethod<KdGraphics>(&KdGraphics::GetFont);
        methods[kScriptSetFont] = CreateScriptMethod<KdGraphics>(&KdGraphics::SetFont);
        methods[kScriptClear] = CreateScriptMethod<KdGraphics>(&KdGraphics::Clear);
        methods[kScriptGetCamera] = CreateScriptMethod<KdGraphics>(&KdGraphics::GetCamera);
        methods[kScriptGetTransform] = CreateScriptMethod<KdGraphics>(&KdGraphics::GetTransform);
        methods[kScriptSetTransform] = CreateScriptMethod<KdGraphics>(&KdGraphics::SetTransform);
        methods[kScriptGetTranslate] = CreateScriptMethod<KdGraphics>(&KdGraphics::GetOrigin);
        methods[kScriptTranslate] = CreateScriptMethod<KdGraphics>(&KdGraphics::Translate);
        methods[kScriptDrawRect] = CreateScriptMethod<KdGraphics>(&KdGraphics::DrawRect);
        methods[kScriptFillRect] = CreateScriptMethod<KdGraphics>(&KdGraphics::FillRect);
        methods[kScriptDrawTexture] = CreateScriptMethod<KdGraphics>(&KdGraphics::DrawImage);
        methods[kScriptDrawString] = CreateScriptMethod<KdGraphics>(&KdGraphics::DrawString);
        methods[kScriptDrawLine] = CreateScriptMethod<KdGraphics>(&KdGraphics::DrawLine);
        methods[kScriptDrawEllipse] = CreateScriptMethod<KdGraphics>(&KdGraphics::DrawEllipse);
    }
    return methods.Find(name, out);
}

}
