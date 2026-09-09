#ifndef VC1_KD_IGRAPHICS_H
#define VC1_KD_IGRAPHICS_H

namespace vc1 {

// SIZE 0x04. KdDisplay base at +0x10, between IRenderTarget and IDisplay.
// The name is inferred from g_iidGraphics. Vtable 0x004d0bb4 has twelve
// graphics operations plus a destructor; it does not use the IUnknown ABI.
struct IGraphics
{
    virtual int Graphics00(void* a, void* b);
    virtual int Graphics04(unsigned a, unsigned b);
    virtual int Graphics08(void* a, void* b, void* c, void* d, void* e);
    virtual int Graphics0c(void* a, void* b, void* c, void* d, void* e);
    virtual int Graphics10(void* a, void* b, void* c, void* d);
    virtual int Graphics14(void* a);
    virtual int Graphics18(void* a, void* b, void* c, void* d, void* e);
    virtual int Graphics1c(void* a);
    virtual int Graphics20(void* a, unsigned b);
    virtual int Graphics24(void* a, void* b, void* c, void* d);
    virtual int Graphics28(void* a, void* b, void* c, void* d);
    virtual int Graphics2c();
    virtual ~IGraphics();
};

typedef char Vc1IGraphicsSizeCheck[sizeof(IGraphics) == 0x04 ? 1 : -1];

}

#endif
