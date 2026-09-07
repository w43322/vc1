#ifndef VC1_KD_IINPUT_STREAM_H
#define VC1_KD_IINPUT_STREAM_H

#include "kd/iface/iunknown.h"

namespace vc1 {

// SIZE 0x24
struct IInputStream : IUnknown
{
    unsigned pad04[8]; // 0x04
};

typedef char Vc1IInputStreamSizeCheck[sizeof(IInputStream) == 0x24 ? 1 : -1];

}

#endif
