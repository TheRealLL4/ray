#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <cmath>

#define PRIVATE_NAMESPACE_NAME  ray
#define PRIVATE_NAMESPACE_BEGIN namespace PRIVATE_NAMESPACE_NAME { namespace {
#define PRIVATE_NAMESPACE_END   } }

PRIVATE_NAMESPACE_BEGIN

#include "basic.h"
#include "math.h"

enum Primitive_Type
{
    PRIMITIVE_TYPE_PLANE     = 0,
    PRIMITIVE_TYPE_ELLIPSOID = 1,
    PRIMITIVE_TYPE_BOX       = 2,
};

enum Surface_Type
{
    SURFACE_TYPE_DIFFUSE    = 0,
    SURFACE_TYPE_METALLIC   = 1,
    SURFACE_TYPE_DIELECTRIC = 2,
};

struct Primitive
{
    Primitive_Type type;
    Surface_Type surface_type = SURFACE_TYPE_DIFFUSE;
    f32 ior;
    Vector3 parameters;

    Vector3    position = {0, 0, 0};
    Quaternion rotation = {0, 0, 0, 1};
    Vector3    color;
};

enum Light_Type
{
    LIGHT_TYPE_DIRECTIONAL = 0,
    LIGHT_TYPE_POINT       = 1,
};

struct Light
{
    Light_Type type;
    union
    {
        // Directional light
        Vector3 direction;

        // Point light
        struct
        {
            Vector3 position;
            Vector3 intensity;
            Vector3 attenuation;
        };
    };
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
    Vector3 ambient_light;
    Array<Light> lights;
};

struct Ray
{
    Vector3 origin, direction;
};

u32 skip_to_next_line(char *buffer, u32 length, u32 cursor)
{
    while (cursor < length && buffer[cursor] != '\n' && buffer[cursor] != '\r') {
        cursor++;
    }

    if (cursor < length) {
        cursor++;
    }
 
    return cursor;
}

bool advance_cursor_if_starts_with(char *buffer, const char *c_string, u32 *cursor)
{
    u32 c = 0;
    while (c_string[c]) {
        if (c_string[c] != buffer[c]) {
            return false;
        }
        c++;
    }

    *cursor += c;

    return true;
}

#define SCAN_VECTOR3(buf, v) sscanf(buf, "%f %f %f", &(v).x, &(v).y, &(v).z)
Primitive parse_primitive(char *buffer, u32 length, u32 *cursor)
{
    Primitive primitive;
    while (*cursor < length) {
        char *current = &buffer[*cursor];
        if (advance_cursor_if_starts_with(current, "PLANE ", cursor)) {
            primitive.type = PRIMITIVE_TYPE_PLANE;
            SCAN_VECTOR3(&buffer[*cursor], primitive.parameters);
        } else if (advance_cursor_if_starts_with(current, "ELLIPSOID ", cursor)) {
            primitive.type = PRIMITIVE_TYPE_ELLIPSOID;
            SCAN_VECTOR3(&buffer[*cursor], primitive.parameters);
        } else if (advance_cursor_if_starts_with(current, "BOX ", cursor)) {
            primitive.type = PRIMITIVE_TYPE_BOX;
            SCAN_VECTOR3(&buffer[*cursor], primitive.parameters);
        } else if (advance_cursor_if_starts_with(current, "POSITION ", cursor)) {
            SCAN_VECTOR3(&buffer[*cursor], primitive.position);
        } else if (advance_cursor_if_starts_with(current, "ROTATION ", cursor)) {
            sscanf(&buffer[*cursor], "%f %f %f %f", 
                &primitive.rotation.x,
                &primitive.rotation.y,
                &primitive.rotation.z,
                &primitive.rotation.w
            );
        } else if (advance_cursor_if_starts_with(current, "COLOR ", cursor)) {
            SCAN_VECTOR3(&buffer[*cursor], primitive.color);
        } else if (advance_cursor_if_starts_with(current, "METALLIC", cursor)) {
            primitive.surface_type = SURFACE_TYPE_METALLIC;
        } else if (advance_cursor_if_starts_with(current, "DIELECTRIC", cursor)) {
            primitive.surface_type = SURFACE_TYPE_DIELECTRIC;
        } else if (advance_cursor_if_starts_with(current, "IOR ", cursor)) {
            sscanf(&buffer[*cursor], "%f", &primitive.ior);
        } else { 
            break;
        }

        *cursor = skip_to_next_line(buffer, length, *cursor);
    }

    return primitive;
}

Light parse_light(char *buffer, u32 length, u32 *cursor)
{
    Light light;
    while (*cursor < length) {
        char *current = &buffer[*cursor];
        if (advance_cursor_if_starts_with(current, "LIGHT_INTENSITY ", cursor)) {
            SCAN_VECTOR3(&buffer[*cursor], light.intensity);
        } else if (advance_cursor_if_starts_with(current, "LIGHT_DIRECTION ", cursor)) {
            light.type = LIGHT_TYPE_DIRECTIONAL;
            SCAN_VECTOR3(&buffer[*cursor], light.direction);
        } else if (advance_cursor_if_starts_with(current, "LIGHT_POSITION ", cursor)) {
            light.type = LIGHT_TYPE_POINT;
            SCAN_VECTOR3(&buffer[*cursor], light.position);
        } else if (advance_cursor_if_starts_with(current, "LIGHT_ATTENUATION ", cursor)) {
            SCAN_VECTOR3(&buffer[*cursor], light.attenuation);
        } else {
            break;
        }

        *cursor = skip_to_next_line(buffer, length, *cursor);
    }

    return light;
}

void parse(char *buffer, u32 length, Scene *scene)
{
    u32 cursor = 0;

    while (cursor < length) {
        char *current = &buffer[cursor];
        if (advance_cursor_if_starts_with(current, "DIMENSIONS ", &cursor)) {
            sscanf(&buffer[cursor], "%d %d", &scene->width, &scene->height);
        } else if (advance_cursor_if_starts_with(current, "BG_COLOR ", &cursor)) {
            SCAN_VECTOR3(&buffer[cursor], scene->background_color);
        } else if (advance_cursor_if_starts_with(current, "CAMERA_POSITION ", &cursor)) {
            SCAN_VECTOR3(&buffer[cursor], scene->camera.position);
        } else if (advance_cursor_if_starts_with(current, "CAMERA_RIGHT ", &cursor)) {
            SCAN_VECTOR3(&buffer[cursor], scene->camera.right);
        } else if (advance_cursor_if_starts_with(current, "CAMERA_UP ", &cursor)) {
            SCAN_VECTOR3(&buffer[cursor], scene->camera.up);
        } else if (advance_cursor_if_starts_with(current, "CAMERA_FORWARD ", &cursor)) {
            SCAN_VECTOR3(&buffer[cursor], scene->camera.forward);
        } else if (advance_cursor_if_starts_with(current, "CAMERA_FOV_X ", &cursor)) {
            sscanf(&buffer[cursor], "%f", &scene->camera.fov_x_radians);
        } else if (advance_cursor_if_starts_with(current, "NEW_PRIMITIVE\n", &cursor)) {
            array_push(&scene->primitives, parse_primitive(buffer, length, &cursor));
            continue;
        } else if (advance_cursor_if_starts_with(current, "RAY_DEPTH ", &cursor)) {
            sscanf(&buffer[cursor], "%u", &scene->ray_depth);
        } else if (advance_cursor_if_starts_with(current, "AMBIENT_LIGHT ", &cursor)) {
            SCAN_VECTOR3(&buffer[cursor], scene->ambient_light);
        } else if (advance_cursor_if_starts_with(current, "NEW_LIGHT\n", &cursor)) {
            array_push(&scene->lights, parse_light(buffer, length, &cursor));
            continue;
        }

        cursor = skip_to_next_line(buffer, length, cursor);
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

    // Write pixels as BGR and flip them upside-down
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

f32 intersect_plane(Primitive *primitive, Ray *ray)
{
    Vector3 normal = primitive->parameters;
    return (-dot(ray->origin, normal) / dot(ray->direction, normal));
}

f32 intersect_ellipsoid(Primitive *primitive, Ray *ray)
{
    Vector3 semi_axes = primitive->parameters;

    f32 a = dot(ray->direction / semi_axes, ray->direction / semi_axes);
    f32 b = 2 * dot(ray->origin / semi_axes, ray->direction / semi_axes);
    f32 c = dot(ray->origin / semi_axes, ray->origin / semi_axes) - 1.0f;

    f32 discriminant = b * b - 4 * a * c;
    if (discriminant < 0) {
        return -1;
    }

    f32 t_min = (-b - sqrtf(discriminant)) / (2 * a);
    f32 t_max = (-b + sqrtf(discriminant)) / (2 * a);

    if (t_min > 0) {
        return t_min;
    }

    return t_max;
}

f32 intersect_box(Primitive *primitive, Ray *ray)
{
    Vector3 dimensions = primitive->parameters;

    Vector3 t1 = (-dimensions - ray->origin) / ray->direction;
    Vector3 t2 = ( dimensions - ray->origin) / ray->direction;

    Vector3 t_min = min(t1, t2);
    Vector3 t_max = max(t1, t2);

    f32 interval_min = max(t_min);
    f32 interval_max = min(t_max);

    if (interval_min > interval_max) {
        return -1;
    }

    if (interval_min > 0) {
        return interval_min;
    }

    return interval_max;
}

#define ROUND_COLOR(f) (roundf((f) * 255.0f))

void ray_trace(Scene *scene, u8 *pixels)
{
    for (u32 y = 0; y < scene->height; y++) {
        for (u32 x = 0; x < scene->width; x++) {
            f32 t_min = INFINITY;
            Primitive *closest = nullptr;

            f32 tan_half_fov_x = tanf(scene->camera.fov_x_radians / 2);
            f32 tan_half_fov_y = (scene->height * tan_half_fov_x) / scene->width;
            f32 normalized_x =  (2 * (x + 0.5f) / scene->width  - 1) * tan_half_fov_x;
            f32 normalized_y = -(2 * (y + 0.5f) / scene->height - 1) * tan_half_fov_y;
            Vector3 raw_direction = normalized_x * scene->camera.right + normalized_y * scene->camera.up + 1.0f * scene->camera.forward;

            for (u32 i = 0; i < scene->primitives.size; i++) {
                Primitive *primitive = &scene->primitives[i];

                Quaternion inverse_rotation = conj(primitive->rotation);
                Ray ray = {
                    .origin = rotate(scene->camera.position - primitive->position, inverse_rotation),
                    .direction = rotate(raw_direction, inverse_rotation),
                };

                f32 t = 0;
                switch (primitive->type) {
                    case PRIMITIVE_TYPE_PLANE:
                        t = intersect_plane(primitive, &ray);
                        break;
                    case PRIMITIVE_TYPE_ELLIPSOID:
                        t = intersect_ellipsoid(primitive, &ray);
                        break;
                    case PRIMITIVE_TYPE_BOX:
                        t = intersect_box(primitive, &ray);
                        break;
                    default:
                        break;
                }

                if (t > 0 && t_min > t) {
                    t_min = t;
                    closest = primitive;
                }
            }

            if (closest) {
                pixels[3 * (x + y * scene->width) + 0] = ROUND_COLOR(closest->color.r);
                pixels[3 * (x + y * scene->width) + 1] = ROUND_COLOR(closest->color.g);
                pixels[3 * (x + y * scene->width) + 2] = ROUND_COLOR(closest->color.b);
            }
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
    parse(buffer, length, &scene);

    u8 *pixels = (u8 *) malloc(3 * scene.width * scene.height);
    for (u32 i = 0; i < 3 * scene.width * scene.height; i += 3) {
        pixels[i + 0] = ROUND_COLOR(scene.background_color.r);
        pixels[i + 1] = ROUND_COLOR(scene.background_color.g);
        pixels[i + 2] = ROUND_COLOR(scene.background_color.b);
    }
    
    ray_trace(&scene, pixels);
    
    write_ppm(argv[2], scene.width, scene.height, pixels);

#ifdef _WIN32
    write_bmp("out.bmp", scene.width, scene.height, pixels);
#endif

    return 0;
}

}

