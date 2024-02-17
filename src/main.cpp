#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <cmath>

#include "basic.h"
#include "math.h"

enum Primitive_Type
{
    PLANE = 0,
    ELLIPSOID = 1,
    BOX = 2,
};

struct Ray
{
    Vector3 origin, direction;
};

struct Primitive
{
    Primitive_Type type;
    Vector3 parameters;

    Vector3    position = {0, 0, 0};
    Quaternion rotation = {0, 0, 0, 1};
    Vector3    color;
};

struct State
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
};

#define ROUND_COLOR(f) (roundf((f) * 255.0f))

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

bool advance_cursor_if_starts_with(char *buffer, const char *c_string, u32 *cursor = nullptr) {
    u32 c = 0;
    while (c_string[c]) {
        if (c_string[c] != buffer[c]) {
            return false;
        }
        c++;
    }

    if (cursor) {
        *cursor += c;
    }

    return true;
}

Primitive parse_primitive(char *buffer, u32 length, u32 *cursor)
{
    Primitive primitive;
    while (*cursor < length) {
        char *current = &buffer[*cursor];
        if (advance_cursor_if_starts_with(current, "PLANE ", cursor)) {
            primitive.type = PLANE;
            sscanf(&buffer[*cursor], "%f %f %f", 
                &primitive.parameters.x, 
                &primitive.parameters.y, 
                &primitive.parameters.z 
            );
        } else if (advance_cursor_if_starts_with(current, "ELLIPSOID ", cursor)) {
            primitive.type = ELLIPSOID;
            sscanf(&buffer[*cursor], "%f %f %f", 
                &primitive.parameters.x, 
                &primitive.parameters.y, 
                &primitive.parameters.z 
            );
        } else if (advance_cursor_if_starts_with(current, "BOX ", cursor)) {
            primitive.type = BOX;
            sscanf(&buffer[*cursor], "%f %f %f", 
                &primitive.parameters.x, 
                &primitive.parameters.y, 
                &primitive.parameters.z 
            );
        } else if (advance_cursor_if_starts_with(current, "POSITION ", cursor)) {
            sscanf(&buffer[*cursor], "%f %f %f", 
                &primitive.position.x,
                &primitive.position.y,
                &primitive.position.z
            );
        } else if (advance_cursor_if_starts_with(current, "ROTATION ", cursor)) {
            sscanf(&buffer[*cursor], "%f %f %f %f", 
                &primitive.rotation.x,
                &primitive.rotation.y,
                &primitive.rotation.z,
                &primitive.rotation.w
            );
        } else if (advance_cursor_if_starts_with(current, "COLOR ", cursor)) {
            sscanf(&buffer[*cursor], "%f %f %f", 
                &primitive.color.x, 
                &primitive.color.y, 
                &primitive.color.z
            );
        } else if (advance_cursor_if_starts_with(current, "NEW_PRIMITIVE")) {
            break;
        }

        *cursor = skip_to_next_line(buffer, length, *cursor);
    }

    return primitive;
}

void parse(char *buffer, u32 length, State *state)
{
    u32 cursor = 0;

    while (cursor < length) {
        char *current = &buffer[cursor];

        if (advance_cursor_if_starts_with(current, "DIMENSIONS ", &cursor)) {
            sscanf(&buffer[cursor], "%d %d", &state->width, &state->height);
        } else if (advance_cursor_if_starts_with(current, "BG_COLOR ", &cursor)) {
            sscanf(&buffer[cursor], "%f %f %f", 
                &state->background_color.x, 
                &state->background_color.y, 
                &state->background_color.z
            ); 
        } else if (advance_cursor_if_starts_with(current, "CAMERA_POSITION ", &cursor)) {
            sscanf(&buffer[cursor], "%f %f %f", 
                &state->camera.position.x, 
                &state->camera.position.y, 
                &state->camera.position.z
            ); 
        } else if (advance_cursor_if_starts_with(current, "CAMERA_RIGHT ", &cursor)) {
            sscanf(&buffer[cursor], "%f %f %f", 
                &state->camera.right.x, 
                &state->camera.right.y, 
                &state->camera.right.z
            ); 
        } else if (advance_cursor_if_starts_with(current, "CAMERA_UP ", &cursor)) {
            sscanf(&buffer[cursor], "%f %f %f", 
                &state->camera.up.x, 
                &state->camera.up.y, 
                &state->camera.up.z
            ); 
        } else if (advance_cursor_if_starts_with(current, "CAMERA_FORWARD ", &cursor)) {
            sscanf(&buffer[cursor], "%f %f %f", 
                &state->camera.forward.x, 
                &state->camera.forward.y, 
                &state->camera.forward.z
            ); 
        } else if (advance_cursor_if_starts_with(current, "CAMERA_FOV_X ", &cursor)) {
            sscanf(&buffer[cursor], "%f", &state->camera.fov_x_radians);
        } else if (advance_cursor_if_starts_with(current, "NEW_PRIMITIVE", &cursor)) {
            array_push(&state->primitives, parse_primitive(buffer, length, &cursor));
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

void ray_trace(State *state, u8 *pixels)
{
    for (u32 y = 0; y < state->height; y++) {
        for (u32 x = 0; x < state->width; x++) {
            f32 t_min = INFINITY;
            Primitive *closest = nullptr;

            f32 tan_half_fov_x = tanf(state->camera.fov_x_radians / 2);
            f32 tan_half_fov_y = (state->height * tan_half_fov_x) / state->width;
            f32 normalized_x =  (2 * (x + 0.5f) / state->width  - 1) * tan_half_fov_x;
            f32 normalized_y = -(2 * (y + 0.5f) / state->height - 1) * tan_half_fov_y;
            Vector3 raw_direction = normalized_x * state->camera.right + normalized_y * state->camera.up + 1.0f * state->camera.forward;

            for (u32 i = 0; i < state->primitives.size; i++) {
                Primitive *primitive = &state->primitives[i];

                Quaternion inverse_rotation = conj(primitive->rotation);
                Ray ray = {
                    .origin = rotate(state->camera.position - primitive->position, inverse_rotation),
                    .direction = rotate(raw_direction, inverse_rotation),
                };

                f32 t = 0;
                switch (primitive->type) {
                    case PLANE:
                        t = intersect_plane(primitive, &ray);
                        break;
                    case ELLIPSOID:
                        t = intersect_ellipsoid(primitive, &ray);
                        break;
                    case BOX:
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
                pixels[3 * (x + y * state->width) + 0] = ROUND_COLOR(closest->color.r);
                pixels[3 * (x + y * state->width) + 1] = ROUND_COLOR(closest->color.g);
                pixels[3 * (x + y * state->width) + 2] = ROUND_COLOR(closest->color.b);
            }
        }
    }
}

int main(int argc, char **argv)
{
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

    State state = {};
    parse(buffer, length, &state);

    u8 *pixels = (u8 *) malloc(3 * state.width * state.height);
    for (u32 i = 0; i < 3 * state.width * state.height; i += 3) {
        pixels[i + 0] = ROUND_COLOR(state.background_color.r);
        pixels[i + 1] = ROUND_COLOR(state.background_color.g);
        pixels[i + 2] = ROUND_COLOR(state.background_color.b);
    }
    
    ray_trace(&state, pixels);
    
    write_ppm(argv[2], state.width, state.height, pixels);

#ifdef _WIN32
    write_bmp("out.bmp", state.width, state.height, pixels);
#endif

    return 0;
}

