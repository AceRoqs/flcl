#include "stdafx.h"
#include "tinycrt.h"

static DWORD __thread_locals;

DWORD _Get_thread_locals()
{
    return __thread_locals;
}

extern "C" int __cdecl _purecall()
{
    abort();
}

// This function parses slightly differently than the MS CRT.  In particular, multiple
// backslashes have no special consideration, and escapes outside of quotes have no effect.
// http://msdn.microsoft.com/en-us/library/windows/desktop/17w5ykft(v=vs.85).aspx.
static int argc_from_command_line(_In_ PCTSTR command_line, _Out_ PTSTR** argv_out)
{
    int argc;
    *argv_out = nullptr;

    for(auto pass = 0; pass < 2; ++pass)
    {
        argc = 0;
        PCTSTR char_ptr = command_line;

        while(*char_ptr != TEXT('\0'))
        {
            while(*char_ptr == TEXT(' '))
            {
                ++char_ptr;
            }

            bool in_quote = *char_ptr == TEXT('"');
            const PCTSTR token_start = in_quote ? char_ptr + 1 : char_ptr;
            PCTSTR token_end;

            if(in_quote)
            {
                ++char_ptr;
                while((*char_ptr != TEXT('\0')) && (*char_ptr != TEXT('"')))
                {
                    const bool escape = *char_ptr == TEXT('\\');

                    ++char_ptr;

                    if(escape && (*char_ptr == TEXT('"')))
                    {
                        ++char_ptr;
                    }
                }

                token_end = char_ptr;

                if(*char_ptr == TEXT('"'))
                {
                    ++char_ptr;
                }
            }
            else
            {
                while((*char_ptr != TEXT('\0')) && (*char_ptr != TEXT(' ')))
                {
                    ++char_ptr;
                }

                token_end = char_ptr;
            }

            // Handle double quote case.
            if(token_end == token_start)
            {
                continue;
            }

            if(pass == 1)
            {
                (*argv_out)[argc] = reinterpret_cast<PTSTR>(malloc((token_end - token_start + 1) * sizeof(*token_start)));
                if((*argv_out)[argc] == nullptr)
                {
                    abort();
                }

                memcpy((*argv_out)[argc], token_start, (token_end - token_start) * sizeof(*token_start));
                (*argv_out)[argc][token_end - token_start] = TEXT('\0');
            }
            ++argc;
        }

        if((pass == 0) && (argc > 0))
        {
            *argv_out = reinterpret_cast<PTSTR*>(malloc(argc * sizeof(PTSTR)));
            if(*argv_out == nullptr)
            {
                abort();
            }
        }
    }

    return argc;
}

static void free_args(int argc, _In_count_(argc) PTSTR argv[])
{
    for(int ii = 0; ii < argc; ++ii)
    {
        free(argv[ii]);
    }

    if(argv != nullptr)
    {
        free(argv);
    }
}

static void setup_tls()
{
    __thread_locals = TlsAlloc();
    if(_Get_thread_locals() == TLS_OUT_OF_INDEXES)
    {
        abort();
    }

    auto thread_locals = reinterpret_cast<_Thread_locals*>(malloc(sizeof(_Thread_locals)));
    if(thread_locals == nullptr)
    {
        abort();
    }

    if(!TlsSetValue(_Get_thread_locals(), thread_locals))
    {
        abort();
    }
}

static void free_tls()
{
    // Do null check only for TlsGetValue, as that is state not maintained by the CRT.
    auto thread_locals = reinterpret_cast<_Thread_locals*>(TlsGetValue(_Get_thread_locals()));
    if(thread_locals == nullptr)
    {
        abort();
    }

    free(thread_locals);
    TlsFree(_Get_thread_locals());
}

#ifdef _UNICODE
#define _tmainCRTStartup wmainCRTStartup
#else
#define _tmainCRTStartup mainCRTStartup
#endif

extern "C" void __cdecl _tmainCRTStartup()
{
    setup_tls();
    PTSTR command_line = GetCommandLine();
    PTSTR* argv = nullptr;
    const int argc = argc_from_command_line(command_line, &argv);

    const int exit_code = _tmain(argc, argv);

    free_args(argc, argv);
    free_tls();
    exit(exit_code);
}

