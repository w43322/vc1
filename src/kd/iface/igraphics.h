#ifndef VC1_KD_IGRAPHICS_H
#define VC1_KD_IGRAPHICS_H

#include "kd/iface/iunknown.h"
#include "bindings/rect.h"
#include "bindings/matrix3.h"
#include "bindings/vector2.h"

namespace vc1 {

// Inferred name. Applied as one block by IGraphics::SetState (0x004187e0); a
// default local copy is built and destroyed in KdGraphics::SetView (0x0045ce30).
// SIZE 0x40
struct GraphicsState
{
    Rect clip;         // 0x00
    unsigned color;    // 0x10
    unsigned mode;     // 0x14
    Matrix3 transform; // 0x18
    IUnknown* font;    // 0x3c

    GraphicsState()
    {
        color = 0xffffffff;
        mode = 2;
        clip.x = 0;
        clip.y = 0;
        clip.width = 0x7fffffff;
        clip.height = 0x7fffffff;
        transform.m[0] = 1.0f;
        transform.m[1] = 0.0f;
        transform.m[2] = 0.0f;
        transform.m[3] = 0.0f;
        transform.m[4] = 1.0f;
        transform.m[5] = 0.0f;
        transform.m[6] = 0.0f;
        transform.m[7] = 0.0f;
        transform.m[8] = 1.0f;
        font = 0;
    }

    ~GraphicsState()
    {
        if (font) {
            font->Release();
        }
        font = 0;
    }
};

typedef char Vc1GraphicsStateSizeCheck[sizeof(GraphicsState) == 0x40 ? 1 : -1];

// IID "IGraphics" (g_iidGraphics 0x004d05a0); KdGraphics QI returns its +0x00.
// Slot names are inferred from the KdGraphics bodies (vtable 0x004d6770).
// SIZE 0x44
struct IGraphics : IUnknown
{
    GraphicsState state; // 0x04

    virtual GraphicsState* GetState();                    // vtable+0x0c
    virtual void SetState(const GraphicsState* value);    // vtable+0x10
    virtual IUnknown** GetCamera() = 0;                   // vtable+0x14
    virtual Rect* GetView() = 0;                          // vtable+0x18
    virtual Rect* GetClipRect() = 0;                      // vtable+0x1c
    virtual void SetClipRect(const Rect* rect) = 0;       // vtable+0x20
    virtual unsigned GetColor() = 0;                      // vtable+0x24
    virtual void SetColor(unsigned color) = 0;            // vtable+0x28
    virtual unsigned GetMode() = 0;                       // vtable+0x2c
    virtual void SetMode(unsigned mode) = 0;              // vtable+0x30
    virtual IUnknown** GetFont() = 0;                     // vtable+0x34
    virtual void SetFont(IUnknown* const* font) = 0;      // vtable+0x38
    virtual void Clear(unsigned color) = 0;               // vtable+0x3c
    virtual Matrix3* GetTransform() = 0;                  // vtable+0x40
    virtual void SetTransform(const Matrix3* matrix) = 0; // vtable+0x44
    virtual Vector2* GetOrigin() = 0;                     // vtable+0x48
    virtual void Translate(const Vector2* offset);        // vtable+0x4c
    virtual void FillRect(float x, float y, float width, float height) = 0; // vtable+0x50
    virtual void Graphics54(float a, float b, float c, float d) = 0;          // vtable+0x54
    virtual void Graphics58(void* a, float b, float c, float d, float e,
        float f, float g) = 0;                                                // vtable+0x58
    virtual void Graphics5c(void* a, void* b, unsigned c) = 0;                // vtable+0x5c
    virtual void DrawString(const char* text, float x, float y) = 0;        // vtable+0x60
    virtual void Graphics64(float a, float b, float c, float d) = 0;          // vtable+0x64
    virtual void Graphics68(void* a, float b, float c, float d) = 0;          // vtable+0x68
};

typedef char Vc1IGraphicsSizeCheck[sizeof(IGraphics) == 0x44 ? 1 : -1];

}

#endif
