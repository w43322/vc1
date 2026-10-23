// Host services for the texture mod core (mods.cpp): files through the CRT,
// PNG through the game's libpng 1.0.2, tile names through the live Squirrel
// Tiles / TileImages tables, messages through HostLog. Not original code.
#include "mod/mod_io.h"
#include "mod/mods.h"
#include "kdcore/native.h"
#include "kdcore/platform.h"

#include <squirrel.h>
#include "sqplus/SquirrelVM.h"

#include <png.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace vc1 {

namespace {

const char* kModsName = "Mods";

bool ReadFile(const char* name, unsigned char** bytes, unsigned* length)
{
    FILE* file = fopen(name, "rb");
    if (!file) {
        return false;
    }
    fseek(file, 0, SEEK_END);
    const long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    bool ok = size >= 0 && size <= 16 * 1024 * 1024;
    if (ok) {
        *bytes = static_cast<unsigned char*>(malloc(size ? size : 1));
        ok = *bytes && fread(*bytes, 1, size, file) == static_cast<size_t>(size);
        if (!ok) {
            free(*bytes);
        }
        *length = static_cast<unsigned>(size);
    }
    fclose(file);
    return ok;
}

void PngError(png_structp png, png_const_charp message)
{
    (void)message;
    longjmp(png->jmpbuf, 1);
}

void PngWarning(png_structp png, png_const_charp message)
{
    (void)png;
    (void)message;
}

void PngRead(png_structp png, png_bytep data, png_size_t length)
{
    if (fread(data, 1, length, static_cast<FILE*>(png_get_io_ptr(png))) != length) {
        png_error(png, "read");
    }
}

// Any PNG to ARGB. This libpng has no 16-to-8 transform, so 16-bit rows keep
// each sample's high byte by hand.
bool ReadPng(const char* name, ModImage* image)
{
    FILE* file = fopen(name, "rb");
    if (!file) {
        return false;
    }
    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, PngError, PngWarning);
    png_infop info = png ? png_create_info_struct(png) : 0;
    unsigned char* rows = 0;
    png_bytep* pointers = 0;
    bool ok = false;
    if (png && info && !setjmp(png->jmpbuf)) {
        png_set_read_fn(png, file, PngRead);
        png_read_info(png, info);
        png_uint_32 width = 0;
        png_uint_32 height = 0;
        int depth = 0;
        int color = 0;
        png_get_IHDR(png, info, &width, &height, &depth, &color, 0, 0, 0);
        if (width && height && width <= 8192 && height <= 8192) {
            png_set_expand(png);
            if (!(color & PNG_COLOR_MASK_COLOR)) {
                png_set_gray_to_rgb(png);
            }
            png_set_bgr(png);
            if (!(color & PNG_COLOR_MASK_ALPHA) && !png_get_valid(png, info, PNG_INFO_tRNS)) {
                png_set_filler(png, depth == 16 ? 0xffff : 0xff, PNG_FILLER_AFTER);
            }
            png_set_interlace_handling(png);
            png_read_update_info(png, info);
            const png_uint_32 stride = png_get_rowbytes(png, info);
            const unsigned bytesPerSample = depth == 16 ? 2 : 1;
            rows = static_cast<unsigned char*>(malloc(stride * height));
            pointers = static_cast<png_bytep*>(malloc(sizeof(png_bytep) * height));
            image->pixels = static_cast<unsigned*>(malloc(width * height * 4));
            if (rows && pointers && image->pixels && stride == width * 4 * bytesPerSample) {
                for (png_uint_32 y = 0; y < height; ++y) {
                    pointers[y] = rows + y * stride;
                }
                png_read_image(png, pointers);
                png_read_end(png, 0);
                unsigned char* out = reinterpret_cast<unsigned char*>(image->pixels);
                for (png_uint_32 i = 0; i < width * height * 4; ++i) {
                    out[i] = rows[i * bytesPerSample];
                }
                image->width = width;
                image->height = height;
                ok = true;
            }
        }
    }
    if (!ok && image->pixels) {
        free(image->pixels);
        image->pixels = 0;
    }
    if (png) {
        png_destroy_read_struct(&png, info ? &info : 0, 0);
    }
    free(pointers);
    free(rows);
    fclose(file);
    return ok;
}

bool TileId(const char* name, int* tile)
{
    HSQUIRRELVM vm = SquirrelVM::_VM;
    if (!vm) {
        return false;
    }
    const SQInteger top = sq_gettop(vm);
    SQInteger id = 0;
    bool ok = false;
    sq_pushroottable(vm);
    sq_pushstring(vm, "Tiles", -1);
    if (SQ_SUCCEEDED(sq_rawget(vm, -2))) {
        sq_pushstring(vm, name, -1);
        ok = SQ_SUCCEEDED(sq_rawget(vm, -2)) && SQ_SUCCEEDED(sq_getinteger(vm, -1, &id));
        if (ok) {
            *tile = id;
        }
    }
    sq_settop(vm, top);
    return ok;
}

bool AtlasName(int index, char* name, unsigned capacity)
{
    HSQUIRRELVM vm = SquirrelVM::_VM;
    if (!vm) {
        return false;
    }
    const SQInteger top = sq_gettop(vm);
    const SQChar* value = 0;
    bool ok = false;
    sq_pushroottable(vm);
    sq_pushstring(vm, "TileImages", -1);
    if (SQ_SUCCEEDED(sq_rawget(vm, -2))) {
        sq_pushinteger(vm, index);
        if (SQ_SUCCEEDED(sq_get(vm, -2)) && SQ_SUCCEEDED(sq_getstring(vm, -1, &value)) &&
            strlen(value) + 5 < capacity) {
            FormatBuffer(name, capacity, "res/%s", value);
            ok = true;
        }
    }
    sq_settop(vm, top);
    return ok;
}

void Log(const char* message)
{
    HostLog(&kModsName, "%s", message);
}

}

// The mods/ folder beside the executable, or VC1_MODS.
void InitModHost()
{
    char root[MAX_PATH];
    const char* overrideRoot = getenv("VC1_MODS");
    if (overrideRoot) {
        FormatBuffer(root, sizeof(root), "%s", overrideRoot);
    }
    else {
        const DWORD length = GetModuleFileNameA(0, root, sizeof(root));
        char* slash = length ? strrchr(root, '\\') : 0;
        if (!slash) {
            slash = length ? strrchr(root, '/') : 0;
        }
        if (!slash) {
            return;
        }
        FormatBuffer(slash + 1, sizeof(root) - (slash + 1 - root), "mods");
    }
    ModHost host = {ReadFile, ReadPng, TileId, AtlasName, Log};
    InitMods(root, host);
    atexit(ShutdownMods);
}

}
