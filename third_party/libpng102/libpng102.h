#ifndef VC1_LIBPNG102_H
#define VC1_LIBPNG102_H

// reccmp name comments for the vendored libpng 1.0.2 (read side). The game
// reads PNG images embedded in .pyro files (src/pyro/pyro_image.cpp). LTCG
// propagated the game's constant png_create_read_struct arguments and inlined
// png_create_info_struct, png_set_expand, png_set_interlace_handling,
// png_set_swap, png_set_bgr and png_read_update_info into the caller (the
// out-of-line copies of the last three setters are still emitted).

// FUNCTION: VC1 0x00445910
// png_set_tRNS

// FUNCTION: VC1 0x00445950
// png_set_IHDR

// FUNCTION: VC1 0x00445a10
// png_set_text

// FUNCTION: VC1 0x004469a0
// png_read_filter_row

// FUNCTION: VC1 0x00446b40
// png_crc_error

// FUNCTION: VC1 0x00446bd0
// png_crc_read

// FUNCTION: VC1 0x00446c40
// png_read_start_row

// FUNCTION: VC1 0x00446e90
// png_check_chunk_name

// FUNCTION: VC1 0x00446ef0
// png_crc_finish

// FUNCTION: VC1 0x00446fc0
// png_read_finish_row

// FUNCTION: VC1 0x004472e0
// png_handle_unknown

// FUNCTION: VC1 0x00447320
// png_handle_zTXt

// FUNCTION: VC1 0x004476a0
// png_handle_tRNS

// FUNCTION: VC1 0x004478c0
// png_handle_gAMA

// FUNCTION: VC1 0x00447a00
// png_handle_IEND

// FUNCTION: VC1 0x00447a60
// png_handle_PLTE

// FUNCTION: VC1 0x00447be0
// png_handle_IHDR

// FUNCTION: VC1 0x00448320
// png_default_error

// FUNCTION: VC1 0x00448340
// png_format_buffer

// FUNCTION: VC1 0x004483d0
// png_warning

// FUNCTION: VC1 0x004483f0
// png_error

// FUNCTION: VC1 0x00448450
// png_chunk_error

// FUNCTION: VC1 0x00448530
// png_sig_cmp

// FUNCTION: VC1 0x00448570
// png_info_destroy

// FUNCTION: VC1 0x004485d0
// png_zalloc

// FUNCTION: VC1 0x00448650
// png_do_bgr

// FUNCTION: VC1 0x00448720
// png_do_strip_filler

// FUNCTION: VC1 0x00448850
// png_do_swap

// FUNCTION: VC1 0x00448880
// png_set_interlace_handling

// FUNCTION: VC1 0x004488a0
// png_set_swap

// FUNCTION: VC1 0x004488c0
// png_set_bgr

// FUNCTION: VC1 0x004488d0
// png_do_expand

// FUNCTION: VC1 0x00448c80
// png_do_expand_palette

// FUNCTION: VC1 0x00448ea0
// png_do_gamma

// FUNCTION: VC1 0x004492b0
// png_do_gray_to_rgb

// FUNCTION: VC1 0x00449400
// png_do_read_filler

// FUNCTION: VC1 0x004496b0
// png_do_read_swap_alpha

// FUNCTION: VC1 0x004497e0
// png_read_transform_info

// FUNCTION: VC1 0x004498c0
// png_do_read_transformations

// FUNCTION: VC1 0x00449a90
// png_build_gamma_table

// FUNCTION: VC1 0x00449e90
// png_init_read_transformations

// FUNCTION: VC1 0x004b40f0
// png_malloc

// FUNCTION: VC1 0x004b4180
// png_set_read_fn

// FUNCTION: VC1 0x004b91f0
// png_read_destroy

// FUNCTION: VC1 0x004b9370
// png_destroy_read_struct

// FUNCTION: VC1 0x004b93f0
// png_create_read_struct

// FUNCTION: VC1 0x004b9570
// png_read_end

// FUNCTION: VC1 0x004b98b0
// png_read_row

// FUNCTION: VC1 0x004b9be0
// png_read_info

// FUNCTION: VC1 0x004b9fa0
// png_read_image

#endif
