/* VC9 has no <inttypes.h>; declarations the vendored klibc scanf files
 * expect from klibc's own headers. */
#ifndef VC1_KLIBC_INTTYPES_H
#define VC1_KLIBC_INTTYPES_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

uintmax_t strntoumax(const char *nptr, char **endptr, int base, size_t n);
int vsscanf(const char *buffer, const char *format, va_list ap);

#endif
