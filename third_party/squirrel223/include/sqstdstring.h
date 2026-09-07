/*	see copyright notice in squirrel.h */
#ifndef _SQSTD_STRING_H_
#define _SQSTD_STRING_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int SQRexBool;
typedef struct SQRex SQRex;

typedef struct {
	const SQChar *begin;
	SQInteger len;
} SQRexMatch;

SQUIRREL_API SQRex *sqstd_rex_compile(const SQChar *pattern,const SQChar **error);
SQUIRREL_API void sqstd_rex_free(SQRex *exp);
SQUIRREL_API SQBool sqstd_rex_match(SQRex* exp,const SQChar* text);
SQUIRREL_API SQBool sqstd_rex_search(SQRex* exp,const SQChar* text, const SQChar** out_begin, const SQChar** out_end);
SQUIRREL_API SQBool sqstd_rex_searchrange(SQRex* exp,const SQChar* text_begin,const SQChar* text_end,const SQChar** out_begin, const SQChar** out_end);
SQUIRREL_API SQInteger sqstd_rex_getsubexpcount(SQRex* exp);
SQUIRREL_API SQBool sqstd_rex_getsubexp(SQRex* exp, SQInteger n, SQRexMatch *subexp);

SQUIRREL_API SQRESULT sqstd_format(HSQUIRRELVM v,SQInteger nformatstringidx,SQInteger *outlen,SQChar **output);

SQUIRREL_API SQRESULT sqstd_register_stringlib(HSQUIRRELVM v);

#ifdef __cplusplus
} /*extern "C"*/
#endif

// FUNCTION: VC1 0x004822c0
// sqstd_rex_matchcclass
// FUNCTION: VC1 0x00482490
// sqstd_rex_parsenumber
// FUNCTION: VC1 0x00482500
// sqstd_rex_escapechar
// FUNCTION: VC1 0x004825b0
// sqstd_rex_expect
// FUNCTION: VC1 0x004825d0
// sqstd_rex_free
// FUNCTION: VC1 0x00482680
// sqstd_rex_matchclass
// FUNCTION: VC1 0x004826e0
// sqstd_rex_matchnode
// FUNCTION: VC1 0x00482b10
// sqstd_rex_searchrange
// FUNCTION: VC1 0x00482c20
// sqstd_rex_newnode
// FUNCTION: VC1 0x00482ca0
// sqstd_rex_charclass
// FUNCTION: VC1 0x00482d20
// sqstd_rex_charnode
// FUNCTION: VC1 0x00482e80
// sqstd_rex_class
// FUNCTION: VC1 0x004830c0
// sqstd_rex_element
// FUNCTION: VC1 0x00483420
// sqstd_rex_list
// FUNCTION: VC1 0x00483540
// sqstd_rex_compile
// FUNCTION: VC1 0x0048d480
// _rexobj_releasehook
// FUNCTION: VC1 0x0048d4a0
// _regexp__typeof
// FUNCTION: VC1 0x0048d4c0
// _regexp_subexpcount
// FUNCTION: VC1 0x0048d540
// validate_format
// FUNCTION: VC1 0x0048d6c0
// _addrexmatch
// FUNCTION: VC1 0x0048d780
// _regexp_match
// FUNCTION: VC1 0x0048d8b0
// _string_split
// FUNCTION: VC1 0x0048dab0
// _string_rstrip
// FUNCTION: VC1 0x0048dbb0
// _string_lstrip
// FUNCTION: VC1 0x0048dc50
// _string_strip
// FUNCTION: VC1 0x0048dd80
// sqstd_format
// FUNCTION: VC1 0x0048e0f0
// _regexp_capture
// FUNCTION: VC1 0x0048e290
// _regexp_search
// FUNCTION: VC1 0x0048e3c0
// _string_format
// FUNCTION: VC1 0x0048e420
// sqstd_register_stringlib
// FUNCTION: VC1 0x0048e600
// _regexp_constructor

#endif /*_SQSTD_STRING_H_*/
