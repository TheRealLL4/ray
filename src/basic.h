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

// Size of static array
template <typename T, u64 N>
static inline u64 array_size(T (&array)[N])
{
    return N;
}

// Integer aligment
#define INTEGER_ALIGN(x, a) (((x) + (a) - 1) / (a) * (a))

// Combine two 32-bit ints into a 64-bit int
#define MAKE_U64(l, h) (((u64(h)) << 32ULL) + (l))

// min, max, clamp(value, min, max)
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) < (b) ? (b) : (a))
#define CLAMP(v, l, h) (MAX(MIN(v, h), l))

// Swap two objects
#define SWAP(a, b) do { auto tmp = b; b = a; a = tmp; } while(false)

// Logging:
static inline void log(const char *format, ...)
{
#ifdef DEVELOPER
    va_list args;
    va_start(args, format);

//    vlog(format, args);
    vprintf(format, args);
    printf("\n");

    va_end(args);
#endif
}

// assert2(condition, message);
#ifdef DEVELOPER
#define assert2(condition, message)                                               \
    do {                                                                          \
        if (!(condition)) {                                                       \
            log("In %s, defined in %s on line %d:\n"                              \
                  "Assertion `%s` failed.\n"                                      \
                  "%s\n", __FUNCTION__, __FILE__, __LINE__, #condition, message); \
            abort();                                                              \
        }                                                                         \
    } while (false)
#else
#define assert2(condition, message) (void) 0
#endif

// defer { expression; };
template <typename F>
struct Defer_Struct
{
    F f;
    Defer_Struct(F f) : f(f) {}
    ~Defer_Struct() { f(); }
};

enum class Defer_Initializer {};

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
template <typename T>
struct Array
{
    static const u32 MINIMAL_CAPACITY = 4U;

    T   *data    = nullptr;
    u32 capacity = 0;
    u32 size     = 0;

    T &operator[](u32 index)
    {
        assert2(index < this->size, "Array index out of bounds.");
        return this->data[index];
    }
};

#define FOR_EACH(array) for (auto it = (array).data; it != ((array).data + (array).size); it++)

template <typename T>
static inline void array_maybe_expand(Array<T> *array, u32 to_add) 
{
    u32 minimal_capacity = array->size + to_add;
    if (minimal_capacity <= array->capacity) {
        return;
    }

    if (minimal_capacity < 2 * array->capacity) {
        minimal_capacity = 2 * array->capacity;
    } else if (minimal_capacity < Array<T>::MINIMAL_CAPACITY) {
        minimal_capacity = Array<T>::MINIMAL_CAPACITY;
    }

    array->data = (T *) realloc(array->data, minimal_capacity * sizeof(T));
    assert2(array->data, "realloc() failed.");

    array->capacity = minimal_capacity;
}

template <typename T>
static inline T array_pop(Array<T> *array)
{
    assert2(array->size > 0, "Attempted to pop an element from an empty array.");
    return (*array)[--array->size];
}

template <typename T>
static inline void array_push(Array<T> *array, T value)
{
    array_maybe_expand(array, 1);
    array->data[array->size++] = value;
}

template <typename T>
static inline void array_reserve(Array<T> *array, u32 count)
{
    array_maybe_expand(array, count); 
}

template <typename T>
static inline void array_resize(Array<T> *array, u32 new_size)
{
    if (new_size > array->size) {
        array_maybe_expand(array, new_size - array->size);
    }

    array->size = new_size;
}

template <typename T>
static inline void array_insert_segment(Array<T> *array, u32 where, u32 how_many)
{
    array_maybe_expand(array, how_many);
    memmove(array->data + where + how_many, array->data + where, (array->size - where) * sizeof(T));
    array->size += how_many;
}

template <typename T>
static inline void array_insert(Array<T> *array, T value, u32 where)
{
    array_insert_segment(array, where, 1);
    array->data[where] = value;
}

template <typename T>
static inline void array_delete_segment(Array<T> *array, u32 where, u32 how_many)
{
    memmove(array->data + where, array->data + where + how_many, (array->size - where) * sizeof(T));
    array->size -= how_many;
}

template <typename T>
static inline T array_delete(Array<T> *array, u32 index)
{
    T value = array->data[index];
    array_delete_segment(array, index, 1);

    return value;
}

template <typename T>
static inline bool array_is_empty(Array<T> *array)
{
    return (array->size == 0);
}

template <typename T>
static inline void array_free(Array<T> *array)
{
    free(array->data);
    *array = {};
}

