// ws2_32.dll over SDL3. The game opens no sockets: startup and shutdown work,
// the socket calls (only the unreferenced KD network sample uses them) fail.
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

SOCKET WINAPI socket(int af, int type, int protocol)
{
    (void)af;
    (void)type;
    (void)protocol;
    WSASetLastError(WSAENETDOWN);
    return INVALID_SOCKET;
}

int WINAPI connect(SOCKET socket, const sockaddr* name, int length)
{
    (void)socket;
    (void)name;
    (void)length;
    WSASetLastError(WSAENETDOWN);
    return SOCKET_ERROR;
}

int WINAPI WSAAsyncSelect(SOCKET socket, HWND window, UINT message, LONG events)
{
    (void)socket;
    (void)window;
    (void)message;
    (void)events;
    WSASetLastError(WSAENETDOWN);
    return SOCKET_ERROR;
}

int WINAPI recvfrom(SOCKET socket, char* buffer, int length, int flags, sockaddr* from, int* fromLength)
{
    (void)socket;
    (void)buffer;
    (void)length;
    (void)flags;
    (void)from;
    (void)fromLength;
    WSASetLastError(WSAENETDOWN);
    return SOCKET_ERROR;
}

int WINAPI sendto(SOCKET socket, const char* buffer, int length, int flags, const sockaddr* to, int toLength)
{
    (void)socket;
    (void)buffer;
    (void)length;
    (void)flags;
    (void)to;
    (void)toLength;
    WSASetLastError(WSAENETDOWN);
    return SOCKET_ERROR;
}

unsigned short WINAPI htons(unsigned short value)
{
    return static_cast<unsigned short>((value << 8) | (value >> 8));
}

char* WINAPI inet_ntoa(in_addr address)
{
    static char text[16];
    const unsigned long a = address.s_addr;
    SDL_snprintf(text, sizeof(text), "%lu.%lu.%lu.%lu", a & 0xff, (a >> 8) & 0xff, (a >> 16) & 0xff,
        (a >> 24) & 0xff);
    return text;
}
