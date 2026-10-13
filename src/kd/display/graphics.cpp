#include "kd/display/graphics.h"
#include "kd/display/display.h"

#include "kd/iface/iid.h"

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
    SetClipRect(&value->clip);
    SetColor(value->color);
    SetMode(value->mode);
    SetTransform(&value->transform);
    SetFont(&value->font);
}

// RET 4. Identity with origin = offset, folded into state.transform in place by
// 0x00473cd0 (EAX=transform, ECX=local), then SetTransform(transform).
// FUNCTION: VC1 0x00418830
void IGraphics::Translate(const Vector2* offset)
{
    Matrix3 move;

    move.m[0] = 1.0f;
    move.m[1] = 0.0f;
    move.m[2] = 0.0f;
    move.m[3] = 0.0f;
    move.m[4] = 1.0f;
    move.m[5] = 0.0f;
    move.m[6] = offset->x;
    move.m[7] = offset->y;
    move.m[8] = 1.0f;
    SetTransform(state.transform.LeftMultiply(&move));
}

// MOV EAX, 0x004d4c40; RET. IScriptObject vtable+0x14.
// FUNCTION: VC1 0x0041c8d0
const char* KdGraphics::Script14()
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

// RET 0x10. Four floats; draws through vtable+0x64. Body not recovered.
// STUB: VC1 0x0045c610
void KdGraphics::Graphics54(float a, float b, float c, float d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
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
unsigned char KdGraphics::Script10(void* a, void* b)
{
    (void)a;
    (void)b;
    return 0;
}

// RET 0x10. Body not recovered.
// STUB: VC1 0x0045c700
void KdGraphics::Graphics68(void* a, float b, float c, float d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
}

// RET 0x10. Body not recovered.
// STUB: VC1 0x0045c810
void KdGraphics::Graphics64(float a, float b, float c, float d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
}

// RET 0xc. Inferred name: draws text at (x, y) with state.font (+0x40);
// KdSystemBase::Render passes the console line. Body not recovered.
// STUB: VC1 0x0045c850
void KdGraphics::DrawString(const char* text, float x, float y)
{
    (void)text;
    (void)x;
    (void)y;
}

// RET 0xc. Batches 0x60-byte records through g_batchCount. Body not recovered.
// STUB: VC1 0x0045c910
void KdGraphics::Graphics5c(void* a, void* b, unsigned c)
{
    (void)a;
    (void)b;
    (void)c;
}

// RET 4. REP MOVSD 9 dwords into state.transform, then 0x00464710 (EAX=matrix).
// FUNCTION: VC1 0x0045c9a0
void KdGraphics::SetTransform(const Matrix3* matrix)
{
    state.transform = *matrix;
    DisplayD3dSetView(matrix);
}

// LEA EAX, [ECX+0x1c]; RET.
// FUNCTION: VC1 0x0045c9c0
Matrix3* KdGraphics::GetTransform()
{
    return &state.transform;
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
void KdGraphics::SetFont(IUnknown* const* font)
{
    if (*font) {
        (*font)->AddRef();
    }
    if (state.font) {
        state.font->Release();
    }
    state.font = *font;
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
Rect* KdGraphics::GetClipRect()
{
    return &state.clip;
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
void KdGraphics::DrawImage(ITexture* const* texture, float x, float y, float u, float v,
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
    quad.texture = (*texture)->native;
    quad.flags = state.mode;
    scaleU = 1.0f / static_cast<float>((*texture)->GetWidth());
    scaleV = 1.0f / static_cast<float>((*texture)->GetHeight());
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
void KdGraphics::Translate(const Vector2* offset)
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
    SetFont(&font);
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
    typedef void (__thiscall *Fn14)(IUnknown* object, IUnknown** ident, Rect* view);

    view = *rect;
    {
        GraphicsState reset;
        SetState(&reset);
    }
    SetClipRect(&view);
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
        reinterpret_cast<Fn14>((*reinterpret_cast<void***>(camera))[5])(camera, &ident, &view);
        if (ident) {
            ident->Release();
        }
    }
    return 1;
}

// GetTransform() then origin at +0x18.
// FUNCTION: VC1 0x0045cfa0
Vector2* KdGraphics::GetOrigin()
{
    return GetTransform()->GetOrigin();
}

// RET 4. Copies the clip rect, then 0x00464c10(x, y; ECX=width, EAX=height).
// FUNCTION: VC1 0x0045cfb0
void KdGraphics::SetClipRect(const Rect* rect)
{
    state.clip = *rect;
    DisplayD3dSetClip(rect->x, rect->y, rect->width, rect->height);
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

// RET 8. Script registration body; not recovered.
// STUB: VC1 0x0045dc20
unsigned char KdGraphics::Script0c(void* a, void* b)
{
    (void)a;
    (void)b;
    return 0;
}

}
