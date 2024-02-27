#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define PRIVATE_NAMESPACE_NAME  ray
#define PRIVATE_NAMESPACE_BEGIN namespace PRIVATE_NAMESPACE_NAME { namespace {
#define PRIVATE_NAMESPACE_END   } }

PRIVATE_NAMESPACE_BEGIN

#include "basic.h"
#include "math.h"
#include "xoroshiro.h"

enum Primitive_Type
{
    PRIMITIVE_PLANE     = 0,
    PRIMITIVE_ELLIPSOID = 1,
    PRIMITIVE_BOX       = 2,
};

enum Surface_Type
{
    SURFACE_DIFFUSE    = 0,
    SURFACE_METALLIC   = 1,
    SURFACE_DIELECTRIC = 2,
};

struct Primitive
{
    Primitive_Type type;
    Surface_Type surface_type = SURFACE_DIFFUSE;
    f32 ior;
    Vector3 parameters;

    Vector3    position = {0, 0, 0};
    Quaternion rotation = {0, 0, 0, 1};
    Vector3    color;
    Vector3    emission = {0, 0, 0};
};

struct Scene
{
    u32 width, height;

    Vector3 background_color;

    struct
    {
        Vector3 position;
        Vector3 right;
        Vector3 up;
        Vector3 forward;
        f32     fov_x_radians;
    } camera;

    Array<Primitive> primitives;

    u32 ray_depth;
    u32 samples;

    Xoroshiro128 xoroshiro;
};

struct Ray
{
    Vector3 origin, direction;
};

struct Intersection
{
    f32 t;
    Vector3 normal;
    bool inner;
};

struct Parser
{
    char *buffer;
    u32 length;
    u32 cursor;
};

inline char *current(Parser *parser)
{
    return (parser->buffer + parser->cursor);
}

void skip_to_next_line(Parser *parser)
{
    while (parser->cursor < parser->length && parser->buffer[parser->cursor] != '\n') {
        parser->cursor += 1;
    }

    if (parser->cursor < parser->length) {
        parser->cursor += 1;
    }
}

bool advance_if_starts_with(Parser *parser, const char *c_string)
{
    u32 c = 0;
    while (c_string[c]) {
        if (((parser->cursor + c) >= parser->length) || (c_string[c] != current(parser)[c])) {
            return false;
        }
        c++;
    }

    parser->cursor += c;

    return true;
}

#define SCAN_VECTOR3(parser, v) sscanf(current(parser), "%f %f %f", &(v).x, &(v).y, &(v).z)
Primitive parse_primitive(Parser *parser)
{
    Primitive primitive;
    while (parser->cursor < parser->length) {
        if (advance_if_starts_with(parser, "PLANE ")) {
            primitive.type = PRIMITIVE_PLANE;
            SCAN_VECTOR3(parser, primitive.parameters);
        } else if (advance_if_starts_with(parser, "ELLIPSOID ")) {
            primitive.type = PRIMITIVE_ELLIPSOID;
            SCAN_VECTOR3(parser, primitive.parameters);
        } else if (advance_if_starts_with(parser, "BOX ")) {
            primitive.type = PRIMITIVE_BOX;
            SCAN_VECTOR3(parser, primitive.parameters);
        } else if (advance_if_starts_with(parser, "POSITION ")) {
            SCAN_VECTOR3(parser, primitive.position);
        } else if (advance_if_starts_with(parser, "ROTATION ")) {
            sscanf(current(parser), "%f %f %f %f",
                &primitive.rotation.x,
                &primitive.rotation.y,
                &primitive.rotation.z,
                &primitive.rotation.w
            );
        } else if (advance_if_starts_with(parser, "COLOR ")) {
            SCAN_VECTOR3(parser, primitive.color);
        } else if (advance_if_starts_with(parser, "METALLIC")) {
            primitive.surface_type = SURFACE_METALLIC;
        } else if (advance_if_starts_with(parser, "DIELECTRIC")) {
            primitive.surface_type = SURFACE_DIELECTRIC;
        } else if (advance_if_starts_with(parser, "IOR ")) {
            sscanf(current(parser), "%f", &primitive.ior);
        } else if (advance_if_starts_with(parser, "EMISSION ")) {
            SCAN_VECTOR3(parser, primitive.emission);
        } else {
            break;
        }

        skip_to_next_line(parser);
    }

    return primitive;
}

void parse(Parser *parser, Scene *scene)
{
    while (parser->cursor < parser->length) {
        if (advance_if_starts_with(parser, "DIMENSIONS ")) {
            sscanf(current(parser), "%d %d", &scene->width, &scene->height);
        } else if (advance_if_starts_with(parser, "BG_COLOR ")) {
            SCAN_VECTOR3(parser, scene->background_color);
        } else if (advance_if_starts_with(parser, "CAMERA_POSITION ")) {
            SCAN_VECTOR3(parser, scene->camera.position);
        } else if (advance_if_starts_with(parser, "CAMERA_RIGHT ")) {
            SCAN_VECTOR3(parser, scene->camera.right);
        } else if (advance_if_starts_with(parser, "CAMERA_UP ")) {
            SCAN_VECTOR3(parser, scene->camera.up);
        } else if (advance_if_starts_with(parser, "CAMERA_FORWARD ")) {
            SCAN_VECTOR3(parser, scene->camera.forward);
        } else if (advance_if_starts_with(parser, "CAMERA_FOV_X ")) {
            sscanf(current(parser), "%f", &scene->camera.fov_x_radians);
        } else if (advance_if_starts_with(parser, "NEW_PRIMITIVE\n")) {
            array_push(&scene->primitives, parse_primitive(parser));
            continue;
        } else if (advance_if_starts_with(parser, "RAY_DEPTH ")) {
            sscanf(current(parser), "%u", &scene->ray_depth);
        } else if (advance_if_starts_with(parser, "SAMPLES ")) {
            sscanf(current(parser), "%u", &scene->samples);
        }

        skip_to_next_line(parser);
    }
}

void write_ppm(const char *file_name, u32 width, u32 height, u8 *pixels)
{
    FILE *file = fopen(file_name, "wb");
    if (!file) {
        log("Could not open file `%s` for writing.", file_name);
        return;
    }

    fprintf(file, "%s\n", "P6");
    fprintf(file, "%d %d\n", width, height);
    fprintf(file, "%d\n", 255);

    fwrite(pixels, 1, 3 * width * height, file);

    fclose(file);
}

#ifdef _WIN32
#pragma pack(push, 1)
struct BMP_Header
{
    u16 file_type;
    u32 file_size;
    u16 reserved1;
    u16 reserved2;
    u32 bitmap_offset;
    u32 size;
    u32 width;
    u32 height;
    u16 color_planes;
    u16 bits_per_pixel;
    u32 compression;
    u32 size_of_bitmap;
    u32 horizontal_resolution;
    u32 vertical_resolution;
    u32 colors_used;
    u32 colors_important;
};
#pragma pack(pop)

void write_bmp(const char *file_name, u32 width, u32 height, u8 *pixels)
{
    FILE *file = fopen(file_name, "wb");
    if (!file) {
        log("Could not open file `%s` for writing.", file_name);
        return;
    }

    // HACK:
    u32 bmp_pixels_size = 4 * width * height;
    u8 *bmp_pixels = (u8 *) malloc(bmp_pixels_size);

    // Write pixels as BGR and flip them upside down
    for (u32 y = 0; y < height; y++) {
        for (u32 x = 0; x < width; x++) {
            u32 bmp_pos = 4 * (x + y * width);
            u32 pos = 3 * (x + (height - 1 - y) * width);
            bmp_pixels[bmp_pos + 0] = pixels[pos + 2];
            bmp_pixels[bmp_pos + 1] = pixels[pos + 1];
            bmp_pixels[bmp_pos + 2] = pixels[pos + 0];
            bmp_pixels[bmp_pos + 3] = 255;
        }
    }

    BMP_Header bmp_header = {
        .file_type = 0x4D42,
        .file_size = (u32) (sizeof(BMP_Header) + bmp_pixels_size),
        .bitmap_offset = sizeof(BMP_Header),
        .size = sizeof(BMP_Header) - 14,
        .width = width,
        .height = height,
        .color_planes = 1,
        .bits_per_pixel = 32,
        .size_of_bitmap = bmp_pixels_size,
    };

    fwrite(&bmp_header, 1, sizeof(bmp_header), file);
    fwrite(bmp_pixels,  1, bmp_pixels_size,    file);

    fclose(file);

    free(bmp_pixels);
}
#endif

Intersection intersect_plane(Primitive *plane, Ray ray)
{
    Vector3 object_normal = plane->parameters;

    Intersection intersection = {
        .t = -dot(ray.origin, object_normal) / dot(ray.direction, object_normal),
        .normal = normalize(rotate(object_normal, plane->rotation)),
    };

    if (dot(object_normal, ray.direction) > 0) {
        intersection.normal = -intersection.normal;
        intersection.inner = true;
    }

    return intersection;
}

Intersection intersect_ellipsoid(Primitive *ellipsoid, Ray ray)
{
    Vector3 semi_axes = ellipsoid->parameters;

    f32 a = dot(ray.direction / semi_axes, ray.direction / semi_axes);
    f32 b = 2.0f * dot(ray.origin / semi_axes, ray.direction / semi_axes);
    f32 c = dot(ray.origin / semi_axes, ray.origin / semi_axes) - 1.0f;

    f32 discriminant = b * b - 4.0f * a * c;
    if (discriminant < 0) {
        return {.t = -1};
    }

    f32 t_min = (-b - sqrtf(discriminant)) / (2 * a);
    f32 t_max = (-b + sqrtf(discriminant)) / (2 * a);

    f32 t = -1;
    if (t_min > 0) {
        t = t_min;
    } else if (t_max > 0) {
        t = t_max;
    } else {
        return {.t = -1};
    }

    Vector3 object_normal = (ray.origin + t * ray.direction) / semi_axes / semi_axes;

    Intersection intersection = {
        .t = t,
        .normal = normalize(rotate(object_normal, ellipsoid->rotation)),
    };

    if (dot(object_normal, ray.direction) > 0) {
        intersection.normal = -intersection.normal;
        intersection.inner = true;
    }

    return intersection;
}

Intersection intersect_box(Primitive *box, Ray ray)
{
    Vector3 dimensions = box->parameters;

    Vector3 t1 = (-dimensions - ray.origin) / ray.direction;
    Vector3 t2 = ( dimensions - ray.origin) / ray.direction;

    Vector3 t_min = min(t1, t2);
    Vector3 t_max = max(t1, t2);

    f32 interval_min = max(t_min);
    f32 interval_max = min(t_max);

    if (interval_min > interval_max) {
        return {.t = -1};
    }

    f32 t = -1.0f;
    if (interval_min > 0) {
        t = interval_min;
    } else if (interval_max > 0) {
        t = interval_max;
    } else {
        return {.t = -1};
    }

    Vector3 object_normal = (ray.origin + t * ray.direction) / dimensions;
    u32 max_index = 0;
    for (u32 i = 1; i < 3; i++) {
        if (fabsf(object_normal[i]) > fabsf(object_normal[max_index])) {
            max_index = i;
        }
    }

    object_normal[(max_index + 1) % 3] = 0.0f;
    object_normal[(max_index + 2) % 3] = 0.0f;

    Intersection intersection = {
        .t = t,
        .normal = normalize(rotate(object_normal, box->rotation)),
    };

    if (dot(ray.direction, object_normal) > 0) {
        intersection.normal = -intersection.normal;
        intersection.inner = true;
    }

    return intersection;
}

Intersection intersect(Array<Primitive> primitives, Ray world_ray, Primitive **closest, f32 t_max = INFINITY)
{
    Intersection out = {.t = INFINITY};
    *closest = nullptr;

    FOR_EACH(primitives) {
        Quaternion inverse_rotation = conj(it->rotation);
        Ray ray = {
            .origin = rotate(world_ray.origin - it->position, inverse_rotation),
            .direction = rotate(world_ray.direction, inverse_rotation),
        };

        Intersection current;
        switch (it->type) {
            case PRIMITIVE_PLANE:
                current = intersect_plane(it, ray);
                break;
            case PRIMITIVE_ELLIPSOID:
                current = intersect_ellipsoid(it, ray);
                break;
            case PRIMITIVE_BOX:
                current = intersect_box(it, ray);
                break;
        }

        if (current.t > 0 && current.t < out.t && current.t < t_max) {
            out = current;
            *closest = it;
        }
    }

    return out;
}

Vector3 semi_sphere_rand(Vector2 unit_square_point, Vector3 normal)
{
    f32 theta = 2.0f * PI * unit_square_point.x;
    f32 z = unit_square_point.y;
    f32 h = sqrtf(1.0f - z * z);
    Vector3 w = {h * cosf(theta), h * sinf(theta), z};

    if (dot(normal, w) < 0) {
        w = -w;
    }

    return w;
}

Vector3 ray_trace(Scene *scene, Ray ray, u32 depth)
{
    if (depth > scene->ray_depth) {
        return {};
    }

    Primitive *closest = nullptr;
    Intersection intersection = intersect(scene->primitives, ray, &closest);

    if (!closest) {
        return scene->background_color;
    }

    Vector3 intersection_point = ray.origin + intersection.t * ray.direction;

    switch (closest->surface_type) {
    case SURFACE_DIFFUSE: {
        Vector3 direction = semi_sphere_rand({xoroshiro_next_f32(&scene->xoroshiro), xoroshiro_next_f32(&scene->xoroshiro)}, intersection.normal);
        Vector3 light = ray_trace(scene, {intersection_point + 1E-4 * intersection.normal, direction}, depth + 1);

        return closest->emission + (2.0f * dot(direction, intersection.normal) * light) * closest->color;
    } break;
    case SURFACE_METALLIC: {
        Ray reflected_ray = {
            .origin = intersection_point + 1E-4 * intersection.normal,
            .direction = reflect(-ray.direction, intersection.normal),
        };
        Vector3 light = ray_trace(scene, reflected_ray, depth + 1);

        return closest->emission + light * closest->color;
    } break;
    case SURFACE_DIELECTRIC: {
        Vector3 light = {};

        Ray reflected_ray = {
            .origin = intersection_point + 1E-4 * intersection.normal,
            .direction = reflect(-ray.direction, intersection.normal),
        };
        Vector3 reflected_light = ray_trace(scene, reflected_ray, depth + 1);

        f32 ior_quotient = intersection.inner ? closest->ior : (1.0f / closest->ior);
        f32 cos_1 = dot(intersection.normal, -ray.direction);
        f32 sin_2 = ior_quotient * sqrtf(1.0f - cos_1 * cos_1);
        if (sin_2 <= 1.0f) {
            f32 reflection_coefficient = square((ior_quotient - 1) / (ior_quotient + 1));
            f32 r = reflection_coefficient + (1 - reflection_coefficient) * powf(1.0f - cos_1, 5.0f);
            f32 random_number_in_unit_inverval = xoroshiro_next_f32(&scene->xoroshiro);
            if (random_number_in_unit_inverval < r) {
                light = reflected_light;
            } else {
                f32 cos_2 = sqrtf(1 - sin_2 * sin_2);
                Ray refracted_ray = {
                    .origin = intersection_point - 1E-4 * intersection.normal,
                    .direction = normalize(ior_quotient * ray.direction + (ior_quotient * cos_1 - cos_2) * intersection.normal),
                };
                Vector3 refracted_light = ray_trace(scene, refracted_ray, depth + 1);

                if (!intersection.inner) {
                    refracted_light *= closest->color;
                }

                light = refracted_light;
            }
        } else {
            light = reflected_light;
        }

        return closest->emission + light;
    } break;
    }
}

Vector3 aces_tonemap(Vector3 x)
{
    const Vector3 a = {2.51f, 2.51f, 2.51f};
    const Vector3 b = {0.03f, 0.03f, 0.03f};
    const Vector3 c = {2.43f, 2.43f, 2.43f};
    const Vector3 d = {0.59f, 0.59f, 0.59f};
    const Vector3 e = {0.14f, 0.14f, 0.14f};

    return pow(clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0f, 1.0f), 1.0f / 2.2f);
}

#define ROUND_COLOR(f) (roundf((f) * 255.0f))
void fill_pixels(Scene *scene, u8 *pixels)
{
    for (u32 y = 0; y < scene->height; y++) {
        for (u32 x = 0; x < scene->width; x++) {
            f32 tan_half_fov_x = tanf(scene->camera.fov_x_radians / 2);
            f32 tan_half_fov_y = (scene->height * tan_half_fov_x) / scene->width;

            Vector3 out_color = {};
            for (u32 i = 0; i < scene->samples; i++) {
                f32 offset_x = xoroshiro_next_f32(&scene->xoroshiro);
                f32 offset_y = xoroshiro_next_f32(&scene->xoroshiro);

                f32 normalized_x =  (2 * (x + offset_x) / scene->width  - 1) * tan_half_fov_x;
                f32 normalized_y = -(2 * (y + offset_y) / scene->height - 1) * tan_half_fov_y;
                Vector3 camera_direction = normalized_x * scene->camera.right + normalized_y * scene->camera.up + 1.0f * scene->camera.forward;

                Ray camera_ray = {
                    .origin = scene->camera.position,
                    .direction = normalize(camera_direction),
                };

                out_color += ray_trace(scene, camera_ray, 1);
            }

            out_color /= scene->samples;
            out_color = aces_tonemap(out_color);

            pixels[3 * (x + y * scene->width) + 0] = ROUND_COLOR(out_color.r);
            pixels[3 * (x + y * scene->width) + 1] = ROUND_COLOR(out_color.g);
            pixels[3 * (x + y * scene->width) + 2] = ROUND_COLOR(out_color.b);
        }
    }
}

PRIVATE_NAMESPACE_END

extern "C"
{

int main(int argc, char **argv)
{
    using namespace ray;

    if (argc != 3) {
        log("Invalid number of command line arguments.");
        return 1;
    }

    char *input_file_name = argv[1];
    FILE *file = fopen(input_file_name, "rb");
    if (!file) {
        log("File `%s` not found.", input_file_name);
        return 1;
    }

    fseek(file, 0, SEEK_END);
    u32 length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char *) malloc(length + 1);
    fread(buffer, 1, length, file);
    buffer[length] = 0;

    fclose(file);

    Scene scene = {};
    xoroshiro_set_seed(&scene.xoroshiro, time(nullptr));
    Parser parser = {.buffer = buffer, .length = length};
    parse(&parser, &scene);

    u8 *pixels = (u8 *) malloc(3 * scene.width * scene.height);

    Vector3 tonemapped_background_color = aces_tonemap(scene.background_color);
    for (u32 i = 0; i < 3 * scene.width * scene.height; i += 3) {
        pixels[i + 0] = ROUND_COLOR(tonemapped_background_color.r);
        pixels[i + 1] = ROUND_COLOR(tonemapped_background_color.g);
        pixels[i + 2] = ROUND_COLOR(tonemapped_background_color.b);
    }

    fill_pixels(&scene, pixels);

    write_ppm(argv[2], scene.width, scene.height, pixels);

#ifdef _WIN32
    write_bmp("out.bmp", scene.width, scene.height, pixels);
#endif

    return 0;
}

}

