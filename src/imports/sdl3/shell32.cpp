// shell32.dll over SDL3: opening documents and URLs, and the AppData folder.
#include "sdl3.h"

#include <stdlib.h>

// The game opens web pages; ShellExecute reports success with a value above 32.
HINSTANCE WINAPI ShellExecuteA(HWND window, LPCSTR operation, LPCSTR file, LPCSTR parameters,
    LPCSTR directory, int show)
{
    (void)window;
    (void)operation;
    (void)parameters;
    (void)directory;
    (void)show;
    return reinterpret_cast<HINSTANCE>(static_cast<uintptr_t>(SDL_OpenURL(file) ? 33 : 2));
}

// CSIDL_APPDATA is %APPDATA% on Windows; elsewhere the home folder.
BOOL WINAPI SHGetSpecialFolderPathA(HWND window, LPSTR path, int folder, BOOL create)
{
    (void)window;
    if (folder != CSIDL_APPDATA) {
        return FALSE;
    }
    const char* base = getenv("APPDATA");
    if (!base) {
        base = SDL_GetUserFolder(SDL_FOLDER_HOME);
    }
    if (!base) {
        return FALSE;
    }
    SDL_strlcpy(path, base, MAX_PATH);
    size_t length = SDL_strlen(path);
    while (length > 1 && (path[length - 1] == '\\' || path[length - 1] == '/')) {
        path[--length] = 0;
    }
    if (create) {
        SDL_CreateDirectory(path);
    }
    return TRUE;
}
