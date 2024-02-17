#pragma once

struct Vector3
{
    f32 x = 0, y = 0, z = 0;
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

struct Vector4
{
    f32 x = 0, y = 0, z = 0, w = 1;
};

