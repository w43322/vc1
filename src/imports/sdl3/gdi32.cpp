// gdi32.dll over SDL3. The window class's background brush is never drawn.
#include "sdl3.h"

HGDIOBJ WINAPI GetStockObject(int object)
{
    (void)object;
    return 0;
}
