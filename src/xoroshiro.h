#pragma once

inline u64 rotl64(u64 x, u8 b)
{
    return (x << b) | (x >> (64-b));
}

struct Xoroshiro128
{
    u64 low, high;
};

inline void xoroshiro_set_seed(Xoroshiro128 *xoroshiro, u64 value)
{
    const u64 xl = 0x9E3779B97F4A7C15ull;
    const u64 xh = 0x6A09E667F3BCC909ull;
    const u64 a  = 0xBF58476D1CE4E5B9ull;
    const u64 b  = 0x94D049BB133111EBull;

    u64 l = value ^ xh;
    u64 h = l + xl;
    l = (l ^ (l >> 30)) * a;
    h = (h ^ (h >> 30)) * a;
    l = (l ^ (l >> 27)) * b;
    h = (h ^ (h >> 27)) * b;
    l = l ^ (l >> 31);
    h = h ^ (h >> 31);

    xoroshiro->low  = l;
    xoroshiro->high = h;
}

inline u64 xoroshiro_next_u64(Xoroshiro128 *xoroshiro)
{
    u64 l = xoroshiro->low;
    u64 h = xoroshiro->high;
    u64 n = rotl64(l + h, 17) + l;

    h ^= l;
    xoroshiro->low  = rotl64(l, 49) ^ h ^ (h << 21);
    xoroshiro->high = rotl64(h, 28);

    return n;
}

// Generates random numbers in [0, n]
inline u32 xoroshiro_next_u32(Xoroshiro128 *xoroshiro, u32 n)
{
    n++;
    u64 r = (xoroshiro_next_u64(xoroshiro) & 0xFFFFFFFF) * n;
    if (((u32) r) < n) {
        while (((u32) r) < (~n + 1) % n) {
            r = (xoroshiro_next_u64(xoroshiro) & 0xFFFFFFFF) * n;
        }
    }

    return r >> 32;
}

inline f32 xoroshiro_next_f32(Xoroshiro128 *xoroshiro)
{
    return (xoroshiro_next_u64(xoroshiro) >> (64-24)) * 5.9604644775390625e-8f;
}

