#ifndef VC1_LIBTGA_H
#define VC1_LIBTGA_H

/* libtga 1.0.1 (tga.c, tgaread.c), used by the TGA texture loader
 * (0x00465770). Built as C++ on KD files (src/host/kd_tga_stdio.h); tga.h and
 * tga.c carry marked VC1 patches. LTCG passes the TGA handle in EAX (TGARead
 * EDI, __TGASeek ESI, TGAClose EDI) and specialises TGAReadScanlines for one
 * scanline without colour swapping. */

// FUNCTION: VC1 0x00459e40
// __TGASeek

// FUNCTION: VC1 0x00459ec0
// TGAClose

// FUNCTION: VC1 0x00459f10
// TGAOpen

// FUNCTION: VC1 0x004ba050
// TGAReadRLE

// FUNCTION: VC1 0x004ba1a0
// TGARead

// FUNCTION: VC1 0x004ba220
// TGAReadScanlines

// FUNCTION: VC1 0x004ba3c0
// TGAReadHeader

#endif
