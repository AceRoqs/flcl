#ifndef _TINYCRT_H
#define _TINYCRT_H

// 7.19.6.1: Max production is 4096 characters.
#define MAX_PRINTF_SIZE 4096

// TODO: need to define the packing for this.
struct _Thread_locals
{
    // TODO: support DBCS or UTF-8?
    char printf_ansi_buffer[MAX_PRINTF_SIZE * 2];   // *2 since DBCS is up to two bytes per character.
    wchar_t printf_wchar_buffer[MAX_PRINTF_SIZE];
};

DWORD _Get_thread_locals();

#if 0
// TODO: vadefs.h?
#define _ADDRESSOF(v)   ( &reinterpret_cast<const char &>(v) )
#define _INTSIZEOF(n)   ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )

typedef char* va_list;
#define va_start(ap,v)  ( ap = (va_list)_ADDRESSOF(v) + _INTSIZEOF(v) )
#define va_end(ap)      ( ap = nullptr )
#endif

#define _CDECL __cdecl
#define _NORETURN __declspec(noreturn)

// TODO: Is extern "C" valid in C programs?
extern "C" _NORETURN void _CDECL exit(int _Status);
extern "C" _NORETURN void _CDECL abort();
extern "C" void* _CDECL malloc(size_t _Size);
extern "C" void* _CDECL realloc(_In_ void* _Ptr, size_t _Size);
extern "C" void _CDECL free(_In_opt_ void* _Ptr);
extern "C" void* _CDECL memcpy(_Out_writes_bytes_all_(_Size) void* _Dst, _In_reads_bytes_(_Size) const void* _Src, size_t _Size);

#ifdef _UNICODE
#define _T(x) L##x
int wmain(int argc, _In_count_(argc) wchar_t* argv[]);
#define _tmain wmain
#define _tprintf wprintf
#define _ftprintf fwprintf
#else
#define _T(x) x
int main(int argc, _In_count_(argc) char* argv[]);
#define _tmain main
#define _tprintf printf
#define _ftprintf fprintf
#endif

#endif

