#pragma once

#include "basic.h"

#define ABS(x) ((x) < 0 ? -(x) : (x))
#define SQUARE(x) ((x) * (x))

const f32 PI = 3.14159265f;

union Vector2
{
    struct
    {
        f32 x, y;
    };

    f32 e[2];

    inline f32 &operator[](u32 index)
    {
        return this->e[index];
    }
};

inline bool operator==(Vector2 u, Vector2 v)
{
    return u.x == v.x && u.y == v.y;
}

inline bool operator!=(Vector2 u, Vector2 v)
{
    return u.x != v.x || u.y != v.y;
}

inline Vector2 operator-(Vector2 v)
{
    return {-v.x, -v.y};
}

inline Vector2 operator+(Vector2 u, Vector2 v)
{
    return {u.x + v.x, u.y + v.y};
}

inline Vector2 operator-(Vector2 u, Vector2 v)
{
    return {u.x - v.x, u.y - v.y};
}

inline Vector2 operator*(Vector2 u, Vector2 v)
{
    return {u.x * v.x, u.y * v.y};
}

inline Vector2 operator/(Vector2 u, Vector2 v)
{
    return {u.x / v.x, u.y / v.y};
}

inline Vector2 operator*(f32 a, Vector2 v)
{
    return {a * v.x, a * v.y};
}

inline Vector2 operator*(Vector2 v, f32 a)
{
    return {a * v.x, a * v.y};
}

inline Vector2 operator/(Vector2 v, f32 a)
{
    return {v.x / a, v.y / a};
}

inline f32 length(Vector2 v)
{
    return sqrtf(v.x * v.x + v.y * v.y);
}

inline Vector2 normalize(Vector2 v)
{
    return v / sqrtf(v.x * v.x + v.y * v.y);
}

inline f32 dot(Vector2 u, Vector2 v)
{
    return u.x * v.x + u.y * v.y;
}

inline f32 skew(Vector2 u, Vector2 v)
{
    return u.x * v.y - u.y * v.x;
}

union Vector3
{
    struct
    {
        f32 x, y, z;
    };

    struct
    {
        f32 r, g, b;
    };

    f32 e[3];

    inline f32 &operator[](u32 index)
    {
        return this->e[index];
    }
};

inline bool operator==(Vector3 u, Vector3 v)
{
    return u.x == v.x && u.y == v.y && u.z == v.z;
}

inline bool operator!=(Vector3 u, Vector3 v)
{
    return u.x != v.x || u.y != v.y || u.z != v.z;
}

inline Vector3 operator-(Vector3 v)
{
    return {-v.x, -v.y, -v.z};
}

inline Vector3 operator+(Vector3 u, Vector3 v)
{
    return {u.x + v.x, u.y + v.y, u.z + v.z};
}

inline Vector3 &operator+=(Vector3 &u, Vector3 v)
{
    u = u + v;

    return u;
}

inline Vector3 operator-(Vector3 u, Vector3 v)
{
    return {u.x - v.x, u.y - v.y, u.z - v.z};
}

inline Vector3 &operator-=(Vector3 &u, Vector3 v)
{
    u = u - v;

    return u;
}

inline Vector3 operator*(Vector3 u, Vector3 v)
{
    return {u.x * v.x, u.y * v.y, u.z * v.z};
}

inline Vector3 &operator*=(Vector3 &u, Vector3 v)
{
    u = u * v;

    return u;
}

inline Vector3 operator/(Vector3 u, Vector3 v)
{
    return {u.x / v.x, u.y / v.y, u.z / v.z};
}

inline Vector3 &operator/=(Vector3 &u, Vector3 v)
{
    u = u / v;

    return u;
}

inline Vector3 operator*(f32 a, Vector3 v)
{
    return {a * v.x, a * v.y, a * v.z};
}

inline Vector3 operator*(Vector3 v, f32 a)
{
    return {a * v.x, a * v.y, a * v.z};
}

inline Vector3 &operator*=(Vector3 &v, f32 a)
{
    v = a * v;

    return v;
}

inline Vector3 operator/(Vector3 v, f32 a)
{
    return {v.x / a, v.y / a, v.z / a};
}

inline Vector3 &operator/=(Vector3 &v, f32 a)
{
    v = v / a;

    return v;
}

inline f32 length_sq(Vector3 v)
{
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

inline f32 length(Vector3 v)
{
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

inline Vector3 normalize(Vector3 v)
{
    return v / sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

inline f32 dot(Vector3 u, Vector3 v)
{
    return u.x * v.x + u.y * v.y + u.z * v.z;
}

inline Vector3 cross(Vector3 u, Vector3 v)
{
    return {
        u.y * v.z - u.z * v.y,
        u.z * v.x - u.x * v.z,
        u.x * v.y - u.y * v.x,
    };
}

inline Vector3 min(Vector3 u, Vector3 v)
{
    return {
        MIN(u.x, v.x),
        MIN(u.y, v.y),
        MIN(u.z, v.z),
    };
}

inline Vector3 max(Vector3 u, Vector3 v)
{
    return {
        MAX(u.x, v.x),
        MAX(u.y, v.y),
        MAX(u.z, v.z),
    };
}

inline Vector3 lerp(Vector3 a, Vector3 b, f32 t)
{
    return t * b + (1 - t) * a;
}

inline f32 min(Vector3 v)
{
    return MIN(MIN(v.x, v.y), v.z);
}

inline f32 max(Vector3 v)
{
    return MAX(MAX(v.x, v.y), v.z);
}

inline Vector3 pow(Vector3 v, f32 a)
{
    return {powf(v.x, a), powf(v.y, a), powf(v.z, a)};
}

inline Vector3 pow(Vector3 v, Vector3 e)
{
    return {powf(v.x, e.x), powf(v.y, e.y), powf(v.z, e.z)};
}

inline Vector3 clamp(Vector3 v, Vector3 min, Vector3 max)
{
    return {
        CLAMP(v.x, min.x, max.x),
        CLAMP(v.y, min.y, max.y),
        CLAMP(v.z, min.z, max.z),
    };
}

inline Vector3 clamp(Vector3 v, f32 min, f32 max)
{
    return {
        CLAMP(v.x, min, max),
        CLAMP(v.y, min, max),
        CLAMP(v.z, min, max),
    };
}

inline Vector3 reflect(Vector3 u, Vector3 v)
{
    return 2.0f * dot(u, v) * v - u;
}

union Quaternion
{
    struct
    {
        Vector3 v;
        f32     s;
    };

    struct
    {
        f32 x, y, z, w;
    };
};

inline Quaternion operator*(Quaternion q, Quaternion r)
{
    return {q.w * r.v + r.w * q.v + cross(q.v, r.v), q.w * r.w - dot(q.v, r.v)};
}

inline Quaternion make_rotation(Vector3 v, f32 radians)
{
    return {sinf(radians / 2.0f) * v, cosf(radians / 2.0f)};
}

inline Quaternion conj(Quaternion q)
{
    return {-q.x, -q.y, -q.z, q.w};
}

inline Vector3 rotate(Vector3 v, Quaternion q)
{
    Vector3 t = 2.0f * cross(q.v, v);

    return v + q.w * t + cross(q.v, t);
}

