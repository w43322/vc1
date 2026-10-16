#ifndef VC1_KD_IPYRO_DEVICE_H
#define VC1_KD_IPYRO_DEVICE_H

namespace vc1 {

struct PyroDeviceImage;
struct PyroDeviceObject;

// Inferred name. KdDisplayDevice SetSize (0x004641b0) hands this base to the
// Pyro library Init slot ("Can't init Pyro"). No IID and no IUnknown slots.
// Slot names are inferred from vtable offsets. Defaults at 0x0041aca0..0x0041ad20.
// SIZE 0x04
// VTABLE: VC1 0x004d6288
struct IPyroDevice
{
    virtual int Pyro00(void* a, void* b) = 0;                         // vtable+0x00
    virtual int Pyro04(unsigned mode, unsigned value) = 0;            // vtable+0x04
    virtual int Pyro08(void** out, int count, int stride, unsigned format, int flags); // vtable+0x08
    virtual int Pyro0c(void** out, int count, int stride, unsigned format, int flags); // vtable+0x0c
    virtual int Pyro10(void* a, void* b, void* c, void* d) = 0;       // vtable+0x10
    virtual int Pyro14(PyroDeviceObject** out, PyroDeviceImage* image, int flags) = 0; // vtable+0x14
    virtual int Pyro18(PyroDeviceImage** out, void* pixels, int width, int height,
        int flags) = 0;                                               // vtable+0x18
    virtual int Pyro1c(void* a) = 0;                                  // vtable+0x1c
    virtual int Pyro20(void* a, unsigned value) = 0;                  // vtable+0x20
    virtual int Pyro24(void* a, void* b, void* c, void* d);           // vtable+0x24
    virtual int Pyro28(void* a, void* b, void* c, void* d);           // vtable+0x28
    virtual int Pyro2c() = 0;                                         // vtable+0x2c
    virtual ~IPyroDevice();                                           // vtable+0x30
};

typedef char Vc1IPyroDeviceSizeCheck[sizeof(IPyroDevice) == 0x04 ? 1 : -1];

// SYNTHETIC: VC1 0x0041ad50
// vc1::IPyroDevice::`scalar deleting destructor'

}

#endif
