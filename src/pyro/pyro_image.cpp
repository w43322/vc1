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

// EAX=image, stack stream. DDS decoder; not recovered (no .pyro image uses it).
// STUB: VC1 0x0044a310
static bool PyroDecodeDds(PyroImage* image, PyroImageStream* stream)
{
    (void)image;
    (void)stream;
    return false;
}

// EAX=stream, stack image. PSD decoder; not recovered (no .pyro image uses it).
// STUB: VC1 0x0044b850
static bool PyroDecodePsd(PyroImageStream* stream, PyroImage* image)
{
    (void)stream;
    (void)image;
    return false;
}

// EAX=stream, stack image. TGA decoder over g_pyroTgaHeader; not recovered
// (no .pyro image uses it).
// STUB: VC1 0x0044c240
static bool PyroDecodeTga(PyroImageStream* stream, PyroImage* image)
{
    (void)stream;
    (void)image;
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
