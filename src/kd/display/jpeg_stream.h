#ifndef VC1_KD_DISPLAY_JPEG_STREAM_H
#define VC1_KD_DISPLAY_JPEG_STREAM_H

// The vendored jpgd 0.93b with its header-inline jpeg_decoder_file_stream
// compiled on KD files, as the game did: open goes through OpenKdFile, read
// (0x0041aab0) through KdFread and an error check through driver+0x138 that
// sets errno 0x11 on a null handle, the destructor (0x0041aa60) closes
// through KdFclose. Name comments for the library are in jpgd093b.h.

#include "kdcore/kd_tga_stdio.h"

namespace vc1 {

inline int KdFerror(void* file)
{
    KdFileHandle* const handle = static_cast<KdFileHandle*>(file);
    if (!handle) {
        errno = 0x11;
        return 0;
    }
    return handle->driver->error(handle->driver, handle->native);
}

}

#undef ferror
#define ferror(file) vc1::KdFerror(file)

#include "jpegdecoder.h"
#include "jpgd093b.h"

#undef ferror

#endif
