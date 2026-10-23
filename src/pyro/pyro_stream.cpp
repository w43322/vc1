#include "pyro/pyro_stream.h"
#include "pyro/pyro_image.h"

namespace vc1 {

// Tell, seek to the end, tell, seek back.
// FUNCTION: VC1 0x0042e6e0
int PyroStreamBase::GetLength()
{
    const int position = Tell();
    Seek(0, 2);
    const int length = Tell();
    Seek(position, 0);
    return length;
}

// RET 4.
// FUNCTION: VC1 0x0042e720
int PyroStreamBase::Skip(int count)
{
    return Seek(count, 1);
}

// FUNCTION: VC1 0x0042e820
int PyroStreamReader::Read(void* buffer, int size)
{
    return stream->Read(buffer, size);
}

// FUNCTION: VC1 0x0042e830 FOLDED
int PyroStreamReader::Write(const void* buffer, int size)
{
    return stream->Write(buffer, size);
}

// FUNCTION: VC1 0x0042e840
int PyroStreamReader::Seek(int offset, int origin)
{
    return stream->Seek(offset, origin);
}

// FUNCTION: VC1 0x0042e850
int PyroStreamReader::Tell()
{
    return stream->Tell();
}

// FUNCTION: VC1 0x0042e830 FOLDED
int PyroImageStreamReader::Read(void* buffer, int size)
{
    return stream->Read(buffer, size);
}

// XOR EAX, EAX; RET 8.
// FUNCTION: VC1 0x0042e860 FOLDED
int PyroImageStreamReader::Write(const void* buffer, int size)
{
    (void)buffer;
    (void)size;
    return 0;
}

// FUNCTION: VC1 0x0042e870
int PyroImageStreamReader::GetLength()
{
    return stream->GetLength();
}

// FUNCTION: VC1 0x0042e880
int PyroImageStreamReader::Seek(int offset, int origin)
{
    return stream->Seek(offset, origin);
}

// FUNCTION: VC1 0x0042e890
int PyroImageStreamReader::Tell()
{
    return stream->Tell();
}

}
