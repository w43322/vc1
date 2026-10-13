#include "kd/font/font.h"

#include "kd/iface/iid.h"
#include "kd/iface/igraphics.h"
#include "kd/iface/itexture.h"

#include <new>
#include <stdlib.h>

namespace vc1 {

// GLOBAL: VC1 0x004d4bd4
extern const char kFont[] = "Font";
// GLOBAL: VC1 0x004d0660
const char* g_kdNameFont = kFont;
// Largest code point for each UTF-8 sequence length.
// GLOBAL: VC1 0x004d0d80
extern const unsigned g_utf8Limits[6] = {
    0x7f, 0x7ff, 0xffff, 0x1fffff, 0x3ffffff, 0x7fffffff
};

// EAX=cursor. Decodes one UTF-8 sequence at cursor->text into cursor->code:
// the lead byte's leading ones give the length, continuation bytes add six
// bits each, and a NUL ends the sequence early.
// FUNCTION: VC1 0x004025d0
void DecodeUtf8(Utf8Cursor* cursor)
{
    unsigned char lead;
    unsigned short bits;
    unsigned short mask;
    int length;
    char next;

    lead = static_cast<unsigned char>(*cursor->text);
    bits = lead;
    mask = 0x80;
    length = 0;
    if (lead & 0x80) {
        do {
            mask >>= 1;
            ++length;
        } while (bits & mask);
    }
    if (length == 0) {
        length = 1;
    }
    cursor->code = (0xff >> length) & lead;
    while (--length != 0) {
        ++cursor->text;
        next = *cursor->text;
        if (next == 0) {
            break;
        }
        cursor->code = (cursor->code << 6) | (next & 0x3f);
    }
}

// ESI=self, RET 4. Releases the old font, then holds source QI'd to IFont.
// FUNCTION: VC1 0x00418090
KdRef<IFont>* AssignFont(KdRef<IFont>* self, IUnknown* const* source)
{
    IFont* queried;

    if (self->ptr) {
        self->ptr->Release();
    }
    if (!*source) {
        self->ptr = 0;
        return self;
    }
    queried = static_cast<IFont*>((*source)->QueryInterface(&g_iidFont));
    self->ptr = queried;
    if (queried) {
        queried->AddRef();
    }
    return self;
}

// MOV EAX, "Font"; RET.
// FUNCTION: VC1 0x00419b00
const char* KdFont::Script14()
{
    return kFont;
}

// EAX=glyphs, RET 4. Lower bound of *code in the code-sorted Glyph table.
// FUNCTION: VC1 0x00419f30
Glyph* __stdcall FindGlyph(KdFactory* glyphs, const unsigned* code)
{
    Glyph* first;
    Glyph* last;
    Glyph* middle;

    first = reinterpret_cast<Glyph*>(glyphs->begin);
    last = reinterpret_cast<Glyph*>(
        reinterpret_cast<unsigned char*>(glyphs->begin) + glyphs->size);
    while (first != last) {
        middle = first + (last - first) / 2;
        if (middle->code < *code) {
            first = middle + 1;
        }
        else {
            last = middle;
        }
    }
    return first;
}

// MOV EAX, [ECX+0x24]; RET.
// FUNCTION: VC1 0x00466e40
int KdFont::GetHeight()
{
    return lineHeight;
}

// XOR AL, AL; RET 8.
// FUNCTION: VC1 0x00466e50
unsigned char KdFont::Script10(void* a, void* b)
{
    (void)a;
    (void)b;
    return 0;
}

// RET 4. IAbstract has no null check in the original.
// FUNCTION: VC1 0x00466e60
void* KdFont::QueryInterface(const char** iid)
{
    const char* name = *iid;
    if (name == g_iidScriptObject) {
        return static_cast<IScriptObject*>(this);
    }
    if (name == g_iidFont) {
        return static_cast<IFont*>(this);
    }
    if (name == g_iidLoadable) {
        return static_cast<ILoadable*>(this);
    }
    if (name == g_iidAbstract) {
        return reinterpret_cast<char*>(this) + 0x0c;
    }
    return 0;
}

// refs at +0x10. Zero calls the deleting destructor through IAbstract.
// FUNCTION: VC1 0x00466eb0
unsigned KdFont::Release()
{
    unsigned value = --refs;
    if (value == 0) {
        delete static_cast<IAbstract*>(this);
        return 0;
    }
    return value;
}

// FUNCTION: VC1 0x00466ed0
unsigned KdFont::AddRef()
{
    unsigned value = refs;
    if (value != 0) {
        ++value;
        refs = value;
    }
    return value;
}

// EAX=this. Called only by CreateKdFont.
// FUNCTION: VC1 0x00466ee0
KdFont::KdFont()
    : lineHeight(0),
      unknown28(0),
      unknown2c(0),
      unknown30(0)
{
}

// FUNCTION: VC1 0x00466f50
void** CreateKdFont(void** out)
{
    KdFont* memory = static_cast<KdFont*>(malloc(sizeof(KdFont)));
    KdFont* object;
    IUnknown* queried;

    object = 0;
    if (memory) {
        object = new (memory) KdFont();
    }
    if (object) {
        queried = static_cast<IUnknown*>(
            static_cast<IAbstract*>(object)->QueryInterface(&g_iidAbstract));
    }
    else {
        queried = 0;
    }
    *out = queried;
    if (queried) {
        queried->AddRef();
    }
    KdRelease(static_cast<IUnknown*>(*out));
    return out;
}

// ESI=this. glyphs is destroyed inline, then pages (0x00419bb0).
// FUNCTION: VC1 0x00466fa0
KdFont::~KdFont()
{
}

// RET 8. Draws each glyph of the UTF-8 text with IGraphics::DrawImage from
// its page texture, advancing the pen by the glyph advance. The caller has
// already translated the transform to the text origin.
// FUNCTION: VC1 0x00467000
void KdFont::DrawText(IUnknown* const* graphics, const char* text)
{
    IGraphics* target;
    Utf8Cursor cursor;
    Glyph* glyph;
    int pen;

    pen = 0;
    if (*graphics) {
        target = static_cast<IGraphics*>((*graphics)->QueryInterface(&g_iidGraphics));
        if (target) {
            target->AddRef();
        }
    }
    else {
        target = 0;
    }
    cursor.text = text;
    cursor.code = 0;
    DecodeUtf8(&cursor);
    while (cursor.code != 0) {
        glyph = Find(cursor.code);
        if (glyph != GlyphEnd()) {
            target->DrawImage(reinterpret_cast<ITexture* const*>(pages.items.begin) + glyph->page,
                static_cast<float>(glyph->offsetX + pen), static_cast<float>(glyph->offsetY),
                static_cast<float>(glyph->u), static_cast<float>(glyph->v),
                static_cast<float>(glyph->width), static_cast<float>(glyph->height));
            pen += glyph->advance;
        }
        ++cursor.text;
        DecodeUtf8(&cursor);
    }
    if (target) {
        target->Release();
    }
}

// RET 4. Sum of glyph advances; code points without a glyph add nothing.
// FUNCTION: VC1 0x00467120
int KdFont::GetTextWidth(const char* text)
{
    Utf8Cursor cursor;
    Glyph* glyph;
    int width;

    cursor.text = text;
    width = 0;
    cursor.code = 0;
    DecodeUtf8(&cursor);
    while (cursor.code != 0) {
        glyph = Find(cursor.code);
        if (glyph != GlyphEnd()) {
            width += glyph->advance;
        }
        ++cursor.text;
        DecodeUtf8(&cursor);
    }
    return width;
}

// RET 4.
// FUNCTION: VC1 0x004671a0
unsigned char KdFont::HasGlyph(unsigned code)
{
    return Find(code) != GlyphEnd();
}

// RET 0xc (hidden result pointer, text, width). Inferred name. Splits the
// UTF-8 text into lines no wider than width: a line ends at '\n', at the end
// of the text, or when the next glyph overflows; it then breaks at the last
// space (or at the overflowing character when the line has none). Lines
// after the first drop their leading spaces. The break test uses the
// current character, so a break at an earlier space still skips one byte
// only when the current character is ' ' or '\n'.
// FUNCTION: VC1 0x00467490
KdStringList KdFont::WrapText(const char* text, int width)
{
    KdStringList lines;
    Utf8Cursor cursor;
    Utf8Cursor next;
    Glyph* glyph;
    unsigned short code;
    int length;
    int position;
    int lineStart;
    int lastBreak;
    int breakAt;
    int lineWidth;
    char current;
    bool ends;

    if (text) {
        lineStart = 0;
        lastBreak = 0;
        lineWidth = 0;
        cursor.text = text;
        cursor.code = 0;
        DecodeUtf8(&cursor);
        for (;;) {
            code = static_cast<unsigned short>(cursor.code);
            if (code < 0x80) {
                length = 1;
            }
            else {
                length = 0;
                while (g_utf8Limits[length++] < code) {
                }
            }
            position = static_cast<int>(cursor.text - length - text) + 1;
            current = text[position];
            ends = current == '\n' || current == 0;
            if (current == ' ') {
                lastBreak = position;
            }
            if (!ends) {
                glyph = Find(cursor.code);
                if (glyph != GlyphEnd()) {
                    lineWidth += glyph->advance;
                }
                if (lineWidth <= width) {
                    ++cursor.text;
                    DecodeUtf8(&cursor);
                    continue;
                }
                if (lastBreak == lineStart) {
                    lastBreak = position;
                }
            }
            else {
                lastBreak = position;
            }
            breakAt = lastBreak;
            if (current == '\n' || current == ' ') {
                ++breakAt;
            }
            next.text = text + breakAt;
            next.code = 0;
            DecodeUtf8(&next);
            cursor = next;
            if (lines.items.size >> 4) {
                while (lineStart < lastBreak && text[lineStart] == ' ') {
                    ++lineStart;
                }
            }
            {
                KdString line(text + lineStart, text + lastBreak);
                lines.PushBack(line);
            }
            if (current == 0) {
                break;
            }
            lineStart = breakAt;
            lastBreak = breakAt;
            lineWidth = 0;
        }
    }
    return lines;
}

// Script binding table built on first use (guard 0x004ee598). Body not
// recovered.
// STUB: VC1 0x004676e0
unsigned char KdFont::Script0c(void* a, void* b)
{
    (void)a;
    (void)b;
    return 0;
}

// ILoadable face. Parses the font description ("charSpacing", page file
// names relative to the font path, glyph records) and logs "Loaded %s
// (height: %i glyphs: %i)". Body not recovered.
// STUB: VC1 0x00467940
unsigned char KdFont::Load(const char* name)
{
    (void)name;
    return 0;
}

}
