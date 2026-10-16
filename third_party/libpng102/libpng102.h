#ifndef VC1_LIBPNG102_H
#define VC1_LIBPNG102_H

// reccmp name comments for the vendored libpng 1.0.2 (read side). The game
// reads PNG images embedded in .pyro files (src/pyro/pyro_image.cpp). LTCG
// propagated the game's constant png_create_read_struct arguments and inlined
// png_create_info_struct, png_set_expand, png_set_interlace_handling,
// png_set_swap, png_set_bgr and png_read_update_info into the caller.

// FUNCTION: VC1 0x004483f0
// png_error

// FUNCTION: VC1 0x00446c40
// png_read_start_row

// FUNCTION: VC1 0x004497e0
// png_read_transform_info

// FUNCTION: VC1 0x004b4180
// png_set_read_fn

// FUNCTION: VC1 0x004b9370
// png_destroy_read_struct

// FUNCTION: VC1 0x004b93f0
// png_create_read_struct

// FUNCTION: VC1 0x004b9570
// png_read_end

// FUNCTION: VC1 0x004b9be0
// png_read_info

// FUNCTION: VC1 0x004b9fa0
// png_read_image

#endif
