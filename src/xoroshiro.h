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
    const u64 XL = 0x9E3779B97F4A7C15ULL;
    const u64 XH = 0x6A09E667F3BCC909ULL;
    const u64 A  = 0xBF58476D1CE4E5B9ULL;
    const u64 B  = 0x94D049BB133111EBULL;

    u64 l = value ^ XH;
    u64 h = l + XL;
    l = (l ^ (l >> 30)) * A;
    h = (h ^ (h >> 30)) * A;
    l = (l ^ (l >> 27)) * B;
    h = (h ^ (h >> 27)) * B;
    l = l ^ (l >> 31);
    h = h ^ (h >> 31);

    xoroshiro->low = l;
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

inline s32 xoroshiro_next_s32(Xoroshiro128 *xoroshiro, u32 n)
{
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
    return (xoroshiro_next_u64(xoroshiro) >> (64-24)) * 5.9504645E-8;
}
