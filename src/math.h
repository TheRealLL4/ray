#pragma once

union Vector3
{
    struct
    {
        f32 x = 0, y = 0, z = 0;
    };

    struct
    {
        f32 r, g, b;
    };
};

static inline Vector3 operator-(Vector3 v)
{
    return { -v.x, -v.y, -v.z };
}

static inline Vector3 operator+(Vector3 u, Vector3 v)
{
    return { u.x + v.x, u.y + v.y, u.z + v.z };
}

static inline Vector3 operator-(Vector3 u, Vector3 v)
{
    return { u.x - v.x, u.y - v.y, u.z - v.z };
}

static inline Vector3 operator*(Vector3 u, Vector3 v)
{
    return { u.x * v.x, u.y * v.y, u.z * v.z };
}

static inline Vector3 operator/(Vector3 u, Vector3 v)
{
    return { u.x / v.x, u.y / v.y, u.z / v.z };
}

static inline Vector3 operator*(f32 a, Vector3 &v)
{
    return { a * v.x, a * v.y, a * v.z };
}

static inline Vector3 operator/(Vector3 &v, f32 a)
{
    return { v.x / a, v.y / a, v.z / a };
}

f32 dot(Vector3 u, Vector3 v)
{
    return u.x * v.x + u.y * v.y + u.z * v.z;
}

Vector3 cross(Vector3 u, Vector3 v)
{
    return {
        u.y * v.z - u.z * v.y,
        u.z * v.x - u.x * v.z,
        u.x * v.y - u.y * v.x,
    };
}

Vector3 min(Vector3 u, Vector3 v)
{
    return {
        MIN(u.x, v.x),
        MIN(u.y, v.y),
        MIN(u.z, v.z),
    };
}

Vector3 max(Vector3 u, Vector3 v)
{
    return {
        MAX(u.x, v.x),
        MAX(u.y, v.y),
        MAX(u.z, v.z),
    };
}

f32 min(Vector3 v)
{
    return MIN(MIN(v.x, v.y), v.z);
}

f32 max(Vector3 v)
{
    return MAX(MAX(v.x, v.y), v.z);
}

struct Vector4
{
    f32 x = 0, y = 0, z = 0, w = 1;
};

using Quaternion = Vector4;

static inline Vector3 vector_part(Quaternion q)
{
    return {q.x, q.y, q.z};
}

static inline Quaternion operator*(Quaternion q, Quaternion r)
{
    Vector3 v1 = vector_part(q);
    Vector3 v2 = vector_part(r);

    Vector3 v = q.w * v2 + r.w * v1 + cross(v1, v2);

    return {
        v.x, v.y, v.z,
        q.w * r.w - dot(v1, v2),
    };
}

Quaternion conj(Quaternion q)
{
    return {-q.x, -q.y, -q.z, q.w};
}

Vector3 rotate(Vector3 v, Quaternion q)
{
    Quaternion u = {v.x, v.y, v.z, 0};
    return vector_part(q * u * conj(q));
}

