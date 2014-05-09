#include "stdafx.h"
#include "tinycrt.h"

// TODO: single exit from function
// TODO: move to property sheet
// TODO: break up into crt0.cpp etc.

// string.h
extern "C" void* __cdecl memcpy(_Out_writes_bytes_all_(_Size) void* _Dst, _In_reads_bytes_(_Size) const void* _Src, size_t _Size)
{
    auto _Dst1 = reinterpret_cast<char*>(_Dst);
    auto _Src1 = reinterpret_cast<const char*>(_Src);
    for(size_t ix = 0; ix < _Size; ++ix)
    {
        _Dst1[ix] = _Src1[ix];
    }

    return _Dst;
}

