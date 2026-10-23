// VC1_MODS texture hook: the one call LoadTextureFile makes into the addon.
// Works on any D3D8 (the real one or imports/sdl3/d3d8.cpp): level 0 is
// locked, copied to packed rows for the mod core and copied back if changed.
// Not original code.
#include "mod/mod_io.h"
#include "mod/mods.h"
#include "kdcore/native.h"

#include <stdlib.h>
#include <string.h>

namespace vc1 {

namespace {

const unsigned kA8R8G8B8 = 0x15;
const unsigned kX8R8G8B8 = 0x16;

// D3DLOCKED_RECT.
struct LockedRect
{
    int pitch;
    unsigned char* bits;
};

}

void ModTexture(const char* name, IUnknown* texture)
{
    typedef int (__stdcall *FnLevelDesc)(IUnknown* texture, unsigned level, unsigned* desc);
    typedef int (__stdcall *FnLockRect)(IUnknown* texture, unsigned level, LockedRect* locked,
        const RECT* rect, unsigned flags);
    typedef int (__stdcall *FnUnlockRect)(IUnknown* texture, unsigned level);
    static bool ready = false;
    void** const vtbl = *reinterpret_cast<void***>(texture);
    unsigned desc[8];
    LockedRect locked;

    if (!ready) {
        ready = true;
        InitModHost();
    }
    if (reinterpret_cast<FnLevelDesc>(vtbl[0xe])(texture, 0, desc) < 0 ||
        (desc[0] != kA8R8G8B8 && desc[0] != kX8R8G8B8)) {
        return;
    }
    const unsigned width = desc[6];
    const unsigned height = desc[7];
    unsigned* pixels = static_cast<unsigned*>(malloc(width * height * 4));
    if (!pixels || reinterpret_cast<FnLockRect>(vtbl[0x10])(texture, 0, &locked, 0, 0) < 0) {
        free(pixels);
        return;
    }
    for (unsigned y = 0; y < height; ++y) {
        memcpy(pixels + y * width, locked.bits + y * locked.pitch, width * 4);
    }
    if (ApplyTextureMods(name, width, height, pixels)) {
        for (unsigned y = 0; y < height; ++y) {
            memcpy(locked.bits + y * locked.pitch, pixels + y * width, width * 4);
        }
    }
    reinterpret_cast<FnUnlockRect>(vtbl[0x11])(texture, 0);
    free(pixels);
}

}
