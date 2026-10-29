#ifndef VC1_MOD_MOD_IO_H
#define VC1_MOD_MOD_IO_H

// The VC1_MODS addon's entry points. Not original code.
namespace vc1 {

struct IDirect3DTexture8;

// Reads mods/ beside the executable (mod_io.cpp).
void InitModHost();

// Replaces a freshly loaded D3D8 texture's pixels with the enabled mods'
// tiles; called from LoadTextureFile (texture_hook.cpp).
void ModTexture(const char* name, IDirect3DTexture8* texture);

}

#endif
