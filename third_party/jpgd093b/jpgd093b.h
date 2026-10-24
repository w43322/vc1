#ifndef VC1_JPGD093B_H
#define VC1_JPGD093B_H

// reccmp name comments for the vendored jpgd 0.93b (built without
// SUPPORT_X86ASM / SUPPORT_MMX). The original keeps the library in source
// order from 0x00457130 (H2V1Convert) to 0x00459da0 (begin); LTCG moved a few
// bodies elsewhere. Inlined in the original, so not mapped: ~jpeg_decoder
// (into free_all_blocks callers), read_sof_marker, read_dri_marker,
// skip_variable_marker, next_marker, locate_sos_marker, init_sequential,
// decode_init, coeff_buf_open, coeff_buf_getp, stuff_char, rol.

// FUNCTION: VC1 0x0041aa30 FOLDED
// jpeg_decoder_stream::attach

// FUNCTION: VC1 0x0041aa30 FOLDED
// jpeg_decoder_stream::detach

// FUNCTION: VC1 0x0041aa60
// jpeg_decoder_file_stream::~jpeg_decoder_file_stream

// FUNCTION: VC1 0x0041aab0
// jpeg_decoder_file_stream::read

// SYNTHETIC: VC1 0x0041ab40
// jpeg_decoder_file_stream::`scalar deleting destructor'

// FUNCTION: VC1 0x0042e3c0
// ?get_char@jpeg_decoder@@AAEIXZ

// EDI = Ppadding_flag.
// FUNCTION: VC1 0x0042e420
// ?get_char@jpeg_decoder@@AAEIPA_N@Z

// Out of line for its huff_decode/get_bits callers; stuff_char inlined.
// FUNCTION: VC1 0x0042e480
// ?get_octet@jpeg_decoder@@AAEEXZ

// FUNCTION: VC1 0x0042e500
// jpeg_decoder::get_bits_1

// FUNCTION: VC1 0x0042e590
// jpeg_decoder::get_bits_2

// FUNCTION: VC1 0x0042e630
// jpeg_decoder::huff_decode

// FUNCTION: VC1 0x0044e450
// idct

// FUNCTION: VC1 0x00456900
// jpeg_decoder::calc_mcu_block_order

// FUNCTION: VC1 0x00456a50
// jpeg_decoder::GrayConvert

// FUNCTION: VC1 0x00456a90
// jpeg_decoder::create_look_ups

// FUNCTION: VC1 0x00456b00
// jpeg_decoder::make_huff_table

// FUNCTION: VC1 0x00456ce0
// jpeg_decoder::H1V2Convert

// FUNCTION: VC1 0x00457130
// jpeg_decoder::H2V1Convert

// FUNCTION: VC1 0x00457500
// jpeg_decoder::H1V1Convert

// FUNCTION: VC1 0x00457750
// jpeg_decoder::transform_row

// FUNCTION: VC1 0x004577a0
// jpeg_decoder::free_all_blocks

// FUNCTION: VC1 0x004577f0
// jpeg_decoder::terminate

// FUNCTION: VC1 0x00457810
// jpeg_decoder::check_quant_tables

// FUNCTION: VC1 0x004578a0
// jpeg_decoder::prep_in_buffer

// FUNCTION: VC1 0x00457930
// jpeg_decoder::alloc

// FUNCTION: VC1 0x004579d0
// jpeg_decoder::init_frame

// FUNCTION: VC1 0x00457c70
// jpeg_decoder::check_huff_tables

// FUNCTION: VC1 0x00457d80
// jpeg_decoder::load_next_row

// FUNCTION: VC1 0x00458060
// jpeg_decoder::init

// FUNCTION: VC1 0x00458490
// jpeg_decoder::locate_soi_marker

// FUNCTION: VC1 0x00458540
// jpeg_decoder::read_sos_marker

// FUNCTION: VC1 0x00458840
// jpeg_decoder::read_dqt_marker

// FUNCTION: VC1 0x00458960
// jpeg_decoder::read_dht_marker

// FUNCTION: VC1 0x00458ad0
// progressive_block_decoder::decode_block_dc_refine

// FUNCTION: VC1 0x00458b40
// jpeg_decoder::process_restart

// FUNCTION: VC1 0x00458cb0
// jpeg_decoder::fix_in_buffer

// FUNCTION: VC1 0x00458d60
// jpeg_decoder::process_markers

// FUNCTION: VC1 0x00458f60
// jpeg_decoder::decode_scan

// FUNCTION: VC1 0x004590f0
// progressive_block_decoder::decode_block_ac_refine

// FUNCTION: VC1 0x00459310
// progressive_block_decoder::decode_block_ac_first

// FUNCTION: VC1 0x00459450
// progressive_block_decoder::decode_block_dc_first

// FUNCTION: VC1 0x00459500
// jpeg_decoder::find_eoi

// FUNCTION: VC1 0x00459540
// jpeg_decoder::decode_next_row

// FUNCTION: VC1 0x00459870
// jpeg_decoder::locate_sof_marker

// FUNCTION: VC1 0x004598f0
// jpeg_decoder::init_scan

// FUNCTION: VC1 0x004599a0
// jpeg_decoder::decode

// FUNCTION: VC1 0x00459b00
// jpeg_decoder::jpeg_decoder

// FUNCTION: VC1 0x00459b70
// jpeg_decoder::init_progressive

// FUNCTION: VC1 0x00459d70
// jpeg_decoder::decode_start

// FUNCTION: VC1 0x00459da0
// jpeg_decoder::begin

// FUNCTION: VC1 0x004b41e0
// jpeg_decoder::H2V2Convert

#endif
