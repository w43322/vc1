#ifndef VC1_HOST_WINDOW_H
#define VC1_HOST_WINDOW_H

// Opaque KD window service. HWND and native message records stay in the backend.
namespace vc1 {
void* CreateKdWindow();
extern void* g_kdWindowMixer; // KD event userptr is the first field.
extern const char kKdParentWindow[];
}
#endif
