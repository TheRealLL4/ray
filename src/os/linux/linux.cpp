#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef DEVELOPER
void os_debug_vlog(const char *format, va_list args)
{
    vprintf(format, args);
    printf("\n");
}

void os_debug_break()
{
    raise(SIGTRAP);
}
#endif

void *os_allocate(u64 amount)
{
    void *address =
        mmap(
            nullptr,
            amount,
            PROT_READ   | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS,
            -1,
            0
        );

    if (address == MAP_FAILED) {
        return nullptr;
    }

    return address;
}

void *os_reallocate(void *start, u64 old_size, u64 new_size)
{
    void *address =
        mremap(
            start,    // Has to be page-aligned
            old_size,
            new_size,
            MREMAP_MAYMOVE
        );

    if (address == MAP_FAILED) {
        return nullptr;
    }

    return address;
}

void os_free(void *address, u64 amount)
{
    munmap(address, amount);
}

u32 os_page_size()
{
    return sysconf(_SC_PAGE_SIZE);
}

bool os_read_file(File *file)
{
    int fd = open(file->name, O_RDONLY);
    if (fd == -1) {
        // File could not be opened
        debug_log("open(%s): %s", file->name, strerror(errno));

        return false;
    }

    defer {
        close(fd);
    };

    struct stat64 stat;
    if (fstat64(fd, &stat) == -1) {
        // Could not stat
        return false;
    }

    file->size = stat.st_size;
    file->data = (u8 *) os_allocate(file->size);
    if (!file->data) {
        // Allocation failed
        return false;
    }

    if (read(fd, file->data, file->size) != (ssize_t) file->size) {
        // Read failed
        return false;
    }

    return true;
}

bool os_write_file(File *file)
{
    int fd = open(file->name, O_WRONLY);
    if (fd == -1) {
        // File could not be opened
        debug_log("open(%s): %s", file->name, strerror(errno));

        return false;
    }

    defer {
        close(fd);
    };

    if (write(fd, file->data, file->size) != (ssize_t) file->size) {
        // Write failed
        return false;
    }

    return true;
}

