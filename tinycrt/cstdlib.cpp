#include "stdafx.h"
#include "tinycrt.h"

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

extern "C" _NORETURN void _CDECL exit(int _Status)
{
    ExitProcess(_Status);
}

extern "C" _NORETURN void _CDECL abort()
{
    exit(EXIT_FAILURE);
}

extern "C" void* _CDECL malloc(size_t _Size)
{
    return HeapAlloc(GetProcessHeap(), 0, _Size);
}

extern "C" void* _CDECL realloc(_In_ void* _Ptr, size_t _Size)
{
    if(_Ptr == nullptr)
    {
        return malloc(_Size);
    }

    return HeapReAlloc(GetProcessHeap(), 0, _Ptr, _Size);
}

extern "C" void _CDECL free(_In_opt_ void* _Ptr)
{
    if(_Ptr != nullptr)
    {
        HeapFree(GetProcessHeap(), 0, _Ptr);
    }
}

