#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cstdarg>

#include "basic.h"

enum Primitive_Type
{
    PLANE = 0,
    ELLIPSOID = 1,
    BOX = 2,
};

struct Vector3
{
    f32 x = 0, y = 0, z = 0;
};

struct Vector4
{
    f32 x = 0, y = 0, z = 0, w = 1;
};

struct Primitive
{
    Primitive_Type type;
    f32 parameters[3];

    Vector3 position;
    Vector4 rotation;
    Vector3 color;
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
        f32     fov_x;
    } camera;

    Array<Primitive> primitives;
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
                &primitive.parameters[0], 
                &primitive.parameters[1], 
                &primitive.parameters[2] 
            );
        } else if (advance_cursor_if_starts_with(current, "ELLIPSOID ", cursor)) {
            primitive.type = ELLIPSOID;
            sscanf(&buffer[*cursor], "%f %f %f", 
                &primitive.parameters[0], 
                &primitive.parameters[1], 
                &primitive.parameters[2] 
            );
        } else if (advance_cursor_if_starts_with(current, "BOX ", cursor)) {
            primitive.type = BOX;
            sscanf(&buffer[*cursor], "%f %f %f", 
                &primitive.parameters[0], 
                &primitive.parameters[1], 
                &primitive.parameters[2] 
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
            sscanf(&buffer[cursor], "%f", &state->camera.fov_x);
        } else if (advance_cursor_if_starts_with(current, "NEW_PRIMITIVE", &cursor)) {
            array_push(&state->primitives, parse_primitive(buffer, length, &cursor));
            continue;
        }

        cursor = skip_to_next_line(buffer, length, cursor);
    }
}

void write_ppm(char *file_name, u32 width, u32 height, u8 *pixels)
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

void ray_trace(State *state, u8 *pixels)
{

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
        pixels[i + 0] = round(255 * state.background_color.x);
        pixels[i + 1] = round(255 * state.background_color.y);
        pixels[i + 2] = round(255 * state.background_color.z);
    }

    ray_trace(&state, pixels);
    
    write_ppm(argv[2], state.width, state.height, pixels);

    return 0;
}

