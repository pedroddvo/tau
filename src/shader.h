#pragma once
#include "texture.h"
#include <cglm/cglm.h>
#include <glad/glad.h>

typedef struct {
    GLuint program;
} shader_t;

shader_t shader_from_files(const char* vs_path, const char* fs_path);

static inline void shader_use(const shader_t* shader) {
    glUseProgram(shader->program);
}

static inline void shader_set_mat4(const shader_t* shader, const char* name,
                                   mat4 mat) {
    glUniformMatrix4fv(glGetUniformLocation(shader->program, name), 1, GL_FALSE,
                       mat[0]);
}

static inline void shader_set_texture(const shader_t* shader, const char* name,
                                      const texture_object_t* object) {
    glActiveTexture(object->texture_unit);
    glBindTexture(object->kind, object->object);
    glUniform1i(glGetUniformLocation(shader->program, name),
                object->texture_unit);
}
