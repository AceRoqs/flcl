#include "stdafx.h"
#include "cstdio"   // TODO: can/should this use <> syntax?
#include "tinycrt.h"

// Definitions from Shlwapi.h.  Shlwapi.h cannot be included as it brings in MSVCRT definitions.
#define LWSTDAPI_(type)   EXTERN_C DECLSPEC_IMPORT type STDAPICALLTYPE
LWSTDAPI_(int)      wvnsprintfA(_Out_writes_(cchDest) PSTR pszDest, _In_ int cchDest, _In_ _Printf_format_string_ PCSTR pszFmt, _In_ va_list arglist);
LWSTDAPI_(int)      wvnsprintfW(_Out_writes_(cchDest) PWSTR pszDest, _In_ int cchDest, _In_ _Printf_format_string_ PCWSTR pszFmt, _In_ va_list arglist);

extern "C" struct __detail::FILE* _CDECL __file_handle(int _Index)
{
    struct __detail::FILE* file_handle = nullptr;

    if(_Index == 1)
    {
        file_handle = reinterpret_cast<__detail::FILE*>(GetStdHandle(STD_OUTPUT_HANDLE));
    }
    else if(_Index == 2)
    {
        file_handle = reinterpret_cast<__detail::FILE*>(GetStdHandle(STD_ERROR_HANDLE));
    }

    return file_handle;
}

int fprintf(_In_ struct __detail::FILE* _File, _In_z_ const char* _Format, ...)
{
    auto thread_locals = reinterpret_cast<_Thread_locals*>(TlsGetValue(_Get_thread_locals()));

    va_list args;
    va_start(args, _Format);
    // TODO: does wvnsprintfA take DBCS or UTF-8?
    int count = wvnsprintfA(thread_locals->printf_ansi_buffer,
                            sizeof(thread_locals->printf_ansi_buffer),
                            _Format,
                            args);
    va_end(args);

    // count does not include null terminator.
    if(count > 0)
    {
        DWORD written;
        WriteFile(reinterpret_cast<HANDLE>(_File), thread_locals->printf_ansi_buffer, count, &written, nullptr);
    }

    return 0;
}

int fwprintf(_In_ struct __detail::FILE* _File, _In_z_ const wchar_t* _Format, ...)
{
    auto thread_locals = reinterpret_cast<_Thread_locals*>(TlsGetValue(_Get_thread_locals()));

    va_list args;
    va_start(args, _Format);
    int count = wvnsprintfW(thread_locals->printf_wchar_buffer,
                            ARRAYSIZE(thread_locals->printf_wchar_buffer),
                            _Format,
                            args);
    va_end(args);

    // count does not include null terminator.
    if(count > 0)
    {
        // WC2MB return value includes null terminator.
        // TODO: support DBCS or UTF8?
        count = WideCharToMultiByte(CP_ACP,
                                    0,
                                    thread_locals->printf_wchar_buffer,
                                    -1,
                                    thread_locals->printf_ansi_buffer,
                                    sizeof(thread_locals->printf_ansi_buffer),
                                    nullptr,
                                    nullptr);
        if(count > 0)
        {
            // -1 removes null.
            DWORD written;
            WriteFile(reinterpret_cast<HANDLE>(_File), thread_locals->printf_ansi_buffer, count - 1, &written, nullptr);
        }
    }

    return 0;
}

int printf(_In_z_ const char* _Format, ...)
{
    // TODO: need tinycrt specific versions of va_list/va_start/va_end.
    va_list args;
    va_start(args, _Format);
    fprintf(stdout, _Format, args);
    va_end(args);

    return 0;
}

int wprintf(_In_z_ const wchar_t* _Format, ...)
{
    va_list args;
    va_start(args, _Format);
    fwprintf(stderr, _Format, args);
    va_end(args);

    return 0;
}

