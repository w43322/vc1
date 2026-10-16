#include "kd/font/font.h"

#include "bindings/sqplus.h"

#include "kd/iface/iid.h"
#include "kd/iface/igraphics.h"
#include "kd/iface/itexture.h"
#include "host/runtime.h"
#include "kd/iface/iresource_manager.h"
#include "kd/stream/file_reader.h"

#include <new>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// <windows.h> (via host/runtime.h) maps DrawText to DrawTextA.
#undef DrawText

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

// ESI=out, RET 4. Appends code as UTF-8 at *out and advances it. The length
// search compares only the low 16 bits; the lead byte keeps six payload bits
// under its length prefix.
// FUNCTION: VC1 0x00402540
char** EncodeUtf8(char** out, unsigned code)
{
    unsigned length;
    unsigned short shift;

    if (static_cast<unsigned short>(code) >= 0x80) {
        length = 0;
        while (g_utf8Limits[length++] < static_cast<unsigned short>(code)) {
        }
        if (length > 1) {
            shift = static_cast<unsigned short>(length * 6 - 6);
            **out = static_cast<char>((0xff << (8 - length)) | ((code >> shift) & 0x3f));
            ++*out;
            while (shift) {
                shift -= 6;
                **out = static_cast<char>(((code >> shift) & 0x3f) | 0x80);
                ++*out;
            }
            return out;
        }
    }
    **out = static_cast<char>(code);
    ++*out;
    return out;
}

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
const char* KdFont::GetTypeName()
{
    return kFont;
}

// ECX=glyphs, EAX=code. Sorted-table operator[]: the record for *code, or a
// zeroed one inserted at its lower bound; returns the record's info.
// FUNCTION: VC1 0x00419c10
GlyphInfo* InsertGlyph(KdFactory* glyphs, const unsigned* code)
{
    Glyph* found;
    unsigned char* oldBegin;
    Glyph* slot;

    found = FindGlyph(glyphs, code);
    oldBegin = reinterpret_cast<unsigned char*>(glyphs->begin);
    if (found != reinterpret_cast<Glyph*>(oldBegin + glyphs->size) && *code >= found->code) {
        return &found->info;
    }
    Glyph record(*code, GlyphInfo());
    ReserveGlyphs(glyphs, glyphs->size / sizeof(Glyph) + 1);
    slot = static_cast<Glyph*>(InsertCountedBytes(glyphs,
        reinterpret_cast<unsigned char*>(glyphs->begin)
            + (reinterpret_cast<unsigned char*>(found) - oldBegin),
        sizeof(Glyph)));
    *slot = record;
    return &slot->info;
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

// EAX=glyphs, ECX=count. Grows to count records (inexact) and
// default-constructs the new tail.
// FUNCTION: VC1 0x0041a170
void ReserveGlyphs(KdFactory* glyphs, unsigned count)
{
    unsigned old;
    unsigned now;
    Glyph* records;

    old = glyphs->capacity / sizeof(Glyph);
    GrowCountedBuffer(glyphs, static_cast<int>(count * sizeof(Glyph)), 0);
    now = glyphs->capacity / sizeof(Glyph);
    if (now > old) {
        records = reinterpret_cast<Glyph*>(glyphs->begin);
        ConstructGlyphs(records + old, records + now);
    }
}

// EDX=first, cdecl last.
// FUNCTION: VC1 0x0041a240
void ConstructGlyphs(Glyph* first, Glyph* last)
{
    for (; first < last; ++first) {
        new (first) Glyph();
    }
}

// MOV EAX, [ECX+0x24]; RET.
// FUNCTION: VC1 0x00466e40
int KdFont::GetHeight()
{
    return lineHeight;
}

// XOR AL, AL; RET 8.
// FUNCTION: VC1 0x00466e50
unsigned char KdFont::SetMember(void* a, void* b)
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
      base(0),
      scaleW(0),
      scaleH(0)
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
            target->DrawImage(reinterpret_cast<const KdPtr<ITexture>*>(pages.items.begin)[glyph->info.page],
                static_cast<float>(glyph->info.offsetX + pen), static_cast<float>(glyph->info.offsetY),
                static_cast<float>(glyph->info.u), static_cast<float>(glyph->info.v),
                static_cast<float>(glyph->info.width), static_cast<float>(glyph->info.height));
            pen += glyph->info.advance;
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
            width += glyph->info.advance;
        }
        ++cursor.text;
        DecodeUtf8(&cursor);
    }
    return width;
}

// RET 4.
// FUNCTION: VC1 0x004671a0
bool KdFont::HasGlyph(unsigned code)
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
                    lineWidth += glyph->info.advance;
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

// RET 8. Script method lookup. The table (original 0x004ee588, guard bit
// 0x004ee598, atexit destructor 0x004cac10) is filled on first use with the
// four IFont methods under their script names. A GLOBAL marker here would
// name it vc1::methods, which no recompiled symbol matches.
// FUNCTION: VC1 0x004676e0
unsigned char KdFont::GetMember(const char* name, SquirrelObject* out)
{
    static ScriptMethodTable methods;

    if (methods.Empty()) {
        // STRING: VC1 0x004d5bac
        methods["IsSymbolExists"] = CreateScriptMethod<KdFont>(&KdFont::HasGlyph);
        methods[kScriptGetWidth] = CreateScriptMethod<KdFont>(&KdFont::GetTextWidth);
        methods[kScriptGetHeight] = CreateScriptMethod<KdFont>(&KdFont::GetHeight);
        // STRING: VC1 0x004d5bbc
        methods["SplitString"] = CreateScriptMethod<KdFont>(&KdFont::WrapText);
    }
    return methods.Find(name, out);
}

// ECX=ILoadable face, RET 4. BMFont text description: an optional
// "charSpacing=" key on the first line, "common", one "page" line per texture
// (resolved next to the font file and loaded through the resource manager as
// ITexture) and one "char" line per glyph, its advance widened by
// charSpacing. Any malformed line or missing page fails.
// FUNCTION: VC1 0x00467940
unsigned char KdFont::Load(const char* name) throw()
{
    KdFileReader reader(name);
    unsigned count;
    unsigned index;
    int charSpacing;
    int* height;

    if (!reader.source.file) {
        return 0;
    }
    KdString line;
    charSpacing = 0;
    reader.ReadLine(line);
    // STRING: VC1 0x004d5bcc
    int at = line.Find(KdString("charSpacing"));
    if (at != -1) {
        KdString tail(line.c_str() + at, line.c_str() + line.size);
        // STRING: VC1 0x004d5bd8
        if (sscanf(tail.c_str(), "charSpacing=%d", &charSpacing) != 1) {
            return 0;
        }
    }
    reader.ReadLine(line);
    height = &lineHeight;
    // STRING: VC1 0x004d5be8
    if (sscanf(line.c_str(), "common lineHeight=%d base=%d scaleW=%d scaleH=%d pages=%d\n",
            &lineHeight, &base, &scaleW, &scaleH, &count) != 5) {
        return 0;
    }
    pages.Resize(count);
    for (index = 0; index < count; ++index) {
        int id;
        char file[0x44];

        reader.ReadLine(line);
        // STRING: VC1 0x004d5c24
        if (sscanf(line.c_str(), "page id=%d file=%64s\n", &id, file) != 2) {
            return 0;
        }
        file[strlen(file) - 1] = 0;
        KdString path(name);
        // STRING: VC1 0x004d5c3c
        path.Assign(path.Slice(0, path.FindLastOf(KdString("/"), static_cast<unsigned>(-1)) + 1));
        path.Append(file + 1, static_cast<unsigned>(strlen(file + 1)));
        {
            KdString full(path.c_str());
            KdRef<IResourceManager> manager;
            IUnknown** slot;
            IUnknown* const* loaded;

            IResourceManager* loader = *GetKdResourceManager(&manager.ptr);
            slot = reinterpret_cast<IUnknown**>(pages.items.begin) + id;
            KdPtr<IUnknown> resource(loader->GetTexture(full));
            loaded = &resource.ptr;
            if (*slot) {
                (*slot)->Release();
            }
            if (*loaded) {
                *slot = static_cast<IUnknown*>((*loaded)->QueryInterface(&g_iidTexture));
                if (*slot) {
                    (*slot)->AddRef();
                }
            }
            else {
                *slot = 0;
            }
        }
        if (!reinterpret_cast<IUnknown**>(pages.items.begin)[id]) {
            return 0;
        }
    }
    reader.ReadLine(line);
    // STRING: VC1 0x004d5c40
    if (sscanf(line.c_str(), "chars count=%d\n", &count) != 1) {
        return 0;
    }
    for (index = 0; index < count; ++index) {
        int code;
        GlyphInfo info;
        int channel;

        reader.ReadLine(line);
        if (sscanf(line.c_str(),
                // STRING: VC1 0x004d5c50
                "char id=%d x=%d y=%d width=%d height=%d xoffset=%d yoffset=%d xadvance=%d page=%d chnl=%d\n",
                &code, &info.u, &info.v, &info.width, &info.height, &info.offsetX,
                &info.offsetY, &info.advance, &info.page, &channel) != 10) {
            return 0;
        }
        info.advance += charSpacing;
        const unsigned key = code;
        *InsertGlyph(&glyphs, &key) = info;
    }
    // STRING: VC1 0x004d5cac
    HostLog(&g_kdNameFont, "Loaded %s (height: %i glyphs: %i)", name, *height,
        glyphs.size / sizeof(Glyph));
    return 1;
}

}
