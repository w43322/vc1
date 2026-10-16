#ifndef VC1_ZLIB113_H
#define VC1_ZLIB113_H

// reccmp name comments for the vendored zlib 1.1.3 inflate side, which the
// game links for libpng (third_party/libpng102, cmake/libpng.cmake). The
// addresses were paired with our build by call order from the mapped libpng
// callers.

// FUNCTION: VC1 0x00444830
// inflate_fast

// FUNCTION: VC1 0x00444b80
// inflate_codes_new

// FUNCTION: VC1 0x00444bc0
// inflate_codes

// FUNCTION: VC1 0x00445250
// huft_build

// FUNCTION: VC1 0x004456d0
// inflate_trees_dynamic

// FUNCTION: VC1 0x00445850
// inflate_trees_bits

// FUNCTION: VC1 0x00445af0
// inflate_blocks_reset

// FUNCTION: VC1 0x00445b60
// inflate_blocks_free

// FUNCTION: VC1 0x00445bf0
// inflate_blocks_new

// FUNCTION: VC1 0x00445c90
// inflate_blocks

// FUNCTION: VC1 0x004466f0
// adler32

// FUNCTION: VC1 0x00446830
// crc32

// FUNCTION: VC1 0x00447e10
// inflateReset

// FUNCTION: VC1 0x00447e50
// inflateEnd

// FUNCTION: VC1 0x00447ea0
// inflate

// FUNCTION: VC1 0x00448240
// inflateInit2_

// FUNCTION: VC1 0x004b3f40
// inflate_flush

#endif
