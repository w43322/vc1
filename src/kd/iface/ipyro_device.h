#ifndef VC1_KD_IPYRO_DEVICE_H
#define VC1_KD_IPYRO_DEVICE_H

namespace vc1 {

struct PyroDeviceImage;
struct PyroDeviceObject;
struct PyroDeviceBuffer;
struct PyroVertexBuffer;

// Inferred name. KdDisplayDevice SetSize (0x004641b0) hands this base to the
// Pyro library Init slot ("Can't init Pyro"). No IID and no IUnknown slots.
// Slot names are inferred from the library's call sites: SetRenderState(0, 1)
// before every layer render (a guess; KdDisplayDevice ignores it),
// SetBlendMode(src, dst), SetWorldMatrix(&matrix) for 3D layers, IsRgba picks
// the vertex colour byte order. Unused has no caller and returns 1.
// Defaults at 0x0041aca0..0x0041ad20.
// SIZE 0x04
// VTABLE: VC1 0x004d6288
struct IPyroDevice
{
    virtual int SetRenderState(int a, int b) = 0;                         // vtable+0x00
    virtual int SetBlendMode(unsigned mode, unsigned value) = 0;            // vtable+0x04
    virtual int CreateBuffer(PyroDeviceBuffer** out, int count, int stride, unsigned format,
        int flags);                                                   // vtable+0x08
    virtual int CreateVertexBuffer(PyroVertexBuffer** out, int count, int stride, unsigned format,
        int flags);                                                   // vtable+0x0c
    virtual int Unused(void* a, void* b, void* c, void* d) = 0;       // vtable+0x10
    virtual int CreateImageTexture(PyroDeviceObject** out, PyroDeviceImage* image, int flags) = 0; // vtable+0x14
    virtual int CreateImage(PyroDeviceImage** out, void* pixels, int width, int height,
        int flags) = 0;                                               // vtable+0x18
    virtual int SetWorldMatrix(void* a) = 0;                                  // vtable+0x1c
    virtual int SetTexture(void* a, PyroDeviceObject* texture) = 0;       // vtable+0x20
    virtual int DrawQuadBuffer(PyroDeviceBuffer* buffer, int first, int count, int flags); // vtable+0x24
    virtual int DrawQuads(PyroVertexBuffer* buffer, int first, int count, int flags); // vtable+0x28
    virtual int IsRgba() = 0;                                         // vtable+0x2c
    virtual ~IPyroDevice();                                           // vtable+0x30
};

typedef char Vc1IPyroDeviceSizeCheck[sizeof(IPyroDevice) == 0x04 ? 1 : -1];

// SYNTHETIC: VC1 0x0041ad50
// vc1::IPyroDevice::`scalar deleting destructor'

}

#endif
