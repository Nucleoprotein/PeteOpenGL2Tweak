#pragma once

#include <windows.h>

template<typename T>
inline bool SafeWrite(void* addr, const T data)
{
    DWORD oldProtect;
    if (VirtualProtect(addr, sizeof(T), PAGE_EXECUTE_READWRITE, &oldProtect))
    {
        *((T*)addr) = data;
        VirtualProtect(addr, sizeof(T), oldProtect, &oldProtect);
        return true;
    }
    return false;
}
