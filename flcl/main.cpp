#include "stdafx.h"
#include "tinycrt.h"
#include "cstdio"

// TODO: 
// minimize calls, heap/stack usage
// allow > MAX_PATH filenames
// error handling, const correctness, style
// breadth first is faster than depth first search.

// This is not a hard value - strings can be longer, but various methods
// may fail if this value is exceeded, in order to prevent arithmetic overflow.
#define _MAX_STRING_LEN 65536

template<typename Ty>
class Simple_string
{
    Ty* m_str;
    size_t m_reserve;

    template<typename Ty_str>
    void cat(_In_z_ const Ty_str* str);

    template<>
    void cat<char>(_In_z_ const char* str)
    {
        lstrcatA(m_str, str);
    }

    template<>
    void cat<wchar_t>(_In_z_ const wchar_t* str)
    {
        lstrcatW(m_str, str);
    }


public:
    Simple_string() : m_str(nullptr), m_reserve(0)
    {
    }

    ~Simple_string()
    {
        dispose();
    }

    // TODO: need a specific dispose method?
    void dispose()
    {
        if(m_str != nullptr)
        {
            free(m_str);
            m_str = nullptr;
        }
    }

    bool reserve(size_t characters)
    {
        if(characters > _MAX_STRING_LEN)
        {
            return false;
        }

        if(characters < m_reserve)
        {
            return true;
        }

        const auto str = reinterpret_cast<Ty*>(realloc(m_str, characters * sizeof(Ty)));
        if(str != nullptr)
        {
            m_reserve = characters;
            m_str = str;
        }

        return str != nullptr;
    }

    template<typename>
    size_t length();

    template<>
    size_t length<char>()
    {
        return static_cast<size_t>(lstrlenA(m_str));
    }

    template<>
    size_t length<wchar_t>()
    {
        return static_cast<size_t>(lstrlenW(m_str));
    }

    bool concat(Ty ch)
    {
        const auto len = static_cast<size_t>(length<Ty>() + 1);
        if(len > _MAX_STRING_LEN)
        {
            return false;
        }

        if(len == m_reserve)
        {
            const auto new_len = len + (len + 1) / 2;
            auto str = reinterpret_cast<Ty*>(realloc(m_str, new_len * sizeof(Ty)));
            if(str == nullptr)
            {
                return false;
            }

            m_reserve = new_len;
            m_str = str;
        }

        Ty str[2] = { ch, 0 };

        cat(str);
        return true;
    }

    Ty* c_str() const
    {
        return m_str;
    }
};

#if 0
// TODO: max string should be MAX_PATH;
PTSTR PathAddBackslash(_Inout_ PTSTR pszPath)
{
    PTSTR pszEnd = pszPath;
    while(*pszEnd != TEXT('\0'))
    {
        ++pszEnd;
    }

    bool add_slash = true;
    if(pszEnd != pszPath)
    {
        if(pszEnd[-1] == TEXT('\\'))
        {
            add_slash = false;
        }
    }

    if(add_slash)
    {
        *pszEnd = TEXT('\\');
        ++pszEnd;
        *pszEnd = TEXT('\0');
    }

    return pszEnd;
}
#endif

struct /*__declspec(novtable)*/ File_traverse_base
{
    virtual bool on_pre_folder(_In_ PCTSTR folder, bool hidden) = 0;
    virtual void on_post_folder(_In_ PCTSTR folder, bool hidden) = 0;
    virtual void on_file(_In_ PCTSTR filename, bool hidden) = 0;
};

#if 0
void process_folder_old(_In_ PCTSTR path, _In_ File_traverse_base* callback, bool recurse)
{
    TCHAR full_path[MAX_PATH];
    lstrcpy(full_path, path);
    const PTSTR filename = full_path + lstrlen(full_path);
    lstrcpy(filename, TEXT("*"));

    WIN32_FIND_DATA fd;
    HANDLE hFind = FindFirstFile(full_path, &fd);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (lstrcmp(fd.cFileName, TEXT(".")) != 0 && lstrcmp(fd.cFileName, TEXT("..")) != 0)
            {
                if (recurse && ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0))
                {
                    lstrcpy(filename, fd.cFileName);
                    lstrcat(filename, TEXT("\\"));

                    const bool hidden = (fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0;
                    if (callback->on_pre_folder(full_path, hidden))
                    {
                        process_folder_old(full_path, callback, recurse);
                        callback->on_post_folder(full_path, hidden);
                    }
                }
                else if ((fd.dwFileAttributes & FILE_ATTRIBUTE_NORMAL) != 0)
                {
                    lstrcpy(filename, fd.cFileName);
                    callback->on_file(filename, (fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0);
                }
            }
        } while (FindNextFile(hFind, &fd));

        FindClose(hFind);
    }
}
#endif

void process_folder(_In_ PCTSTR path, _In_ File_traverse_base* callback, bool recurse)
{
    //Simple_string<TCHAR> full_path2;
    //full_path2.
    TCHAR full_path[MAX_PATH];
    lstrcpy(full_path, path);
    const PTSTR filename = full_path + lstrlen(full_path);
    lstrcpy(filename, TEXT("*"));

    WIN32_FIND_DATA fd;
    HANDLE hFind = FindFirstFile(full_path, &fd);
    if(hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if(lstrcmp(fd.cFileName, TEXT(".")) != 0 && lstrcmp(fd.cFileName, TEXT("..")) != 0)
            {
                if(recurse && ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0))
                {
                    lstrcpy(filename, fd.cFileName);
                    lstrcat(filename, TEXT("\\"));

                    const bool hidden = (fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0;
                    if(callback->on_pre_folder(full_path, hidden))
                    {
                        process_folder(full_path, callback, recurse);
                        callback->on_post_folder(full_path, hidden);
                    }
                }
                else if((fd.dwFileAttributes & FILE_ATTRIBUTE_NORMAL) != 0)
                {
                    lstrcpy(filename, fd.cFileName);
                    callback->on_file(filename, (fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0);
                }
            }
        } while(FindNextFile(hFind, &fd));

        FindClose(hFind);
    }
}

class File_traverse : public File_traverse_base
{
    bool m_verbose;
    bool m_process_hidden;
    bool m_delete_files;

public:
    File_traverse(bool verbose, bool process_hidden, bool delete_files) :
        m_verbose(verbose),
        m_process_hidden(process_hidden),
        m_delete_files(delete_files)
    {
    }

    virtual bool on_pre_folder(_In_ PCTSTR folder, bool hidden)
    {
        const bool process_folder = !hidden || m_process_hidden;

        if(process_folder)
        {
            if(m_verbose)
            {
                _tprintf(_T("Recursing into: %s\n"), folder);
            }
        }

        return process_folder;
    }

    virtual void on_post_folder(_In_ PCTSTR folder, bool hidden)
    {
        const bool process_folder = !hidden || m_process_hidden;

        if(process_folder)
        {
            if(m_verbose)
            {
                _tprintf(_T("Folder: %s\n"), folder);
            }

            if(m_delete_files)
            {
                RemoveDirectory(folder);
            }
        }
    }

    virtual void on_file(_In_ PCTSTR filename, bool hidden)
    {
        const bool process_folder = !hidden || m_process_hidden;

        if(process_folder)
        {
            if(m_verbose)
            {
                _tprintf(_T("Filename: %s\n"), filename);
            }

            if(m_delete_files)
            {
                DeleteFile(filename);
            }
        }
    }
};

void print_usage()
{
    _tprintf(_T("FLCL - Fast Lightweight Clean\n"));
    _tprintf(_T("Usage: flcl [-r][-h][-s][-d]\n"));
    _tprintf(_T(" -r: Recurse sub-directories\n"));
    _tprintf(_T(" -h: Operate on hidden files/directories\n"));
    _tprintf(_T(" -s: Silent mode\n"));
    _tprintf(_T(" -d: Delete writable files\n"));
}

int _tmain(int argc, _In_count_(argc) PTSTR argv[])
{
    if(argc <= 1)
    {
        print_usage();
    }
    else
    {
        bool recurse = false;
        bool process_hidden = false;
        bool verbose = true;
        bool delete_files = false;

        // TODO: No raw loops?  Transform/map/etc?
        for(auto ii = 1; ii < argc; ++ii)
        {
            if(lstrcmp(argv[ii], TEXT("-r")) == 0)
            {
                recurse = true;
            }
            else if(lstrcmp(argv[ii], TEXT("-h")) == 0)
            {
                process_hidden = true;
            }
            else if(lstrcmp(argv[ii], TEXT("-s")) == 0)
            {
                verbose = false;
            }
            else if(lstrcmp(argv[ii], TEXT("-d")) == 0)
            {
                delete_files = true;
            }
        }

#if 0
        auto op = [](PCTSTR arg1, PCTSTR arg2) { return lstrcmp(arg1, arg2) == 0; };
        recurse |= std::find_if(argv, argv + argc, [](PCTSTR arg) -> bool { return op(TEXT("-r")); }) != argv + argc;
        bool recurse = std::find_if(argv, argv + argc, std::bind2nd(op, TEXT("-r"))) != argv + argc;
#endif

        const DWORD character_count = GetCurrentDirectory(0, nullptr);
        if(character_count > 0)
        {
            Simple_string<TCHAR> folder;
            if(folder.reserve(character_count + 1))
            {
                if(GetCurrentDirectory(character_count, folder.c_str()))
                {
                    folder.concat(_T('\\'));
#ifdef _DEBUG
                    _tprintf(_T("DEBUG: Current dir: %s\n"), folder.c_str());
#endif

                    File_traverse callback(verbose, process_hidden, delete_files);
                    process_folder(folder.c_str(), &callback, recurse);
                }
                else
                {
                    _ftprintf(stderr, _T("Error getting current directory.\n"));
                }
            }
        }

#if 0
        TCHAR folder[MAX_PATH];
        if(GetCurrentDirectory(ARRAYSIZE(folder), folder) != 0)
        {
            PathAddBackslash(folder);
#ifdef _DEBUG
            _tprintf(_T("DEBUG: Current dir: %s\n"), folder);
#endif
        }

        File_traverse callback(silent, process_hidden, delete_files);
        process_folder(folder, &callback, recurse);
#endif
    }

    return 0;
}

