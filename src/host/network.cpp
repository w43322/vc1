#include "host/runtime.h"

#include <stdlib.h>

namespace vc1 {

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
                void* const event = AllocEvent();
                if (event) {
                    *reinterpret_cast<unsigned*>(static_cast<unsigned char*>(event) + 0xc) =
                        node->unused04;
                }
                {
                    const unsigned kind = (static_cast<unsigned>(lparam) & 0xffff) - 1;
                    if (kind <= 0xf) {
                        if (event) {
                            unsigned* const fields = static_cast<unsigned*>(event);
                            if (kind == 0) {
                                fields[2] = 0x31;
                            }
                            else if (kind == 1) {
                                fields[2] = 0x32;
                            }
                            fields[4] = reinterpret_cast<unsigned>(node);
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

}
