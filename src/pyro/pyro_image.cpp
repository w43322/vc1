#include "pyro/pyro_crt.h"
#include "pyro/pyro_image.h"

#include <string.h>

#include "png.h"

namespace vc1 {

// The game's libpng configuration (cmake/libpng.cmake): malloc(0x168) in
// png_create_read_struct, malloc(0x48) in the inlined png_create_info_struct.
typedef char Vc1PngStructSizeCheck[sizeof(png_struct) == 0x168 ? 1 : -1];
typedef char Vc1PngInfoSizeCheck[sizeof(png_info) == 0x48 ? 1 : -1];

// Inferred names. Header of the last image sniffed as TGA; the TGA decoder
// continues from it.
#pragma pack(push, 1)
struct PyroTgaHeader
{
    unsigned char idLength;        // 0x00
    unsigned char colorMapType;    // 0x01
    unsigned char imageType;       // 0x02
    unsigned short colorMapStart;  // 0x03
    unsigned short colorMapLength; // 0x05
    unsigned char colorMapDepth;   // 0x07
    short xOrigin;                 // 0x08
    short yOrigin;                 // 0x0a
    short width;                   // 0x0c
    short height;                  // 0x0e
    unsigned char depth;           // 0x10
    unsigned char descriptor;      // 0x11
};
#pragma pack(pop)

typedef char Vc1PyroTgaHeaderSizeCheck[sizeof(PyroTgaHeader) == 0x12 ? 1 : -1];

// GLOBAL: VC1 0x004ed10c
PyroTgaHeader g_pyroTgaHeader;

static bool PyroDecodeDds(PyroImage* image, PyroImageStream* stream);
static bool PyroDecodePsd(PyroImageStream* stream, PyroImage* image);
static bool PyroDecodeTga(PyroImageStream* stream, PyroImage* image);

// "return [this+4]", shared with other getters.
// FUNCTION: VC1 0x0040f130 FOLDED
int PyroMemoryStream::GetLength()
{
    return size;
}

// FUNCTION: VC1 0x0042f140 FOLDED
int PyroMemoryStream::Tell()
{
    return position;
}

// Run-length state of the PSD and TGA decoders (one image at a time).
// GLOBAL: VC1 0x004ed108
int g_pyroTgaPixelSize;
// GLOBAL: VC1 0x004ed11e
bool g_pyroPsdRle;
// GLOBAL: VC1 0x004ed11f
bool g_pyroTgaRle;
// GLOBAL: VC1 0x004ed120
unsigned g_pyroTgaRunPixel;
// GLOBAL: VC1 0x004ed124
PyroImageStream* g_pyroPsdStream;
// GLOBAL: VC1 0x004ed128
bool g_pyroTgaRunRepeat;
// GLOBAL: VC1 0x004ed129
bool g_pyroPsdRunRepeat;
// GLOBAL: VC1 0x004ed12c
int g_pyroTgaRunLeft;
// GLOBAL: VC1 0x004ed130
unsigned short g_pyroPsdRunValue;
// GLOBAL: VC1 0x004ed134
int g_pyroPsdRunLeft;
// GLOBAL: VC1 0x004ed138
PyroImageStream* g_pyroTgaStream;
// GLOBAL: VC1 0x004eb0f4
int g_pyroPsdSampleSize;

// EAX=mask, ECX=shiftRight, ESI=shiftLeft. DevIL GetBitsFromMask.
// FUNCTION: VC1 0x00449f70
static void PyroDdsBitsFromMask(unsigned mask, unsigned* shiftRight, unsigned* shiftLeft)
{
    unsigned i;

    if (mask == 0) {
        *shiftRight = 0;
        *shiftLeft = 0;
        return;
    }
    for (i = 0; i < 32; ++i, mask >>= 1) {
        if (mask & 1) {
            break;
        }
    }
    *shiftRight = i;
    for (i = 0; i < 8; ++i, mask >>= 1) {
        if (!(mask & 1)) {
            break;
        }
    }
    *shiftLeft = 8 - i;
}

// ESI=out. One PSD sample (1 or 2 bytes); PackBits packets when
// compressed: 0x80 and above repeat the next sample 0x101 - n times, below
// copy n + 1 samples.
// FUNCTION: VC1 0x00449fb0
static void PyroPsdReadSample(unsigned short* out)
{
    unsigned char packet;

    *out = 0;
    if (!g_pyroPsdRle) {
        g_pyroPsdStream->Read(out, g_pyroPsdSampleSize);
        return;
    }
    if (g_pyroPsdRunLeft == 0) {
        g_pyroPsdStream->Read(&packet, 1);
        if (packet >= 0x80) {
            g_pyroPsdRunLeft = 0x101 - packet;
            g_pyroPsdRunRepeat = true;
        }
        else if (packet <= 0x7f) {
            g_pyroPsdRunLeft = packet + 1;
            g_pyroPsdRunRepeat = false;
        }
        g_pyroPsdStream->Read(out, g_pyroPsdSampleSize);
        --g_pyroPsdRunLeft;
        g_pyroPsdRunValue = *out;
        return;
    }
    if (!g_pyroPsdRunRepeat) {
        g_pyroPsdStream->Read(out, g_pyroPsdSampleSize);
        --g_pyroPsdRunLeft;
        return;
    }
    --g_pyroPsdRunLeft;
    *out = g_pyroPsdRunValue;
}

// png_create_read_struct warning callback: ignored.
// FUNCTION: VC1 0x0044a090
static void PngWarning(png_structp png, png_const_charp message)
{
    (void)png;
    (void)message;
}

// png_create_read_struct error callback.
// FUNCTION: VC1 0x0044a0a0
static void PngError(png_structp png, png_const_charp message)
{
    (void)message;
    longjmp(png->jmpbuf, 1);
}

// ESI=stream. Reads the 18-byte header into g_pyroTgaHeader; a plausible
// depth and size leave the stream after it, otherwise it is rewound (a short
// read is not).
// FUNCTION: VC1 0x0044a0b0
static bool PyroIsTga(PyroImageStream* stream)
{
    bool failed;
    const int position = stream->Tell();

    if (stream->Read(&g_pyroTgaHeader, sizeof(g_pyroTgaHeader)) != sizeof(g_pyroTgaHeader)) {
        return false;
    }
    if ((g_pyroTgaHeader.depth == 8 || g_pyroTgaHeader.depth == 16 ||
            g_pyroTgaHeader.depth == 24 || g_pyroTgaHeader.depth == 32) &&
        g_pyroTgaHeader.width > 0 && g_pyroTgaHeader.height > 0) {
        failed = false;
    }
    else {
        stream->Seek(position, 0);
        failed = true;
    }
    return !failed;
}

// ESI=out. One TGA pixel of g_pyroTgaPixelSize bytes; RLE packets above
// 0x7f repeat the next pixel n - 0x7f times, others copy n + 1 pixels.
// FUNCTION: VC1 0x0044a130
static void PyroTgaReadPixel(void* out)
{
    unsigned char packet;

    if (!g_pyroTgaRle) {
        g_pyroTgaStream->Read(out, g_pyroTgaPixelSize);
        return;
    }
    if (g_pyroTgaRunLeft == 0) {
        g_pyroTgaStream->Read(&packet, 1);
        if (packet > 0x7f) {
            g_pyroTgaRunLeft = packet - 0x7f;
            g_pyroTgaRunRepeat = true;
        }
        else {
            g_pyroTgaRunLeft = packet + 1;
            g_pyroTgaRunRepeat = false;
        }
        g_pyroTgaStream->Read(out, g_pyroTgaPixelSize);
        memcpy(&g_pyroTgaRunPixel, out, g_pyroTgaPixelSize);
        --g_pyroTgaRunLeft;
        return;
    }
    if (!g_pyroTgaRunRepeat) {
        g_pyroTgaStream->Read(out, g_pyroTgaPixelSize);
    }
    else {
        memcpy(out, &g_pyroTgaRunPixel, g_pyroTgaPixelSize);
    }
    --g_pyroTgaRunLeft;
}

// libpng read callback over the image stream in io_ptr.
// FUNCTION: VC1 0x0044a210
static void PngReadData(png_structp png, png_bytep data, png_size_t length)
{
    PyroImageStream* const stream = static_cast<PyroImageStream*>(png->io_ptr);

    if (stream->Read(data, static_cast<int>(length)) != static_cast<int>(length)) {
        // STRING: VC1 0x004d804c
        png_error(png, "read Error");
    }
}

// EAX=stream. Peeks two dwords: 6 PNG, 1 JPEG (JFIF), 5 PSD, 4 DDS, 2 JPEG
// 2000, else 3 when the TGA header check passes, 0 otherwise.
// FUNCTION: VC1 0x0044a240
static int PyroDetectImageFormat(PyroImageStream* stream)
{
    unsigned magic[2];
    const int position = stream->Tell();

    if (stream->Read(&magic[0], 4) != 4) {
        stream->Seek(position, 0);
        return 0;
    }
    if (stream->Read(&magic[1], 4) != 4) {
        stream->Seek(position, 0);
        return 0;
    }
    stream->Seek(position, 0);
    if (magic[0] == 0x474e5089) {
        return 6;
    }
    if (magic[0] == 0xe0ffd8ff) {
        return 1;
    }
    if (magic[0] == 0x53504238) {
        return 5;
    }
    if (magic[0] == 0x20534444) {
        return 4;
    }
    if (magic[1] == 0x2020506a) {
        return 2;
    }
    return PyroIsTga(stream) ? 3 : 0;
}

// The DDS decoder is DevIL's il_dds.c (DecodePixelFormat,
// AdjustVolumeTexture, ReadData, DecompressARGB / DXT1 / DXT3 / DXT5,
// CorrectPreMult) inlined into one function; names follow DevIL. Unlike
// DevIL, every depth slice is written over the first one and the output row
// stride is the read pitch.

// SIZE 0x80
struct PyroDdsHeader
{
    char signature[4];       // 0x00
    unsigned size1;          // 0x04  0x7c
    unsigned flags1;         // 0x08  0x08 pitch, 0x80000 linear size
    unsigned height;         // 0x0c
    unsigned width;          // 0x10
    unsigned linearSize;     // 0x14
    unsigned depth;          // 0x18
    unsigned mipMapCount;    // 0x1c
    unsigned alphaBitDepth;  // 0x20
    unsigned notUsed[10];    // 0x24
    unsigned size2;          // 0x4c  0x20
    unsigned flags2;         // 0x50  1 alpha pixels, 4 FourCC
    unsigned fourCC;         // 0x54
    unsigned rgbBitCount;    // 0x58
    unsigned rBitMask;       // 0x5c
    unsigned gBitMask;       // 0x60
    unsigned bBitMask;       // 0x64
    unsigned alphaBitMask;   // 0x68
    unsigned caps1;          // 0x6c  8 complex
    unsigned caps2;          // 0x70  0x200000 volume
    unsigned caps3;          // 0x74
    unsigned caps4;          // 0x78
    unsigned textureStage;   // 0x7c
};

typedef char Vc1PyroDdsHeaderSizeCheck[sizeof(PyroDdsHeader) == 0x80 ? 1 : -1];

// DevIL IL_MAX.
#define PYRO_DDS_MAX(a, b) ((a) > (b) ? (a) : (b))

// DevIL's compressed formats.
enum {
    kDdsArgb = 0,
    kDdsRgb = 1,
    kDdsDxt1 = 2,
    kDdsDxt2 = 3,
    kDdsDxt3 = 4,
    kDdsDxt4 = 5,
    kDdsDxt5 = 6,
    kDdsUnknown = 7
};

// DevIL Color8888.
struct PyroDdsColor
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

// DevIL DxtcReadColor: a 5:6:5 colour, low bits dropped.
static __forceinline void PyroDdsReadColor(unsigned short data, PyroDdsColor* out)
{
    out->r = static_cast<unsigned char>((data >> 8) & 0xf8);
    out->g = static_cast<unsigned char>((data >> 5) << 2);
    out->b = static_cast<unsigned char>(data << 3);
}

// colours[2] and [3] of a four-colour block, in DevIL's b, g, r order.
static __forceinline void PyroDdsMixColors(PyroDdsColor* colours)
{
    colours[2].b = static_cast<unsigned char>((2 * colours[0].b + colours[1].b + 1) / 3);
    colours[2].g = static_cast<unsigned char>((2 * colours[0].g + colours[1].g + 1) / 3);
    colours[2].r = static_cast<unsigned char>((2 * colours[0].r + colours[1].r + 1) / 3);
    colours[3].b = static_cast<unsigned char>((colours[0].b + 2 * colours[1].b + 1) / 3);
    colours[3].g = static_cast<unsigned char>((colours[0].g + 2 * colours[1].g + 1) / 3);
    colours[3].r = static_cast<unsigned char>((colours[0].r + 2 * colours[1].r + 1) / 3);
}

// DevIL CorrectPreMult over the decoded pixels.
static __forceinline void PyroDdsCorrectPreMult(PyroImage* image)
{
    unsigned char* data = static_cast<unsigned char*>(image->pixels);
    unsigned count = image->width * image->height;

    while (count--) {
        if (data[3] != 0) {
            const unsigned alpha = data[3];
            data[0] = static_cast<unsigned char>((static_cast<unsigned>(data[0]) << 8) / alpha);
            data[1] = static_cast<unsigned char>((static_cast<unsigned>(data[1]) << 8) / alpha);
            data[2] = static_cast<unsigned char>((static_cast<unsigned>(data[2]) << 8) / alpha);
        }
        data += 4;
    }
}

// EAX=image, stack stream, RET 4. DDS decoder: uncompressed RGB / ARGB by
// bit masks and DXT1-5.
// FUNCTION: VC1 0x0044a310
static bool PyroDecodeDds(PyroImage* image, PyroImageStream* stream)
{
    PyroDdsHeader header;
    PyroDdsColor colours[4];
    const PyroDdsColor* col;
    unsigned char alphas[8];
    int format;
    unsigned size;
    unsigned depth;
    unsigned channels;
    unsigned stride;
    unsigned char* data;
    unsigned char* src;
    unsigned char* out;
    unsigned x;
    unsigned y;
    unsigned z;
    unsigned i;
    unsigned j;

    if (stream->Read(&header, sizeof(header)) != sizeof(header)) {
        return false;
    }
    if (header.depth == 0) {
        header.depth = 1;
    }
    depth = header.depth;
    if (header.size1 != 0x7c || header.size2 != 0x20 || header.width == 0 || header.height == 0) {
        return false;
    }

    // DecodePixelFormat.
    if (header.flags2 & 4) {
        const unsigned blocks = ((header.width + 3) >> 2) * ((header.height + 3) >> 2)
            * ((header.depth + 3) >> 2);
        switch (header.fourCC) {
        case 0x31545844:
            format = kDdsDxt1;
            size = blocks * 8;
            break;
        case 0x32545844:
            format = kDdsDxt2;
            size = blocks * 16;
            break;
        case 0x33545844:
            format = kDdsDxt3;
            size = blocks * 16;
            break;
        case 0x34545844:
            format = kDdsDxt4;
            size = blocks * 16;
            break;
        case 0x35545844:
            format = kDdsDxt5;
            size = blocks * 16;
            break;
        default:
            format = kDdsUnknown;
            size = blocks * 16;
            break;
        }
    }
    else {
        format = (header.flags2 & 1) ? kDdsArgb : kDdsRgb;
        size = (header.rgbBitCount >> 3) * header.height * header.width * header.depth;
    }
    if (!(header.flags1 & 0x80008)) {
        header.flags1 |= 0x80000;
        header.linearSize = size;
    }
    image->width = header.width;
    image->height = header.height;

    // AdjustVolumeTexture.
    if (header.depth > 1) {
        if (!(header.caps1 & 8) || !(header.caps2 & 0x200000)) {
            header.depth = 1;
        }
        switch (format) {
        case kDdsArgb:
        case kDdsRgb:
            header.linearSize = PYRO_DDS_MAX(1, header.width) * PYRO_DDS_MAX(1, header.height)
                * (header.rgbBitCount >> 3);
            break;
        case kDdsDxt1:
            header.linearSize = PYRO_DDS_MAX(1, header.width >> 2) * PYRO_DDS_MAX(1, header.height >> 2) * 8;
            break;
        case kDdsDxt2:
        case kDdsDxt3:
        case kDdsDxt4:
        case kDdsDxt5:
            header.linearSize = PYRO_DDS_MAX(1, header.width >> 2) * PYRO_DDS_MAX(1, header.height >> 2) * 16;
            break;
        }
        header.flags1 |= 0x80000;
        header.linearSize *= header.depth;
    }
    channels = (format != kDdsRgb) + 3;

    // ReadData.
    if (!(header.flags1 & 0x80000)) {
        stride = (header.rgbBitCount * header.width) >> 3;
        data = static_cast<unsigned char*>(malloc(stride * depth * header.height));
        if (!data) {
            return false;
        }
        src = data;
        for (z = 0; z < depth; ++z) {
            const unsigned slice = image->height * stride;
            if (static_cast<unsigned>(stream->Read(src, slice)) != slice) {
                free(data);
                return false;
            }
            src += slice;
        }
    }
    else {
        stride = channels * header.width;
        data = static_cast<unsigned char*>(malloc(header.linearSize));
        if (!data) {
            return false;
        }
        if (static_cast<unsigned>(stream->Read(data, header.linearSize)) != header.linearSize) {
            free(data);
            return false;
        }
    }
    image->pixels = malloc(image->width * image->height * 4);
    out = static_cast<unsigned char*>(image->pixels);
    src = data;

    if (format == kDdsArgb || format == kDdsRgb) {
        // DecompressARGB.
        unsigned count = image->width * image->height;
        unsigned redRight;
        unsigned redLeft;
        unsigned greenRight;
        unsigned greenLeft;
        unsigned blueRight;
        unsigned blueLeft;
        unsigned alphaRight;
        unsigned alphaLeft;

        PyroDdsBitsFromMask(header.rBitMask, &redRight, &redLeft);
        PyroDdsBitsFromMask(header.gBitMask, &greenRight, &greenLeft);
        PyroDdsBitsFromMask(header.bBitMask, &blueRight, &blueLeft);
        PyroDdsBitsFromMask(header.alphaBitMask, &alphaRight, &alphaLeft);
        while (count--) {
            const unsigned value = *reinterpret_cast<unsigned*>(src);
            src += header.rgbBitCount >> 3;
            out[0] = static_cast<unsigned char>(((value & header.rBitMask) >> redRight) << redLeft);
            out[1] = static_cast<unsigned char>(((value & header.gBitMask) >> greenRight) << greenLeft);
            out[2] = static_cast<unsigned char>(((value & header.bBitMask) >> blueRight) << blueLeft);
            if (channels == 4) {
                const unsigned char alpha = static_cast<unsigned char>(
                    ((value & header.alphaBitMask) >> alphaRight) << alphaLeft);
                out[3] = alpha;
                if (alphaLeft >= 7) {
                    out[3] = alpha ? 0xff : 0;
                }
                else if (alphaLeft > 3) {
                    out[3] = static_cast<unsigned char>(alpha | (alpha >> 4));
                }
            }
            else {
                out[3] = 0xff;
            }
            out += 4;
        }
    }
    else if (format == kDdsDxt1) {
        // DecompressDXT1.
        for (z = 0; z < depth; ++z) {
            for (y = 0; y < image->height; y += 4) {
                for (x = 0; x < image->width; x += 4) {
                    const unsigned short color0 = *reinterpret_cast<unsigned short*>(src);
                    const unsigned short color1 = *reinterpret_cast<unsigned short*>(src + 2);
                    const unsigned bitmask = *reinterpret_cast<unsigned*>(src + 4);
                    unsigned k = 0;

                    src += 8;
                    PyroDdsReadColor(color0, colours);
                    PyroDdsReadColor(color1, colours + 1);
                    colours[0].a = 0xff;
                    colours[1].a = 0xff;
                    colours[2].a = 0xff;
                    if (color0 > color1) {
                        PyroDdsMixColors(colours);
                        colours[3].a = 0xff;
                    }
                    else {
                        colours[2].b = static_cast<unsigned char>((colours[0].b + colours[1].b) / 2);
                        colours[2].g = static_cast<unsigned char>((colours[0].g + colours[1].g) / 2);
                        colours[2].r = static_cast<unsigned char>((colours[0].r + colours[1].r) / 2);
                        colours[3].b = static_cast<unsigned char>((colours[0].b + 2 * colours[1].b + 1) / 3);
                        colours[3].g = static_cast<unsigned char>((colours[0].g + 2 * colours[1].g + 1) / 3);
                        colours[3].r = static_cast<unsigned char>((colours[0].r + 2 * colours[1].r + 1) / 3);
                        colours[3].a = 0;
                    }
                    for (j = 0; j < 4; ++j) {
                        for (i = 0; i < 4; ++i, ++k) {
                            const unsigned select = (bitmask & (3 << k * 2)) >> k * 2;
                            col = &colours[select];
                            if (x + i < image->width && y + j < image->height) {
                                const unsigned offset = (y + j) * stride + (x + i) * channels;
                                static_cast<unsigned char*>(image->pixels)[offset + 0] = col->r;
                                static_cast<unsigned char*>(image->pixels)[offset + 1] = col->g;
                                static_cast<unsigned char*>(image->pixels)[offset + 2] = col->b;
                                static_cast<unsigned char*>(image->pixels)[offset + 3] = col->a;
                            }
                        }
                    }
                }
            }
        }
    }
    else if (format == kDdsDxt2 || format == kDdsDxt3) {
        // DecompressDXT3.
        for (z = 0; z < depth; ++z) {
            for (y = 0; y < image->height; y += 4) {
                for (x = 0; x < image->width; x += 4) {
                    const unsigned short* const alpha = reinterpret_cast<unsigned short*>(src);
                    const unsigned short color0 = *reinterpret_cast<unsigned short*>(src + 8);
                    const unsigned short color1 = *reinterpret_cast<unsigned short*>(src + 10);
                    const unsigned bitmask = *reinterpret_cast<unsigned*>(src + 12);
                    unsigned k = 0;

                    src += 16;
                    PyroDdsReadColor(color0, colours);
                    PyroDdsReadColor(color1, colours + 1);
                    colours[0].a = 0xff;
                    colours[1].a = 0xff;
                    colours[2].a = 0xff;
                    colours[3].a = 0xff;
                    PyroDdsMixColors(colours);
                    for (j = 0; j < 4; ++j) {
                        for (i = 0; i < 4; ++i, ++k) {
                            const unsigned select = (bitmask & (3 << k * 2)) >> k * 2;
                            col = &colours[select];
                            if (x + i < image->width && y + j < image->height) {
                                const unsigned offset = (y + j) * stride + (x + i) * channels;
                                static_cast<unsigned char*>(image->pixels)[offset + 0] = col->r;
                                static_cast<unsigned char*>(image->pixels)[offset + 1] = col->g;
                                static_cast<unsigned char*>(image->pixels)[offset + 2] = col->b;
                            }
                        }
                    }
                    for (j = 0; j < 4; ++j) {
                        unsigned word = alpha[j];
                        for (i = 0; i < 4; ++i) {
                            if (x + i < image->width && y + j < image->height) {
                                const unsigned offset = (y + j) * stride + (x + i) * channels;
                                const unsigned char low = static_cast<unsigned char>(word & 0x0f);
                                static_cast<unsigned char*>(image->pixels)[offset + 3] = static_cast<unsigned char>(low | (word << 4));
                            }
                            word >>= 4;
                        }
                    }
                }
            }
        }
        if (format == kDdsDxt2) {
            PyroDdsCorrectPreMult(image);
        }
    }
    else if (format == kDdsDxt4 || format == kDdsDxt5) {
        // DecompressDXT5.
        for (z = 0; z < depth; ++z) {
            for (y = 0; y < image->height; y += 4) {
                for (x = 0; x < image->width; x += 4) {
                    const unsigned char* const alphamask = src + 2;
                    const unsigned short color0 = *reinterpret_cast<unsigned short*>(src + 8);
                    const unsigned short color1 = *reinterpret_cast<unsigned short*>(src + 10);
                    const unsigned bitmask = *reinterpret_cast<unsigned*>(src + 12);
                    unsigned bits;
                    unsigned k = 0;

                    alphas[0] = src[0];
                    alphas[1] = src[1];
                    PyroDdsReadColor(color0, colours);
                    PyroDdsReadColor(color1, colours + 1);
                    colours[0].a = 0xff;
                    colours[1].a = 0xff;
                    colours[2].a = 0xff;
                    colours[3].a = 0xff;
                    PyroDdsMixColors(colours);
                    for (j = 0; j < 4; ++j) {
                        for (i = 0; i < 4; ++i, ++k) {
                            const unsigned select = (bitmask & (3 << k * 2)) >> k * 2;
                            col = &colours[select];
                            if (x + i < image->width && y + j < image->height) {
                                const unsigned offset = (y + j) * stride + (x + i) * channels;
                                static_cast<unsigned char*>(image->pixels)[offset + 0] = col->r;
                                static_cast<unsigned char*>(image->pixels)[offset + 1] = col->g;
                                static_cast<unsigned char*>(image->pixels)[offset + 2] = col->b;
                            }
                        }
                    }
                    if (alphas[0] > alphas[1]) {
                        alphas[2] = static_cast<unsigned char>((6 * alphas[0] + 1 * alphas[1] + 3) / 7);
                        alphas[3] = static_cast<unsigned char>((5 * alphas[0] + 2 * alphas[1] + 3) / 7);
                        alphas[4] = static_cast<unsigned char>((4 * alphas[0] + 3 * alphas[1] + 3) / 7);
                        alphas[5] = static_cast<unsigned char>((3 * alphas[0] + 4 * alphas[1] + 3) / 7);
                        alphas[6] = static_cast<unsigned char>((2 * alphas[0] + 5 * alphas[1] + 3) / 7);
                        alphas[7] = static_cast<unsigned char>((1 * alphas[0] + 6 * alphas[1] + 3) / 7);
                    }
                    else {
                        alphas[2] = static_cast<unsigned char>((4 * alphas[0] + 1 * alphas[1] + 2) / 5);
                        alphas[3] = static_cast<unsigned char>((3 * alphas[0] + 2 * alphas[1] + 2) / 5);
                        alphas[4] = static_cast<unsigned char>((2 * alphas[0] + 3 * alphas[1] + 2) / 5);
                        alphas[5] = static_cast<unsigned char>((1 * alphas[0] + 4 * alphas[1] + 2) / 5);
                        alphas[6] = 0x00;
                        alphas[7] = 0xff;
                    }
                    bits = *reinterpret_cast<const unsigned*>(alphamask);
                    for (j = 0; j < 2; ++j) {
                        for (i = 0; i < 4; ++i) {
                            if (x + i < image->width && y + j < image->height) {
                                static_cast<unsigned char*>(image->pixels)[(y + j) * stride + (x + i) * channels + 3] = alphas[bits & 7];
                            }
                            bits >>= 3;
                        }
                    }
                    bits = *reinterpret_cast<const unsigned*>(alphamask + 3);
                    for (j = 2; j < 4; ++j) {
                        for (i = 0; i < 4; ++i) {
                            if (x + i < image->width && y + j < image->height) {
                                static_cast<unsigned char*>(image->pixels)[(y + j) * stride + (x + i) * channels + 3] = alphas[bits & 7];
                            }
                            bits >>= 3;
                        }
                    }
                    src += 16;
                }
            }
        }
        if (format == kDdsDxt4) {
            PyroDdsCorrectPreMult(image);
        }
    }
    free(data);
    return true;
}

// The PSD file header, big-endian.
#pragma pack(push, 1)
struct PyroPsdHeader
{
    unsigned signature;        // 0x00
    unsigned short version;    // 0x04
    unsigned char reserved[6]; // 0x06
    unsigned short channels;   // 0x0c
    unsigned rows;             // 0x0e
    unsigned columns;          // 0x12
    unsigned short depth;      // 0x16
    unsigned short mode;       // 0x18
};
#pragma pack(pop)

typedef char Vc1PyroPsdHeaderSizeCheck[sizeof(PyroPsdHeader) == 0x1a ? 1 : -1];

static unsigned short PyroSwap16(unsigned short value)
{
    return static_cast<unsigned short>((value << 8) | (value >> 8));
}

static unsigned PyroSwap32(unsigned value)
{
    return (((value & 0xff0000) | (value >> 16)) >> 8) | (((value << 16) | (value & 0xff00)) << 8);
}

// EAX=stream, stack image, RET 4. PSD decoder: grayscale, indexed, RGB (a
// fourth channel is alpha, more are skipped) and CMYK (multiplied by K, a
// fifth channel is alpha), 8 or 16 bits per sample with optional PackBits.
// The indexed palette is never freed, as in the original.
// FUNCTION: VC1 0x0044b850
static bool PyroDecodePsd(PyroImageStream* stream, PyroImage* image)
{
    PyroPsdHeader header;
    unsigned length;
    unsigned resourceLength;
    unsigned layerLength;
    unsigned short compression;
    unsigned* palette = 0;
    unsigned char byte;
    // PyroPsdReadSample fills the low half.
    unsigned sample;
    unsigned char* pixels;
    int count;
    int i;
    int c;

    if (stream->Read(&header, sizeof(header)) != sizeof(header)) {
        return false;
    }
    header.version = PyroSwap16(header.version);
    header.channels = PyroSwap16(header.channels);
    header.depth = PyroSwap16(header.depth);
    header.mode = PyroSwap16(header.mode);
    header.columns = PyroSwap32(header.columns);
    header.rows = PyroSwap32(header.rows);
    if (header.version != 1 || static_cast<unsigned short>(header.channels - 1) > 0x17
        || header.rows < 1 || header.columns < 1
        || (header.depth != 1 && header.depth != 8 && header.depth != 16)) {
        return false;
    }
    image->Clear();
    image->height = header.rows;
    image->width = header.columns;
    count = header.rows * header.columns;
    stream->Read(&length, 4);
    g_pyroPsdRunRepeat = false;
    g_pyroPsdRunLeft = 0;
    length = PyroSwap32(length);
    if (header.mode == 2) {
        if (length % 3) {
            return false;
        }
        palette = static_cast<unsigned*>(malloc(length * 4));
        for (c = 0; c < static_cast<int>(length) / 3; ++c) {
            stream->Read(&byte, 1);
            palette[c] = byte;
        }
        for (c = 0; c < static_cast<int>(length) / 3; ++c) {
            stream->Read(&byte, 1);
            palette[c] |= byte << 8;
        }
        for (c = 0; c < static_cast<int>(length) / 3; ++c) {
            stream->Read(&byte, 1);
            palette[c] |= byte << 16;
        }
    }
    stream->Read(&resourceLength, 4);
    resourceLength = PyroSwap32(resourceLength);
    stream->Seek(resourceLength, 1);
    stream->Read(&layerLength, 4);
    layerLength = PyroSwap32(layerLength);
    stream->Seek(layerLength, 1);
    stream->Read(&compression, 2);
    compression = PyroSwap16(compression);
    g_pyroPsdRle = compression != 0;
    g_pyroPsdStream = stream;
    if (header.depth == 8) {
        g_pyroPsdSampleSize = 1;
    }
    else if (header.depth == 16) {
        g_pyroPsdSampleSize = 2;
    }
    else {
        image->Clear();
        return false;
    }
    if (compression) {
        stream->Seek(header.channels * image->height * 2, 1);
    }
    pixels = static_cast<unsigned char*>(malloc(image->width * image->height * 4));
    image->pixels = pixels;

    // Every plane loop exists once per sample size: 16-bit samples keep the
    // first (high) byte through sample &= 0xff.
    switch (header.mode) {
    case 1:
        if (header.channels != 1 && header.depth != 8) {
            break;
        }
        sample = 0;
        if (g_pyroPsdSampleSize == 1) {
            for (i = 0; i < count; ++i) {
                PyroPsdReadSample(reinterpret_cast<unsigned short*>(&sample));
                const unsigned char value = static_cast<unsigned char>(sample);
                pixels[i * 4 + 0] = value;
                pixels[i * 4 + 1] = value;
                pixels[i * 4 + 2] = value;
                pixels[i * 4 + 3] = 0xff;
            }
        }
        else {
            for (i = 0; i < count; ++i) {
                PyroPsdReadSample(reinterpret_cast<unsigned short*>(&sample));
                const unsigned char value = static_cast<unsigned char>(sample &= 0xff);
                pixels[i * 4 + 0] = value;
                pixels[i * 4 + 1] = value;
                pixels[i * 4 + 2] = value;
                pixels[i * 4 + 3] = 0xff;
            }
        }
        return true;
    case 2:
        if (header.channels != 1 && header.depth != 8) {
            break;
        }
        sample = 0;
        if (g_pyroPsdSampleSize == 1) {
            for (i = 0; i < count; ++i) {
                PyroPsdReadSample(reinterpret_cast<unsigned short*>(&sample));
                static_cast<unsigned*>(image->pixels)[i] = palette[static_cast<unsigned short>(sample)];
            }
        }
        else {
            for (i = 0; i < count; ++i) {
                PyroPsdReadSample(reinterpret_cast<unsigned short*>(&sample));
                sample = PyroSwap16(static_cast<unsigned short>(sample));
                static_cast<unsigned*>(image->pixels)[i] = palette[static_cast<unsigned short>(sample)];
            }
        }
        return true;
    case 3:
        sample = 0;
        if (g_pyroPsdSampleSize == 1) {
            for (i = 0; i < count; ++i) {
                PyroPsdReadSample(reinterpret_cast<unsigned short*>(&sample));
                pixels[i * 4 + 0] = static_cast<unsigned char>(sample);
            }
            for (i = 0; i < count; ++i) {
                PyroPsdReadSample(reinterpret_cast<unsigned short*>(&sample));
                pixels[i * 4 + 1] = static_cast<unsigned char>(sample);
            }
            if (header.channels == 3) {
                for (i = 0; i < count; ++i) {
                    PyroPsdReadSample(reinterpret_cast<unsigned short*>(&sample));
                    pixels[i * 4 + 2] = static_cast<unsigned char>(sample);
                    pixels[i * 4 + 3] = 0xff;
                }
                return true;
            }
            if (header.channels < 4) {
                break;
            }
            for (i = 0; i < count; ++i) {
                PyroPsdReadSample(reinterpret_cast<unsigned short*>(&sample));
                pixels[i * 4 + 2] = static_cast<unsigned char>(sample);
            }
            for (i = 0; i < count; ++i) {
                PyroPsdReadSample(reinterpret_cast<unsigned short*>(&sample));
                pixels[i * 4 + 3] = static_cast<unsigned char>(sample);
            }
            for (c = 0; c < header.channels - 4; ++c) {
                for (i = 0; i < count; ++i) {
                    PyroPsdReadSample(reinterpret_cast<unsigned short*>(&sample));
                }
            }
            return true;
        }
        else {
            for (i = 0; i < count; ++i) {
                PyroPsdReadSample(reinterpret_cast<unsigned short*>(&sample));
                pixels[i * 4 + 0] = static_cast<unsigned char>(sample &= 0xff);
            }
            for (i = 0; i < count; ++i) {
                PyroPsdReadSample(reinterpret_cast<unsigned short*>(&sample));
                pixels[i * 4 + 1] = static_cast<unsigned char>(sample &= 0xff);
            }
            if (header.channels == 3) {
                for (i = 0; i < count; ++i) {
                    PyroPsdReadSample(reinterpret_cast<unsigned short*>(&sample));
                    pixels[i * 4 + 2] = static_cast<unsigned char>(sample &= 0xff);
                    pixels[i * 4 + 3] = 0xff;
                }
                return true;
            }
            if (header.channels < 4) {
                break;
            }
            for (i = 0; i < count; ++i) {
                PyroPsdReadSample(reinterpret_cast<unsigned short*>(&sample));
                pixels[i * 4 + 2] = static_cast<unsigned char>(sample &= 0xff);
            }
            for (i = 0; i < count; ++i) {
                PyroPsdReadSample(reinterpret_cast<unsigned short*>(&sample));
                pixels[i * 4 + 3] = static_cast<unsigned char>(sample &= 0xff);
            }
            for (c = 0; c < header.channels - 4; ++c) {
                for (i = 0; i < count; ++i) {
                    PyroPsdReadSample(reinterpret_cast<unsigned short*>(&sample));
                }
            }
            return true;
        }
    case 4:
        sample = 0;
        if (g_pyroPsdSampleSize == 1) {
            for (i = 0; i < count; ++i) {
                PyroPsdReadSample(reinterpret_cast<unsigned short*>(&sample));
                pixels[i * 4 + 0] = static_cast<unsigned char>(sample);
            }
            for (i = 0; i < count; ++i) {
                PyroPsdReadSample(reinterpret_cast<unsigned short*>(&sample));
                pixels[i * 4 + 1] = static_cast<unsigned char>(sample);
            }
            for (i = 0; i < count; ++i) {
                PyroPsdReadSample(reinterpret_cast<unsigned short*>(&sample));
                pixels[i * 4 + 2] = static_cast<unsigned char>(sample);
            }
            for (i = 0; i < count; ++i) {
                PyroPsdReadSample(reinterpret_cast<unsigned short*>(&sample));
                pixels[i * 4 + 3] = static_cast<unsigned char>(sample);
            }
        }
        else {
            for (i = 0; i < count; ++i) {
                PyroPsdReadSample(reinterpret_cast<unsigned short*>(&sample));
                pixels[i * 4 + 0] = static_cast<unsigned char>(sample &= 0xff);
            }
            for (i = 0; i < count; ++i) {
                PyroPsdReadSample(reinterpret_cast<unsigned short*>(&sample));
                pixels[i * 4 + 1] = static_cast<unsigned char>(sample &= 0xff);
            }
            for (i = 0; i < count; ++i) {
                PyroPsdReadSample(reinterpret_cast<unsigned short*>(&sample));
                pixels[i * 4 + 2] = static_cast<unsigned char>(sample &= 0xff);
            }
            for (i = 0; i < count; ++i) {
                PyroPsdReadSample(reinterpret_cast<unsigned short*>(&sample));
                pixels[i * 4 + 3] = static_cast<unsigned char>(sample &= 0xff);
            }
        }
        for (i = 0; i < count; ++i) {
            unsigned char* const p = pixels + i * 4;
            const int k = p[3];
            p[0] = static_cast<unsigned char>((p[0] * k) >> 8);
            p[1] = static_cast<unsigned char>((p[1] * k) >> 8);
            p[2] = static_cast<unsigned char>((p[2] * k) >> 8);
            p[3] = 0xff;
        }
        if (header.channels > 4) {
            if (g_pyroPsdSampleSize == 1) {
                for (i = 0; i < count; ++i) {
                    PyroPsdReadSample(reinterpret_cast<unsigned short*>(&sample));
                    pixels[i * 4 + 3] = static_cast<unsigned char>(sample);
                }
            }
            else {
                for (i = 0; i < count; ++i) {
                    PyroPsdReadSample(reinterpret_cast<unsigned short*>(&sample));
                    pixels[i * 4 + 3] = static_cast<unsigned char>(sample &= 0xff);
                }
            }
        }
        return true;
    }
    image->Clear();
    return false;
}

// A 15/16-bit TGA colour (x1555) as 32-bit with an opaque alpha.
static unsigned PyroTga16To32(unsigned value)
{
    return (((value | 0xffffffe0) << 13 | (value & 0x3e0)) << 6) | ((value >> 7) & 0xf8);
}

// EAX=stream, stack image, RET 4. TGA decoder over g_pyroTgaHeader (read by
// PyroIsTga): true colour 16/24/32, 8-bit gray and 8-bit colour-mapped with a
// 15/16, 24 or 32-bit map; RLE when the type has bit 3. The descriptor's
// origin bits pick the row and pixel direction.
// FUNCTION: VC1 0x0044c240
static bool PyroDecodeTga(PyroImageStream* stream, PyroImage* image)
{
    unsigned short rawMap[0x100];
    unsigned palette[0x100];
    const bool bottomUp = !((g_pyroTgaHeader.descriptor >> 5) & 1);
    const bool rightToLeft = (g_pyroTgaHeader.descriptor >> 4) & 1;
    unsigned* pixel;
    int step;
    int rowSkip;
    int x;
    int y;
    unsigned value;
    unsigned i;

    g_pyroTgaPixelSize = g_pyroTgaHeader.depth >> 3;
    g_pyroTgaHeader.descriptor &= 0xcf;
    g_pyroTgaRle = (g_pyroTgaHeader.imageType >> 3) & 1;
    g_pyroTgaHeader.imageType &= 0xf7;
    g_pyroTgaRunRepeat = false;
    g_pyroTgaRunLeft = 0;
    g_pyroTgaStream = stream;
    image->Clear();
    image->width = g_pyroTgaHeader.width;
    image->height = g_pyroTgaHeader.height;
    image->pixels = malloc(image->width * image->height * 4);
    pixel = static_cast<unsigned*>(image->pixels);
    step = rightToLeft ? -1 : 1;
    if (!bottomUp) {
        if (rightToLeft) {
            rowSkip = image->width * 2;
            pixel += image->width - 1;
        }
        else {
            rowSkip = 0;
        }
    }
    else {
        if (rightToLeft) {
            rowSkip = 0;
            pixel += image->width - 1;
        }
        else {
            rowSkip = -image->width * 2;
        }
        pixel += (image->height - 1) * image->width;
    }
    stream->Seek(g_pyroTgaHeader.idLength, 1);

    switch (g_pyroTgaHeader.depth) {
    case 32:
        for (y = 0; y < image->height; ++y) {
            for (x = 0; x < image->width; ++x) {
                unsigned char* const bytes = reinterpret_cast<unsigned char*>(pixel);
                unsigned char swap;
                PyroTgaReadPixel(pixel);
                swap = bytes[0];
                bytes[0] = bytes[2];
                bytes[2] = swap;
                pixel += step;
            }
            pixel += rowSkip;
        }
        return true;
    case 24:
        for (y = 0; y < image->height; ++y) {
            for (x = 0; x < image->width; ++x) {
                unsigned char* const bytes = reinterpret_cast<unsigned char*>(&value);
                unsigned char swap;
                value = 0xff000000;
                PyroTgaReadPixel(&value);
                swap = bytes[0];
                bytes[0] = bytes[2];
                bytes[2] = swap;
                *pixel = value;
                pixel += step;
            }
            pixel += rowSkip;
        }
        return true;
    case 16:
        for (y = 0; y < image->height; ++y) {
            for (x = 0; x < image->width; ++x) {
                PyroTgaReadPixel(&value);
                *pixel = PyroTga16To32(static_cast<unsigned short>(value));
                pixel += step;
            }
            pixel += rowSkip;
        }
        return true;
    case 8:
        if (g_pyroTgaHeader.imageType == 3) {
            for (y = 0; y < image->height; ++y) {
                for (x = 0; x < image->width; ++x) {
                    PyroTgaReadPixel(&value);
                    value &= 0xff;
                    *pixel = (((value | 0xffffff00) << 8 | value) << 8) | value;
                    pixel += step;
                }
                pixel += rowSkip;
            }
            return true;
        }
        switch (g_pyroTgaHeader.colorMapDepth) {
        case 15:
        case 16:
            if (g_pyroTgaStream->Read(rawMap, g_pyroTgaHeader.colorMapLength * 2)
                != g_pyroTgaHeader.colorMapLength * 2) {
                break;
            }
            for (i = 0; static_cast<int>(i) < g_pyroTgaHeader.colorMapLength; ++i) {
                palette[i] = PyroTga16To32(rawMap[i]);
            }
            goto indexed;
        case 24:
            for (i = 0; i < g_pyroTgaHeader.colorMapLength; ++i) {
                value = 0;
                if (g_pyroTgaStream->Read(&value, 3) != 3) {
                    goto failed;
                }
                palette[i] = value | 0xff000000;
            }
            goto indexed;
        case 32:
            g_pyroTgaStream->Read(palette, g_pyroTgaHeader.colorMapLength * 4);
            goto indexed;
        }
        break;
    indexed:
        for (y = 0; y < image->height; ++y) {
            for (x = 0; x < image->width; ++x) {
                PyroTgaReadPixel(&value);
                *pixel = palette[value & 0xff];
                pixel += step;
            }
            pixel += rowSkip;
        }
        return true;
    }
failed:
    image->Clear();
    return false;
}

// RET 8. libpng decode to 32-bit B, G, R, A: palette and low-depth gray are
// expanded, 16-bit samples swapped (the high byte is taken), colour samples
// read as BGR. merge (never set) would only replace the alpha of an image of
// the same size.
// FUNCTION: VC1 0x0044c7b0
static bool PyroDecodePng(PyroImage* image, PyroImageStream* stream)
{
    bool merge;
    unsigned char alpha;
    png_infop info;
    unsigned char* data;
    png_bytep* rows;
    png_structp png;
    unsigned palette[256];
    unsigned char* source;
    unsigned char* target;
    int bitDepth;
    int count;
    int width;
    int x;
    int y;
    int i;

    merge = false;
    alpha = 0xff;
    info = 0;
    data = 0;
    png = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, PngError, PngWarning);
    if (!png) {
        return false;
    }
    if (setjmp(png->jmpbuf)) {
        if (data) {
            free(data);
        }
        png_destroy_read_struct(&png, &info, 0);
        image->Clear();
        return false;
    }
    info = png_create_info_struct(png);
    if (!info) {
        png_destroy_read_struct(&png, &info, 0);
        return false;
    }
    png_set_read_fn(png, stream, PngReadData);
    png_read_info(png, info);
    if ((info->color_type == PNG_COLOR_TYPE_PALETTE && info->bit_depth < 8) ||
        (info->color_type == PNG_COLOR_TYPE_GRAY && info->bit_depth < 8) ||
        (info->valid & PNG_INFO_tRNS)) {
        png_set_expand(png);
    }
    if (info->interlace_type != PNG_INTERLACE_NONE) {
        png_set_interlace_handling(png);
    }
    if (info->bit_depth == 16) {
        png_set_swap(png);
    }
    if (info->color_type & PNG_COLOR_MASK_COLOR) {
        png_set_bgr(png);
    }
    png_read_update_info(png, info);

    rows = static_cast<png_bytep*>(malloc(info->height * sizeof(png_bytep)));
    data = static_cast<unsigned char*>(malloc(info->rowbytes * info->height));
    for (i = 0; i < static_cast<int>(info->height); ++i) {
        rows[i] = data + info->rowbytes * i;
    }
    png_read_image(png, rows);
    png_read_end(png, info);

    if (!merge) {
        image->Clear();
        image->width = info->width;
        image->height = info->height;
        image->pixels = malloc(image->width * image->height * 4);
    }
    else if (image->width != static_cast<int>(info->width) ||
        image->height != static_cast<int>(info->height)) {
        image->Clear();
    }

    source = data;
    bitDepth = info->bit_depth;
    width = image->width;
    count = image->height * width;
    target = static_cast<unsigned char*>(image->pixels);
    switch (info->color_type) {
    case PNG_COLOR_TYPE_GRAY:
        if (!merge) {
            if (bitDepth == 8) {
                while (count--) {
                    target[0] = *source;
                    target[1] = *source;
                    target[2] = *source;
                    target[3] = alpha;
                    target += 4;
                    source++;
                }
            }
            else if (bitDepth == 16) {
                source = data + 1;
                while (count--) {
                    target[0] = *source;
                    target[1] = *source;
                    target[2] = *source;
                    target[3] = alpha;
                    source += 2;
                    target += 4;
                }
            }
        }
        else if (bitDepth == 8) {
            target += 3;
            while (count--) {
                *target = *source;
                source++;
                target += 4;
            }
        }
        else if (bitDepth == 16) {
            target += 3;
            source = data + 1;
            while (count--) {
                *target = *source;
                source += 2;
                target += 4;
            }
        }
        break;
    case PNG_COLOR_TYPE_RGB:
        if (bitDepth == 8) {
            source = data + 2;
            while (count--) {
                target[0] = source[-2];
                target[1] = source[-1];
                target[2] = source[0];
                target[3] = alpha;
                source += 3;
                target += 4;
            }
        }
        else if (bitDepth == 16) {
            source = data + 3;
            while (count--) {
                target[0] = source[-2];
                target[1] = source[0];
                target[2] = source[2];
                target[3] = alpha;
                source += 6;
                target += 4;
            }
        }
        break;
    case PNG_COLOR_TYPE_PALETTE:
        {
            const png_byte* entry = reinterpret_cast<const png_byte*>(png->palette);
            unsigned char* slot = reinterpret_cast<unsigned char*>(palette);

            for (i = png->num_palette; i != 0; --i) {
                slot[0] = entry[0];
                slot[1] = entry[1];
                slot[2] = entry[2];
                slot[3] = alpha;
                entry += 3;
                slot += 4;
            }
        }
        if (bitDepth == 2) {
            unsigned* out = reinterpret_cast<unsigned*>(target);

            for (y = 0; y < image->height; ++y) {
                for (x = 0; x < image->width; x += 4) {
                    const unsigned char bits = *source++;
                    *out++ = palette[bits >> 6];
                    if (x + 1 >= image->width) {
                        break;
                    }
                    *out++ = palette[(bits >> 4) & 3];
                    if (x + 2 >= image->width) {
                        break;
                    }
                    *out++ = palette[(bits >> 2) & 3];
                    if (x + 3 >= image->width) {
                        break;
                    }
                    *out++ = palette[bits & 3];
                }
            }
            count = 0;
        }
        else if (bitDepth == 4) {
            unsigned* out = reinterpret_cast<unsigned*>(target);

            for (y = 0; y < image->height; ++y) {
                for (x = 0; x < image->width; x += 2) {
                    const unsigned char bits = *source++;
                    *out++ = palette[bits >> 4];
                    if (x + 1 >= image->width) {
                        break;
                    }
                    *out++ = palette[bits & 0xf];
                }
            }
            count = 0;
        }
        else if (bitDepth == 8) {
            unsigned* out = reinterpret_cast<unsigned*>(target);

            while (count--) {
                *out++ = palette[*source];
                source++;
            }
        }
        break;
    case PNG_COLOR_TYPE_GRAY_ALPHA:
        if (bitDepth == 8) {
            while (count--) {
                target[0] = source[0];
                target[1] = source[0];
                target[2] = source[0];
                target[3] = source[1];
                source += 2;
                target += 4;
            }
        }
        else if (bitDepth == 16) {
            source = data + 1;
            while (count--) {
                target[0] = source[0];
                target[1] = source[0];
                target[2] = source[0];
                target[3] = source[2];
                source += 4;
                target += 4;
            }
        }
        break;
    case PNG_COLOR_TYPE_RGB_ALPHA:
        if (bitDepth == 8) {
            source = data + 2;
            while (count--) {
                target[0] = source[-2];
                target[1] = source[-1];
                target[2] = source[0];
                target[3] = source[1];
                source += 4;
                target += 4;
            }
        }
        else if (bitDepth == 16) {
            source = data + 3;
            while (count--) {
                target[0] = source[-2];
                target[1] = source[0];
                target[2] = source[2];
                target[3] = source[4];
                source += 8;
                target += 4;
            }
        }
        break;
    }
    free(data);
    free(rows);
    png_destroy_read_struct(&png, &info, 0);
    if (count > 0) {
        image->Clear();
        return false;
    }
    return true;
}

// ESI=stream, EDI=image. JPEG and JPEG 2000 are detected but not decoded.
// The original keeps it out of line at both texture creators.
// FUNCTION: VC1 0x0044ce50
__declspec(noinline) bool PyroDecodeImage(PyroImageStream* stream, PyroImage* image)
{
    switch (PyroDetectImageFormat(stream)) {
    case 3:
        return PyroDecodeTga(stream, image);
    case 4:
        return PyroDecodeDds(image, stream);
    case 5:
        return PyroDecodePsd(stream, image);
    case 6:
        return PyroDecodePng(image, stream);
    }
    return false;
}

// RET 8. SEEK_END ignores the offset.
// FUNCTION: VC1 0x0044daa0
int PyroMemoryStream::Seek(int offset, int origin)
{
    if (origin == 0) {
        position = offset;
        return offset;
    }
    if (origin == 1) {
        position += offset;
        return position;
    }
    if (origin == 2) {
        position = size;
    }
    return position;
}

// RET 8.
// FUNCTION: VC1 0x0044dce0
int PyroMemoryStream::Read(void* buffer, int count)
{
    const int available = size - position;

    if (count > available) {
        count = available;
    }
    if (count > 0) {
        memcpy(buffer, data + position, count);
        position += count;
    }
    else if (count < 0) {
        return 0;
    }
    return count;
}

// RET 8. Grows the buffer from 0x80 bytes by doubling.
// FUNCTION: VC1 0x0044df80
int PyroMemoryStream::Write(const void* buffer, int count)
{
    const int end = position + count;

    if (capacity <= end) {
        if (!capacity) {
            capacity = 0x80;
        }
        while (end > capacity) {
            capacity += capacity;
        }
        data = static_cast<unsigned char*>(realloc(data, capacity));
    }
    if (size < end) {
        size = end;
    }
    memcpy(data + position, buffer, count);
    position += count;
    return count;
}

}
