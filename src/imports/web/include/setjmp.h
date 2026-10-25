#ifndef VC1_IMPORTS_WEB_SETJMP_H
#define VC1_IMPORTS_WEB_SETJMP_H

// VC9's x86 jmp_buf is 16 ints, and libpng keeps one first in png_struct
// (0x168 bytes, checked in pyro/pyro_image.cpp). Emscripten's setjmp keeps
// two words in it, so the VC9 size serves; clang lowers setjmp / longjmp by
// name whatever the buffer type.
#ifdef __cplusplus
extern "C" {
#endif

typedef struct vc1_jmp_buf
{
    int vc1_words[16];
} jmp_buf[1];

int setjmp(jmp_buf env) __attribute__((returns_twice));
void longjmp(jmp_buf env, int value) __attribute__((noreturn));

#ifdef __cplusplus
}
#endif

#endif
