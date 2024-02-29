#pragma once
#include "stb_image.h"
#include <glad/glad.h>

extern GLenum global_texture_unit;

typedef struct {
    stbi_uc** images;
    size_t clamp_width, clamp_height, clamp_components;
} texture_array_t;

typedef struct {
    GLuint object, texture_unit;
    GLenum kind;
} texture_object_t;

typedef size_t texture_instance_t;

GLenum texture_unit_next();

texture_array_t texture_array_init(size_t clamp_width, size_t clamp_height,
                                   size_t clamp_components);
void texture_array_deinit(texture_array_t* texture_array);
texture_instance_t texture_array_add(texture_array_t* arr,
                                     const char* texture_path);

texture_object_t texture_array_to_object(texture_array_t* arr);
