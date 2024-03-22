#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

void print_win32_error(const char *function_name)
{
    char *message;
    DWORD error = GetLastError();

    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        error,
        MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
        (LPTSTR) &message,
        0,
        nullptr
    );

    debug_log("%s:\n%s", function_name, message);

    LocalFree(message);
}

#ifdef DEVELOPER
void os_debug_break()
{
    DebugBreak();
}

void os_debug_vlog(const char *format, va_list args)
{
    // HACK:
    char buffer[1024];
    vsnprintf(buffer, array_size(buffer), format, args);
    OutputDebugStringA(buffer);
    OutputDebugStringA("\n");
}
#endif

void *os_allocate(u64 amount)
{
    return
        VirtualAlloc(
            nullptr,
            amount,
            MEM_COMMIT | MEM_RESERVE,
            PAGE_READWRITE
        );
}

void *os_reallocate(void *start, u64 old_size, u64 new_size)
{
    return
        VirtualAlloc(
            start,
            new_size,
            MEM_COMMIT | MEM_RESERVE,
            PAGE_READWRITE
        );
}

void os_free(void *address, u64 amount)
{
    VirtualFree(address, 0, MEM_RELEASE);
}

u32 os_page_size()
{
    SYSTEM_INFO info;
    GetSystemInfo(&info);

    return info.dwPageSize;
}

bool os_read_file(File *file)
{
    HANDLE handle =
        CreateFileA(
            file->name,
            GENERIC_READ,
            FILE_SHARE_READ,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );

    if (handle == INVALID_HANDLE_VALUE) {
        print_win32_error("CreateFile");
        return false;
    }

    defer {
        CloseHandle(handle);
    };

    DWORD size_high;
    DWORD size_low = GetFileSize(handle, &size_high);

    if (size_high != 0) {
        debug_log("Error: File `%s` is too large (> 4 GiB).", file->name);
        return false;
    }

    u32 size = size_low;

    file->size = size;
    file->data = (u8 *) os_allocate(file->size);

    DWORD read;
    if (ReadFile(handle, file->data, file->size, &read, nullptr) == FALSE) {
        print_win32_error("ReadFile");
        return false;
    }

    return true;
}

bool os_write_file(File *file)
{
    HANDLE handle =
        CreateFileA(
            file->name,
            GENERIC_WRITE,
            0,
            nullptr,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );

    if (handle == INVALID_HANDLE_VALUE) {
        print_win32_error("CreateFile");
        return false;
    }

    defer {
        CloseHandle(handle);
    };

    DWORD written;
    if (WriteFile(handle, file->data, file->size, &written, nullptr) != TRUE) {
        print_win32_error("WriteFile");
        return false;
    }

    return true;
}

