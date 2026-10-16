#include "pyro/pyro_matrix.h"

#include <math.h>
#include <string.h>

namespace vc1 {

// EAX=this, RET 0x10. Scales each column.
// FUNCTION: VC1 0x0044cee0
void PyroMatrix34::Scale(float x, float y, float z, float w)
{
    m[0][0] = x * m[0][0];
    m[0][1] = y * m[0][1];
    m[0][2] = z * m[0][2];
    m[0][3] = w * m[0][3];
    m[1][0] = m[1][0] * x;
    m[1][1] = m[1][1] * y;
    m[1][2] = m[1][2] * z;
    m[1][3] = m[1][3] * w;
    m[2][0] = m[2][0] * x;
    m[2][1] = m[2][1] * y;
    m[2][2] = z * m[2][2];
    m[2][3] = w * m[2][3];
}

// ESI=this, EDI=q. Rotation part from a unit quaternion, no translation.
// FUNCTION: VC1 0x0044d290
void PyroMatrix34::SetRotation(const PyroQuat* q)
{
    memset(this, 0, sizeof(*this));
    const float x2 = q->x + q->x;
    const float y2 = q->y + q->y;
    const float z2 = q->z + q->z;
    const float wx = q->w * x2;
    const float base = 1.0f - x2 * q->x;

    m[0][0] = 1.0f - y2 * q->y - z2 * q->z;
    m[1][1] = base - z2 * q->z;
    m[2][2] = base - y2 * q->y;
    m[0][1] = y2 * q->x - q->w * z2;
    m[1][0] = q->w * z2 + y2 * q->x;
    m[0][2] = q->w * y2 + z2 * q->x;
    m[2][0] = z2 * q->x - q->w * y2;
    m[1][2] = q->z * y2 - wx;
    m[2][1] = wx + q->z * y2;
    m[2][3] = 0.0f;
    m[1][3] = 0.0f;
    m[0][3] = 0.0f;
}

// ESI=this, EDI=source; returns this.
// FUNCTION: VC1 0x0044d3c0
PyroQuat* PyroQuat::FromAxisAngle(const PyroAxisAngle* source)
{
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
    const float half = source->angle * 0.5f;
    const float s = sinf(half);
    w = cosf(half);
    x = source->axis[0] * s;
    y = source->axis[1] * s;
    z = s * source->axis[2];
    return this;
}

// ESI=this, EDI=out, RET 4. out = b * this; returns out.
// FUNCTION: VC1 0x0044d510
PyroMatrix34* PyroMatrix34::Multiply(PyroMatrix34* out, const PyroMatrix34* b) const
{
    memset(out, 0, sizeof(*out));
    for (int r = 0; r < 3; ++r) {
        const float* const row = b->m[r];
        const float w = row[3] * 0.0f;
        out->m[r][0] = m[2][0] * row[2] + m[1][0] * row[1] + m[0][0] * row[0] + w;
        out->m[r][1] = m[2][1] * row[2] + m[1][1] * row[1] + m[0][1] * row[0] + w;
        out->m[r][2] = m[2][2] * row[2] + m[0][2] * row[0] + m[1][2] * row[1] + w;
        out->m[r][3] = m[2][3] * row[2] + m[0][3] * row[0] + m[1][3] * row[1] + row[3];
    }
    return out;
}

}
