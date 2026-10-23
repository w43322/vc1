// wininet.dll. There is no HTTP client: opening the session fails, so the
// "http" KD driver opens nothing.
#include <windows.h>

HINTERNET WINAPI InternetOpenA(LPCSTR, DWORD, LPCSTR, LPCSTR, DWORD)
{
    return 0;
}

HINTERNET WINAPI InternetOpenUrlA(HINTERNET, LPCSTR, LPCSTR, DWORD, DWORD, DWORD_PTR)
{
    return 0;
}

BOOL WINAPI InternetCloseHandle(HINTERNET)
{
    return TRUE;
}

BOOL WINAPI InternetReadFile(HINTERNET, void*, DWORD, DWORD* read)
{
    *read = 0;
    return FALSE;
}

DWORD WINAPI InternetSetFilePointer(HINTERNET, LONG, LONG*, DWORD, DWORD_PTR)
{
    return static_cast<DWORD>(-1);
}

BOOL WINAPI HttpQueryInfoA(HINTERNET, DWORD, void*, DWORD*, DWORD*)
{
    return FALSE;
}
