/* VC1: replaces the configure-generated tgaconfig.h. The game's build left
 * SIZEOF_UNSIGNED_INT undefined, so tga.h takes the fallback typedefs
 * (tuint16 is unsigned int): TGA is 0x34 bytes with int-sized header fields
 * (width at +0x24, height at +0x28, depth at +0x2c, error proc at +0x30). */
#define STDC_HEADERS 1
#define VERSION "1.0.1"
