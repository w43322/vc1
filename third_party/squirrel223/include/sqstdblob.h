/*	see copyright notice in squirrel.h */
#ifndef _SQSTDBLOB_H_
#define _SQSTDBLOB_H_

#ifdef __cplusplus
extern "C" {
#endif

SQUIRREL_API SQUserPointer sqstd_createblob(HSQUIRRELVM v, SQInteger size);
SQUIRREL_API SQRESULT sqstd_getblob(HSQUIRRELVM v,SQInteger idx,SQUserPointer *ptr);
SQUIRREL_API SQInteger sqstd_getblobsize(HSQUIRRELVM v,SQInteger idx);

SQUIRREL_API SQRESULT sqstd_register_bloblib(HSQUIRRELVM v);

#ifdef __cplusplus
} /*extern "C"*/
#endif

// FUNCTION: VC1 0x0040ef00
// SQBlob::SQBlob
// FUNCTION: VC1 0x0040ef70
// SQBlob::Write
// FUNCTION: VC1 0x0040efd0
// SQBlob::Read
// FUNCTION: VC1 0x0040f020
// SQBlob::Resize
// FUNCTION: VC1 0x0040f0b0
// SQBlob::Seek
// FUNCTION: VC1 0x0040f110
// SQBlob::IsValid
// FUNCTION: VC1 0x0040f120
// SQBlob::EOS
// SYNTHETIC: VC1 0x0040f140
// SQBlob::`scalar deleting destructor'
// FUNCTION: VC1 0x0042f140 FOLDED
// SQBlob::Tell
// FUNCTION: VC1 0x0048e780
// _g_blob_swapfloat
// FUNCTION: VC1 0x0048e7f0
// _g_blob_swap4
// FUNCTION: VC1 0x0048e890
// _g_blob_swap2
// FUNCTION: VC1 0x0048e910
// _g_blob_castf2i
// FUNCTION: VC1 0x0048e980
// _g_blob_casti2f
// FUNCTION: VC1 0x0048e9d0
// sqstd_getblobsize
// FUNCTION: VC1 0x0048ea10
// _blob_constructor
// FUNCTION: VC1 0x0048eb20
// _blob__typeof
// FUNCTION: VC1 0x0048eb40
// _blob__nexti
// FUNCTION: VC1 0x0048ec60
// _blob__get
// FUNCTION: VC1 0x0048ed30
// _blob__set
// FUNCTION: VC1 0x0048ee40
// _blob_swap2
// FUNCTION: VC1 0x0048ef00
// _blob_swap4
// FUNCTION: VC1 0x0048efc0
// _blob_resize
// FUNCTION: VC1 0x0048f080
// sqstd_createblob

#endif /*_SQSTDBLOB_H_*/

