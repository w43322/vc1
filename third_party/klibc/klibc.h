#ifndef VC1_KLIBC_H
#define VC1_KLIBC_H

/* klibc's scanf family (usr/klibc/{sscanf,vsscanf,strntoumax}.c), linked in
 * place of the CRT sscanf. The original keeps skipspace out of line and
 * inlines digitval / set_bit / test_bit. sscanf takes str in ECX, vsscanf
 * format in EAX, strntoumax nptr in ECX and n in EAX (LTCG). */

// FUNCTION: VC1 0x0045a010
// skipspace

// FUNCTION: VC1 0x0045a040
// strntoumax

// FUNCTION: VC1 0x0045a180
// vsscanf

// FUNCTION: VC1 0x0045a800
// sscanf

#endif
