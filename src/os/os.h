#pragma once

#ifdef DEVELOPER
void os_debug_vlog(const char *format, va_list args);
void os_debug_break();
#endif

void *os_allocate(u64 amount);
void *os_reallocate(void *start, u64 old_size, u64 new_size);
void os_free(void *address, u64 amount);
u32 os_page_size();

struct File
{
    char *name; // This is one of the rare cases when we use null-terminated strings for convenience
    u8   *data;
    u32  size;
};

// File name has to be filled in
bool os_read_file(File *file);
bool os_write_file(File *file);

