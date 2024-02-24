#pragma once

static inline f32 square(f32 x)
{
    return x * x;
}

static inline f32 lerp(f32 a, f32 b, f32 t)
{
    return t * b + (1 - t) * a;
}

union Vector2
{
    struct
    {
        f32 x, y;
    };

    f32 e[2];

    f32 &operator[](u32 index)
    {
        return this->e[index];
    }
};

static inline Vector2 operator-(Vector2 v)
{
    return {-v.x, -v.y};
}

static inline Vector2 operator+(Vector2 u, Vector2 v)
{
    return {u.x + v.x, u.y + v.y};
}

static inline Vector2 operator-(Vector2 u, Vector2 v)
{
    return {u.x - v.x, u.y - v.y};
}

static inline Vector2 operator*(Vector2 u, Vector2 v)
{
    return {u.x * v.x, u.y * v.y};
}

static inline Vector2 operator/(Vector2 u, Vector2 v)
{
    return {u.x / v.x, u.y / v.y};
}

static inline Vector2 operator*(f32 a, Vector2 v)
{
    return {a * v.x, a * v.y};
}

static inline Vector2 operator/(Vector2 v, f32 a)
{
    return {v.x / a, v.y / a};
}

static inline f32 length(Vector2 v)
{
    return sqrtf(v.x * v.x + v.y * v.y);
}

static inline Vector2 normalize(Vector2 v)
{
    return v / length(v);
}

static inline f32 dot(Vector2 u, Vector2 v)
{
    return u.x * v.x + u.y * v.y;
}

static inline f32 skew(Vector2 u, Vector2 v)
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

    f32 &operator[](u32 index)
    {
        return this->e[index];
    }
};

static inline Vector3 operator-(Vector3 v)
{
    return {-v.x, -v.y, -v.z};
}

static inline Vector3 operator+(Vector3 u, Vector3 v)
{
    return {u.x + v.x, u.y + v.y, u.z + v.z};
}

static inline Vector3 &operator+=(Vector3 &u, Vector3 v)
{
    u = u + v;

    return u;
}

static inline Vector3 operator-(Vector3 u, Vector3 v)
{
    return {u.x - v.x, u.y - v.y, u.z - v.z};
}

static inline Vector3 &operator-=(Vector3 &u, Vector3 v)
{
    u = u - v;

    return u;
}

static inline Vector3 operator*(Vector3 u, Vector3 v)
{
    return {u.x * v.x, u.y * v.y, u.z * v.z};
}

static inline Vector3 &operator*=(Vector3 &u, Vector3 v)
{
    u = u * v;

    return u;
}

static inline Vector3 operator/(Vector3 u, Vector3 v)
{
    return {u.x / v.x, u.y / v.y, u.z / v.z};
}

static inline Vector3 &operator/=(Vector3 &u, Vector3 v)
{
    u = u + v;

    return u;
}

static inline Vector3 operator*(f32 a, Vector3 v)
{
    return {a * v.x, a * v.y, a * v.z};
}

static inline Vector3 &operator*=(f32 a, Vector3 &v)
{
    v = a * v;

    return v;
}

static inline Vector3 operator/(Vector3 v, f32 a)
{
    return {v.x / a, v.y / a, v.z / a};
}

static inline Vector3 &operator/=(Vector3 &v, f32 a)
{
    v = v / a;

    return v;
}

static inline f32 length_sq(Vector3 v)
{
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

static inline f32 length(Vector3 v)
{
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

static inline Vector3 normalize(Vector3 v)
{
    return v / length(v);
}

static inline f32 dot(Vector3 u, Vector3 v)
{
    return u.x * v.x + u.y * v.y + u.z * v.z;
}

static inline Vector3 cross(Vector3 u, Vector3 v)
{
    return {
        u.y * v.z - u.z * v.y,
        u.z * v.x - u.x * v.z,
        u.x * v.y - u.y * v.x,
    };
}

static inline Vector3 min(Vector3 u, Vector3 v)
{
    return {
        MIN(u.x, v.x),
        MIN(u.y, v.y),
        MIN(u.z, v.z),
    };
}

static inline Vector3 max(Vector3 u, Vector3 v)
{
    return {
        MAX(u.x, v.x),
        MAX(u.y, v.y),
        MAX(u.z, v.z),
    };
}

static inline Vector3 lerp(Vector3 a, Vector3 b, f32 t)
{
    return t * b + (1 - t) * a;
}

static inline f32 min(Vector3 v)
{
    return MIN(MIN(v.x, v.y), v.z);
}

static inline f32 max(Vector3 v)
{
    return MAX(MAX(v.x, v.y), v.z);
}

static inline Vector3 pow(Vector3 v, f32 a)
{
    return {powf(v.x, a), powf(v.y, a), powf(v.z, a)};
}

static inline Vector3 pow(Vector3 v, Vector3 e)
{
    return {powf(v.x, e.x), powf(v.y, e.y), powf(v.z, e.z)};
}

static inline Vector3 clamp(Vector3 v, Vector3 min, Vector3 max)
{
    return {
        CLAMP(v.x, min.x, max.x),
        CLAMP(v.y, min.y, max.y),
        CLAMP(v.z, min.z, max.z),
    };
}

static inline Vector3 clamp(Vector3 v, f32 min, f32 max)
{
    return {
        CLAMP(v.x, min, max),
        CLAMP(v.y, min, max),
        CLAMP(v.z, min, max),
    };
}

static inline Vector3 reflect(Vector3 u, Vector3 v)
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

static inline Quaternion operator*(Quaternion q, Quaternion r)
{
    return {q.w * r.v + r.w * q.v + cross(q.v, r.v), q.w * r.w - dot(q.v, r.v)};
}

static inline Quaternion make_rotation(Vector3 v, f32 radians)
{
    return {sinf(radians / 2) * v, cosf(radians / 2)};
}

static inline Quaternion conj(Quaternion q)
{
    return {-q.x, -q.y, -q.z, q.w};
}

static inline Vector3 rotate(Vector3 v, Quaternion q)
{
    Quaternion u = {v, 0};
    return (q * u * conj(q)).v;
}

