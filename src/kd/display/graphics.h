#ifndef VC1_KD_GRAPHICS_H
#define VC1_KD_GRAPHICS_H

#include "kd/iface/igraphics.h"
#include "kd/iface/iscript_object.h"
#include "kd/iface/iabstract.h"

namespace vc1 {

// Inferred name from Script14 "Graphics" (0x004d4c40). IRenderTarget() mallocs
// 0x64 bytes and constructs it (0x0045cff0). QI 0x0045caa0: IGraphics this,
// IScriptObject +0x44, IAbstract +0x48.
// SIZE 0x64
// VTABLE: VC1 0x004d6770 vc1::IGraphics
// VTABLE: VC1 0x004d67dc vc1::IScriptObject
// VTABLE: VC1 0x004d67f4 vc1::IAbstract
class KdGraphics : public IGraphics,
                   public IScriptObject,
                   public IAbstract
{
public:
    IUnknown* camera; // 0x50
    Rect view;        // 0x54

    KdGraphics();
    ~KdGraphics();

    void* QueryInterface(const char** iid); // vtable+0x00
    unsigned AddRef();                      // vtable+0x04
    unsigned Release();                     // vtable+0x08

    IUnknown** GetCamera();
    Rect* GetView();
    Rect& GetClipRect();
    void SetClipRect(const Rect& rect);
    unsigned GetColor();
    void SetColor(unsigned color);
    unsigned GetMode();
    void SetMode(unsigned mode);
    IUnknown** GetFont();
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

    unsigned char Script0c(const char* name, SquirrelObject* out);
    unsigned char Script10(void* a, void* b);
    const char* Script14();

    void BindScene();
    unsigned char SetView(const Rect* rect, IUnknown* const* newCamera);
};

extern const char kScriptGetTransform[];
extern const char kScriptSetTransform[];

typedef char Vc1KdGraphicsSizeCheck[sizeof(KdGraphics) == 0x64 ? 1 : -1];

// SYNTHETIC: VC1 0x0041c8e0
// vc1::KdGraphics::`scalar deleting destructor'
// SYNTHETIC: VC1 0x0041d200
// ??_EKdGraphics@vc1@@WEI@AEPAXI@Z
// SYNTHETIC: VC1 0x0041d210
// ?Release@KdGraphics@vc1@@WEI@AEIXZ
// SYNTHETIC: VC1 0x0041d220
// ?AddRef@KdGraphics@vc1@@WEE@AEIXZ
// SYNTHETIC: VC1 0x0041d250
// ?AddRef@KdGraphics@vc1@@WEI@AEIXZ
// SYNTHETIC: VC1 0x0041d280
// ?QueryInterface@KdGraphics@vc1@@WEE@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0041d2b0
// ?QueryInterface@KdGraphics@vc1@@WEI@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0041d2c0
// ?Release@KdGraphics@vc1@@WEE@AEIXZ

}

#endif
