#include "integral.h"

namespace vc1 {

// Bound as "SetIntegralSize" (0x004d7438) via 0x00423410 (dispatcher
// 0x00424af0). this in ECX; RET 4. Unsigned clamp to at least one sample.
// FUNCTION: VC1 0x0041dbc0
void IntegralF::SetIntegralSize(unsigned size)
{
    integralSize = size < 1 ? 1 : size;
}

// Bound as "PushValue" (0x004d7448) via 0x004234c0 (dispatcher 0x00424b40).
// this in ECX, value on the stack; RET 4. Drops the oldest sample, then
// fills the window back up to integralSize with value.
// FUNCTION: VC1 0x0041dbe0
void IntegralF::PushValue(SQFloat value)
{
    if (values.size != 0) {
        unsigned char* const first = reinterpret_cast<unsigned char*>(values.begin);
        EraseCountedBytes(first, first + sizeof(SQFloat), first + values.size);
        values.size -= sizeof(SQFloat);
    }
    while (values.size / sizeof(SQFloat) != integralSize) {
        const unsigned bytes = (values.size / sizeof(SQFloat) + 1) * sizeof(SQFloat);
        if (values.capacity < bytes) {
            GrowCountedBuffer(&values, static_cast<int>(bytes), 0);
        }
        values.size = bytes;
        *reinterpret_cast<SQFloat*>(
            reinterpret_cast<unsigned char*>(values.begin) + bytes - sizeof(SQFloat)) = value;
    }
}

// Bound as "Clear" (0x004d17c8) via 0x00423570 (dispatcher 0x00424060).
// this in ECX. Resize-to-zero of the sample buffer.
// FUNCTION: VC1 0x0041dc50
void IntegralF::Clear()
{
    if (values.capacity < values.Count()) {
        GrowCountedBuffer(&values, 0, 1);
    }
    values.size = 0;
}

// Bound as "GetAverageValue" (0x004d7454) via 0x00422b70 from 0x004c1578.
// Integral divided by the unsigned sample count.
// FUNCTION: VC1 0x0041dc80
SQFloat IntegralF::GetAverageValue() const
{
    if (values.size == 0) {
        return 0.0f;
    }
    const SQFloat sum = SumFloats(First() + 1, Last(), *First());
    return sum / static_cast<SQFloat>(values.size / sizeof(SQFloat));
}

// Bound as "GetIntegralValue" (0x004d7464) via 0x00422b70 from 0x004c1594.
// FUNCTION: VC1 0x0041dcd0
SQFloat IntegralF::GetIntegralValue() const
{
    if (values.size == 0) {
        return 0.0f;
    }
    return SumFloats(First() + 1, Last(), *First());
}

// Inferred name. ECX = first, ESI = last, sum on the stack; result in ST0.
// The trip count rounds up, so the loop is first < last, not std::accumulate.
// FUNCTION: VC1 0x0041ee20
SQFloat SumFloats(const SQFloat* first, const SQFloat* last, SQFloat sum)
{
    for (; first < last; ++first) {
        sum = sum + *first;
    }
    return sum;
}

// Inferred name. EAX = values, EBX = last, first on the stack (RET 4).
// Sizes the buffer to the range in whole floats (reserving exactly through
// GrowCountedBuffer 0x004b8930) and copies the samples.
// FUNCTION: VC1 0x00427300
void AssignSamples(KdFactory& values, const SQFloat* first, const SQFloat* last)
{
    const unsigned bytes = static_cast<unsigned>(last - first) * sizeof(SQFloat);
    if (values.capacity < bytes) {
        GrowCountedBuffer(&values, static_cast<int>(bytes), 1);
    }
    SQFloat* out = reinterpret_cast<SQFloat*>(values.begin);
    values.size = bytes;
    for (; first != last; ++first, ++out) {
        *out = *first;
    }
}

}
