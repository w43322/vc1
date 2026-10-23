// ws2_32.dll over SDL3. The game opens no sockets; startup and shutdown only.
#include "sdl3.h"

namespace {
SDL_TLSID lastErrorSlot;
}

int WINAPI WSAStartup(WORD version, WSADATA* data)
{
    SDL_memset(data, 0, sizeof(*data));
    data->wVersion = version;
    data->wHighVersion = version;
    return 0;
}

int WINAPI WSACleanup()
{
    return 0;
}

int WINAPI closesocket(SOCKET socket)
{
    (void)socket;
    return 0;
}

int WINAPI WSAGetLastError()
{
    return static_cast<int>(reinterpret_cast<intptr_t>(SDL_GetTLS(&lastErrorSlot)));
}

void WINAPI WSASetLastError(int error)
{
    SDL_SetTLS(&lastErrorSlot, reinterpret_cast<void*>(static_cast<intptr_t>(error)), 0);
}
