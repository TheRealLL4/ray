#pragma once

// Basic types:
typedef int8_t   s8;
typedef uint8_t  u8;
typedef int16_t  s16;
typedef uint16_t u16;
typedef int32_t  s32;
typedef uint32_t u32;
typedef int64_t  s64;
typedef uint64_t u64;

typedef float  f32;
typedef double f64;

// Limits:
const s8 S8_MIN = 0x80;
const s8 S8_MAX = 0x7F;
const u8 U8_MAX = 0xFF;

const s16 S16_MIN = 0x8000;
const s16 S16_MAX = 0x7FFF;
const u16 U16_MAX = 0xFFFF;

const s32 S32_MIN = 0x80000000;
const s32 S32_MAX = 0x7FFFFFFF;
const u32 U32_MAX = 0xFFFFFFFFu;

const s64 S64_MIN = 0x8000000000000000ll;
const s64 S64_MAX = 0x7FFFFFFFFFFFFFFFll;
const u64 U64_MAX = 0xFFFFFFFFFFFFFFFFull;

#include "os/os.h"

// Size of static array
template <typename T, u64 N>
inline u64 array_size(T (&array)[N])
{
    return N;
}

#define ALIGN_POW2(x, a) (((x) + (a) - 1) & ~((a) - 1))
#define DIV_UP(a, b) (((a) + (b) - 1) / (b))

// Combine two 32-bit ints into a 64-bit int
#define MAKE_U64(l, h) (((u64(h)) << 32) + (l))

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) < (b) ? (b) : (a))
#define CLAMP(v, l, h) (MAX(MIN(v, h), l))

// Logging:
inline void debug_log(const char *format, ...)
{
#ifdef DEVELOPER
    va_list args;
    va_start(args, format);

    os_debug_vlog(format, args);

    va_end(args);
#endif
}

// ASSERT(condition);
// ASSERT2(condition, message);
#ifdef DEVELOPER
#define ASSERT(condition)                                                                \
    do {                                                                                 \
        if (!(condition)) {                                                              \
            debug_log("%s(%d): Assertion `%s` failed.", __FILE__, __LINE__, #condition); \
            os_debug_break();                                                            \
        }                                                                                \
    } while (false)

#define ASSERT2(condition, message)                           \
    do {                                                      \
        if (!(condition)) {                                   \
            debug_log("%s(%d): Assertion `%s` failed: %s.",   \
                    __FILE__, __LINE__, #condition, message); \
            os_debug_break();                                 \
        }                                                     \
    } while (false)
#else
#define ASSERT(condition) (void) 0
#define ASSERT2(condition, message) (void) 0
#endif

// defer {expression;};
template <typename F>
struct Defer_Struct
{
    F f;

    Defer_Struct(F to_defer) : f(to_defer) {}

    ~Defer_Struct()
    {
        this->f();
    }
};

struct Defer_Initializer {};

template <typename F>
Defer_Struct<F> operator+(Defer_Initializer x, F to_defer)
{
    return Defer_Struct<F>(to_defer);
}

#define DEFER_JOIN_STRING(x, y) x##y
#define DEFER_NAME_1(x, y)      DEFER_JOIN_STRING(x, y)
#define DEFER_NAME_2(x)         DEFER_NAME_1(x, __COUNTER__)
#define defer                   auto DEFER_NAME_2(_defer_) = Defer_Initializer() + [&] ()

// Dynamic array:
const u32 ARRAY_MINIMAL_CAPACITY = 4;
template <typename T>
struct Array
{
    T   *data    = nullptr;
    u32 capacity = 0;
    u32 size     = 0;

    inline T &operator[](u32 index)
    {
        ASSERT2(index < this->size, "Array index out of bounds.");
        return this->data[index];
    }
};

#define ARRAY_ITERATE(array) for (auto it = (array).data; it != ((array).data + (array).size); it++)

template <typename T>
inline void array_maybe_expand(Array<T> *array, u32 to_add)
{
    ASSERT(array->size <= U32_MAX - to_add);

    u32 minimal_capacity = array->size + to_add;
    if (minimal_capacity <= array->capacity) {
        return;
    }

    if (minimal_capacity < 2 * array->capacity) {
        minimal_capacity = 2 * array->capacity;
    } else if (minimal_capacity < ARRAY_MINIMAL_CAPACITY) {
        minimal_capacity = ARRAY_MINIMAL_CAPACITY;
    }

    if (array->data) {
        array->data = (T *) os_reallocate(array->data, array->capacity, minimal_capacity * sizeof(T));
    } else {
        array->data = (T *) os_allocate(minimal_capacity * sizeof(T));
    }
    ASSERT(array->data);

    // os_allocate allocates pages, so re-calculate the capacity
    u32 page_size = os_page_size(); // @SPEED: We shouldn't be doing this for every allocation
    array->capacity = ALIGN_POW2(minimal_capacity * sizeof(T), page_size) / sizeof(T);
}

template <typename T>
inline T array_pop(Array<T> *array)
{
    ASSERT2(array->size > 0, "Attempted to pop an element from an empty array.");
    return (*array)[--array->size];
}

template <typename T>
inline void array_push(Array<T> *array, T value)
{
    array_maybe_expand(array, 1);
    array->data[array->size++] = value;
}

template <typename T>
inline void array_resize(Array<T> *array, u32 new_size)
{
    if (new_size > array->size) {
        array_maybe_expand(array, new_size - array->size);
    }

    array->size = new_size;
}

template <typename T>
inline void array_insert_segment(Array<T> *array, u32 where, u32 how_many)
{
    array_maybe_expand(array, how_many);
    memmove(array->data + where + how_many, array->data + where, (array->size - where) * sizeof(T));
    array->size += how_many;
}

template <typename T>
inline void array_insert(Array<T> *array, T value, u32 where)
{
    array_insert_segment(array, where, 1);
    array->data[where] = value;
}

template <typename T>
inline void array_delete_segment(Array<T> *array, u32 where, u32 how_many)
{
    memmove(array->data + where, array->data + where + how_many, (array->size - where) * sizeof(T));
    array->size -= how_many;
}

template <typename T>
inline T array_delete(Array<T> *array, u32 index)
{
    T value = array->data[index];
    array_delete_segment(array, index, 1);

    return value;
}

template <typename T>
inline void array_free(Array<T> *array)
{
    os_free(array->data, array->capacity);
    *array = {};
}

