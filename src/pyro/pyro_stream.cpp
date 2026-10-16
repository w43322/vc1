#include "pyro/pyro_stream.h"

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

// Also slot 1 of the 0x004d7cb8 stream (ICF).
// FUNCTION: VC1 0x0042e830
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

}
