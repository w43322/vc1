#include "kdcore/runtime.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

namespace vc1 {

// The OpenKODE network layer: socket nodes on the g_socketHead list, posted
// to the [kdNetwork] window with WSAAsyncSelect. Nothing in the game creates
// a socket; the create/connect/lookup helpers and the test client handler
// below are unreferenced sample code (inferred names).

// KDSockaddr: KD_AF_INET (0x46), then the port and the IPv4 address in
// network order.
struct KdSockaddr
{
    unsigned short family; // +0x00
    unsigned short pad02;
    unsigned short port;   // +0x04
    unsigned short pad06;
    unsigned address;      // +0x08
};

void SocketEventHandler(const KdEvent* event);

// EAX=address, cdecl node. connect(); a pending non-blocking connect
// (WSAEWOULDBLOCK) counts as success.
// FUNCTION: VC1 0x0047bf10
int SocketConnect(const KdSockaddr* address, SocketNode* node)
{
    sockaddr_in name;

    name.sin_family = AF_INET;
    name.sin_port = address->port;
    name.sin_addr.s_addr = address->address;
    const int result = connect(node->socket, reinterpret_cast<sockaddr*>(&name), sizeof(name));
    if (result == 0) {
        return 0;
    }
    return WSAGetLastError() != WSAEWOULDBLOCK ? result : 0;
}

// EAX=node. Unlinks a node, moving the cursor and the head past it.
// FUNCTION: VC1 0x0047bf70
void UnlinkSocketNode(SocketNode* node)
{
    if (g_socketCursor == node) {
        g_socketCursor = node->prev;
    }
    if (g_socketHead == node) {
        g_socketHead = node->next;
    }
    if (node->next) {
        node->next->prev = node->prev;
    }
    if (node->prev) {
        node->prev->next = node->next;
    }
}

// EAX=node. Pushes a node at the head; the first one is also the cursor.
// FUNCTION: VC1 0x0047bfb0
void LinkSocketNode(SocketNode* node)
{
    SocketNode* const head = g_socketHead;

    node->next = head;
    node->prev = 0;
    g_socketHead = node;
    if (!head) {
        g_socketCursor = node;
        return;
    }
    head->prev = node;
}

// EAX=address, ESI=out[0x80]. kdInetNtop: the dotted address, or an empty
// string with errno 0x11 when it does not fit.
// FUNCTION: VC1 0x0047bfe0
char* InetNtop(const in_addr* address, char* out)
{
    const char* const text = inet_ntoa(*address);
    const unsigned length = static_cast<unsigned>(strlen(text)) + 1;

    if (length > 0x80) {
        *out = 0;
        errno = 0x11;
        return out;
    }
    memcpy(out, text, length);
    return out;
}

// EDI=socket. A node for a new socket, its events posted to the network
// window as WM_USER (FD_READ | FD_WRITE | FD_CONNECT | FD_CLOSE).
// FUNCTION: VC1 0x0047c020
SocketNode* CreateSocketNode(SOCKET socket)
{
    if (socket == INVALID_SOCKET) {
        return 0;
    }
    SocketNode* const node = static_cast<SocketNode*>(malloc(sizeof(SocketNode)));
    node->socket = socket;
    node->handler = reinterpret_cast<void*>(SocketEventHandler);
    WSAAsyncSelect(socket, g_kdNetworkWindow, 0x400, 0x1b);
    LinkSocketNode(node);
    return node;
}

// Closes the head node: its pending events, the node and the socket.
// FUNCTION: VC1 0x0047c080
void DestroyHeadSocket()
{
    SocketNode* const node = g_socketHead;
    const SOCKET socket = node->socket;

    UnlinkHeapByOwner(node);
    if (g_socketCursor == node) {
        g_socketCursor = node->prev;
    }
    g_socketHead = node->next;
    if (g_socketHead) {
        g_socketHead->prev = node->prev;
    }
    if (node->prev) {
        node->prev->next = node->next;
    }
    free(node);
    closesocket(socket);
}

// kdSocketCreate(KD_SOCK_TCP).
// FUNCTION: VC1 0x0047c0e0
SocketNode* CreateTcpSocket()
{
    return CreateSocketNode(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
}

// Walk DAT_004ed510, unlink, free, closesocket; DestroyWindow; tail-jump WSACleanup.
// FUNCTION: VC1 0x0047c100
void ShutdownNetwork()
{
    SocketNode* node = g_socketHead;
    while (node) {
        const SOCKET socket = node->socket;
        UnlinkHeapByOwner(node);
        if (g_socketCursor == node) {
            g_socketCursor = node->prev;
        }
        g_socketHead = node->next;
        if (g_socketHead) {
            g_socketHead->prev = node->prev;
        }
        if (node->prev) {
            node->prev->next = node->next;
        }
        free(node);
        closesocket(socket);
        node = g_socketHead;
    }

    DestroyWindow(g_kdNetworkWindow);
    g_kdNetworkWindow = 0;
    WSACleanup();
}

// Hidden [kdNetwork] window. WM_USER 0x400 walks DAT_004ed50c comparing socket == wParam.
// FUNCTION: VC1 0x0047c190
LRESULT CALLBACK KdNetworkWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    if (msg == 0x400) {
        SocketNode* node = g_socketCursor;
        while (node) {
            if (node->socket == static_cast<SOCKET>(wparam)) {
                KdEvent* const event = AllocEvent();
                if (event) {
                    event->userptr = node->handler;
                }
                {
                    const unsigned kind = (static_cast<unsigned>(lparam) & 0xffff) - 1;
                    if (kind <= 0xf) {
                        if (event) {
                            if (kind == 0) {
                                event->type = 0x31;
                            }
                            else if (kind == 1) {
                                event->type = 0x32;
                            }
                            event->data[0] = reinterpret_cast<unsigned>(node);
                            QueueEvent(event);
                        }
                        return 0;
                    }
                }
                QueueEvent(event);
                return 0;
            }
            node = node->prev;
        }
    }
    return DefWindowProcA(hwnd, msg, wparam, lparam);
}

// The sample client's event handler (cdecl event). Name lookup complete:
// log the address and connect a new TCP socket to its FTP port (21).
// Connect complete: log the result. Data (0x31): log what arrived and send
// "quit".
// FUNCTION: VC1 0x0047c280
void SocketEventHandler(const KdEvent* event)
{
    char text[0x80];
    KdSockaddr address;
    int fromLength;

    switch (event->type) {
    case 0x31: {
        SocketNode* const node = reinterpret_cast<SocketNode*>(event->data[0]);
        fromLength = 0xc;
        text[recvfrom(node->socket, text, 0x80, 0, 0, &fromLength)] = 0;
        KdLog(text);
        // STRING: VC1 0x004d4484
        sendto(reinterpret_cast<SocketNode*>(event->data[0])->socket, "quit\r\n", 6, 0, 0, 0xc);
        break;
    }
    case 0x33:
        // STRING: VC1 0x004d4458
        KdLog("KD_EVENT_SOCKET_CONNECT_COMPLETE: %x, %d\n", event->data[0], event->data[1]);
        break;
    case 0x35: {
        const unsigned* const lookup = reinterpret_cast<const unsigned*>(event->data[2]);
        // STRING: VC1 0x004d4434
        KdLog("KD_EVENT_NAME_LOOKUP_COMPLETE: %s\n",
            InetNtop(reinterpret_cast<const in_addr*>(lookup + 2), text));
        SocketNode* const node = CreateSocketNode(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
        address.family = 0x46;
        address.address = lookup[2];
        address.port = htons(0x15);
        SocketConnect(&address, node);
        break;
    }
    }
}

// The [kdNetwork] class, registered on first use (StartupNetwork inlines
// three tries of this).
// FUNCTION: VC1 0x0047c390
WNDCLASSA* NetworkWindowClass()
{
    if (g_kdNetworkClass.hInstance == 0) {
        g_kdNetworkClass.hInstance = GetModuleHandleA(0);
        RegisterClassA(&g_kdNetworkClass);
    }
    return &g_kdNetworkClass;
}

// WSAStartup(0x202), RegisterClassA of 0x004e89d8 retried three times, CreateWindowExA CW_USEDEFAULT.
// FUNCTION: VC1 0x0047c3c0
void StartupNetwork()
{
    WSADATA wsa;

    WSAStartup(0x202, &wsa);
    if (g_kdNetworkClass.hInstance == 0) {
        g_kdNetworkClass.hInstance = GetModuleHandleA(0);
        RegisterClassA(&g_kdNetworkClass);
        if (g_kdNetworkClass.hInstance == 0) {
            g_kdNetworkClass.hInstance = GetModuleHandleA(0);
            RegisterClassA(&g_kdNetworkClass);
            if (g_kdNetworkClass.hInstance == 0) {
                g_kdNetworkClass.hInstance = GetModuleHandleA(0);
                RegisterClassA(&g_kdNetworkClass);
            }
        }
    }

    g_kdNetworkWindow = CreateWindowExA(
        0,
        g_kdNetworkClass.lpszClassName,
        g_kdNetworkClass.lpszClassName,
        0,
        static_cast<int>(0x80000000),
        static_cast<int>(0x80000000),
        static_cast<int>(0x80000000),
        static_cast<int>(0x80000000),
        0,
        0,
        g_kdNetworkClass.hInstance,
        0);
}


// EDI=name, ESI=error. Inferred name. The system message for a Win32 error
// code, or the number when FormatMessageA has none.
// FUNCTION: VC1 0x0047c480
void LogLastError(const char* name, DWORD error)
{
    char* message;

    if (FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 0, error, 0,
            reinterpret_cast<LPSTR>(&message), 0, 0)) {
        // STRING: VC1 0x004d4408
        KdLog("[%s] error: %s\n", name, message);
        LocalFree(message);
    }
    else {
        // STRING: VC1 0x004d4418
        KdLog("[%s] error: %u\n", name, error);
    }
}

}
