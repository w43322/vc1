// VC1_MODS texture hook: the one call LoadTextureFile makes into the addon.
// Works on any D3D8 (the real one or imports/sdl3/d3d8.cpp): level 0 is
// locked, copied to packed rows for the mod core and copied back if changed.
// Not original code.
#include "mod/mod_io.h"
#include "mod/mods.h"
#include "kd/display/d3d8_api.h"

#include <stdlib.h>
#include <string.h>

namespace vc1 {

namespace {

const unsigned kA8R8G8B8 = 0x15;
const unsigned kX8R8G8B8 = 0x16;

}

void ModTexture(const char* name, IDirect3DTexture8* texture)
{
    static bool ready = false;
    D3dSurfaceDesc desc;
    D3dLockedRect locked;

    if (!ready) {
        ready = true;
        InitModHost();
    }
    if (texture->GetLevelDesc(0, &desc) < 0 ||
        (desc.format != kA8R8G8B8 && desc.format != kX8R8G8B8)) {
        return;
    }
    const unsigned width = desc.width;
    const unsigned height = desc.height;
    unsigned* pixels = static_cast<unsigned*>(malloc(width * height * 4));
    if (!pixels || texture->LockRect(0, &locked, 0, 0) < 0) {
        free(pixels);
        return;
    }
    unsigned char* const bits = static_cast<unsigned char*>(locked.bits);
    for (unsigned y = 0; y < height; ++y) {
        memcpy(pixels + y * width, bits + y * locked.pitch, width * 4);
    }
    if (ApplyTextureMods(name, width, height, pixels)) {
        for (unsigned y = 0; y < height; ++y) {
            memcpy(bits + y * locked.pitch, pixels + y * width, width * 4);
        }
    }
    texture->UnlockRect(0);
    free(pixels);
}

}
