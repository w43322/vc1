// shlwapi.dll over SDL3.
#include "sdl3.h"

BOOL WINAPI PathAppendA(LPSTR path, LPCSTR more)
{
    size_t length = SDL_strlen(path);
    while (*more == '\\' || *more == '/') {
        ++more;
    }
    if (length && path[length - 1] != '\\' && path[length - 1] != '/') {
        if (length + 1 >= MAX_PATH) {
            return FALSE;
        }
        path[length++] = '\\';
        path[length] = 0;
    }
    return SDL_strlcat(path, more, MAX_PATH) < MAX_PATH;
}
