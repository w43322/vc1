#ifndef VC1_KD_FONT_H
#define VC1_KD_FONT_H

#include "kd/factory.h"
#include "kd/iface/ifont.h"
#include "kd/iface/iloadable.h"
#include "kd/iface/iscript_object.h"
#include "kd/iface/iabstract.h"

namespace vc1 {

// Inferred name. One BMFont "char" record: source rectangle in page texture
// `page`, pen offset and advance in pixels.
// SIZE 0x20
struct GlyphInfo
{
    int u;       // 0x00
    int v;       // 0x04
    int width;   // 0x08
    int height;  // 0x0c
    int offsetX; // 0x10
    int offsetY; // 0x14
    int advance; // 0x18
    int page;    // 0x1c
};

typedef char Vc1GlyphInfoSizeCheck[sizeof(GlyphInfo) == 0x20 ? 1 : -1];

// Inferred name. Glyph table record: code point key plus its GlyphInfo.
// SIZE 0x24
struct Glyph
{
    unsigned code;  // 0x00  Unicode code point, table key
    GlyphInfo info; // 0x04

    Glyph() : code(0), info(GlyphInfo()) {}
    Glyph(unsigned key, const GlyphInfo& value) : code(key), info(value) {}
};

typedef char Vc1GlyphSizeCheck[sizeof(Glyph) == 0x24 ? 1 : -1];

// Inferred name. Cursor for DecodeUtf8: text stays on the last byte of the
// decoded sequence; callers step past it.
// SIZE 0x08
struct Utf8Cursor
{
    const char* text; // 0x00
    unsigned code;    // 0x04
};

void DecodeUtf8(Utf8Cursor* cursor);
GlyphInfo* InsertGlyph(KdFactory* glyphs, const unsigned* code);
Glyph* __stdcall FindGlyph(KdFactory* glyphs, const unsigned* code);
void ReserveGlyphs(KdFactory* glyphs, unsigned count);
void ConstructGlyphs(Glyph* first, Glyph* last);
extern const char kFont[];
extern const unsigned g_utf8Limits[6];

// Inferred name. Script14 "Font"; created by the factory (0x00466f50) and
// cached by the resource manager. Bitmap pages plus a code-sorted glyph table.
// SIZE 0x44
// VTABLE: VC1 0x004d5cd0 vc1::IFont
// VTABLE: VC1 0x004d5cf0 vc1::ILoadable
// VTABLE: VC1 0x004d5d00 vc1::IScriptObject
// VTABLE: VC1 0x004d5d18 vc1::IAbstract
class KdFont : public IFont,
               public ILoadable,
               public IScriptObject,
               public IAbstract
{
public:
    KdRefList pages;  // 0x14  ITexture* per page
    int lineHeight;   // 0x24
    int base;         // 0x28  BMFont common base
    int scaleW;       // 0x2c
    int scaleH;       // 0x30
    KdFactory glyphs; // 0x34  Glyph records sorted by code

    KdFont();
    ~KdFont();

    void* QueryInterface(const char** iid); // vtable+0x00
    unsigned AddRef();                      // vtable+0x04
    unsigned Release();                     // vtable+0x08

    bool HasGlyph(unsigned code);
    int GetTextWidth(const char* text);
    int GetHeight();
    KdStringList WrapText(const char* text, int width);
    void DrawText(IUnknown* const* graphics, const char* text);

    unsigned char Load(const char* name) throw();

    unsigned char Script0c(const char* name, SquirrelObject* out);
    unsigned char Script10(void* a, void* b);
    const char* Script14();

    Glyph* Find(unsigned code)
    {
        Glyph* found = FindGlyph(&glyphs, &code);
        Glyph* end = reinterpret_cast<Glyph*>(
            reinterpret_cast<unsigned char*>(glyphs.begin) + glyphs.size);
        if (found < end && code < found->code) {
            found = end;
        }
        return found;
    }
    Glyph* GlyphEnd()
    {
        return reinterpret_cast<Glyph*>(
            reinterpret_cast<unsigned char*>(glyphs.begin) + glyphs.size);
    }
};

typedef char Vc1KdFontSizeCheck[sizeof(KdFont) == 0x44 ? 1 : -1];

// SYNTHETIC: VC1 0x00419b10
// vc1::KdFont::`scalar deleting destructor'
// SYNTHETIC: VC1 0x0041a2a0
// ?QueryInterface@KdFont@vc1@@WM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0041a2b0
// ?Release@KdFont@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x0041a2c0
// ??_EKdFont@vc1@@WM@AEPAXI@Z
// SYNTHETIC: VC1 0x0041a2d0
// ?Release@KdFont@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x0041a2e0
// ?AddRef@KdFont@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x0041a2f0
// ?AddRef@KdFont@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x0041a300
// ?AddRef@KdFont@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x0041a310
// ?QueryInterface@KdFont@vc1@@W3AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0041a320
// ?QueryInterface@KdFont@vc1@@W7AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0041a330
// ?Release@KdFont@vc1@@W3AEIXZ

}

#endif
