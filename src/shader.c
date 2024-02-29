#include "shader.h"
#include "resource.h"
#include "util.h"
#include <assert.h>

static GLuint shader_compile(const char* path, GLenum kind) {
    assert(kind == GL_VERTEX_SHADER || kind == GL_FRAGMENT_SHADER);
    GLuint shader = glCreateShader(kind);

    size_t src_size = 0;
    char* src = resource_read_file(path, &src_size);
    assert_fatalf(src, "failed to read shader file %s", path);
    glShaderSource(shader, 1, (const GLchar**)&src, (const GLint*)&src_size);
    free(src);

    GLint ok = 0;
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512] = {0};
        glGetShaderInfoLog(shader, 512, NULL, log);
        assert_fatalf(false, "failed to compile shader resource/%s\n%s", path,
                      log);
    }

    return shader;
}

shader_t shader_from_files(const char* vs_path, const char* fs_path) {
    GLuint vshader = shader_compile(vs_path, GL_VERTEX_SHADER);
    GLuint fshader = shader_compile(fs_path, GL_FRAGMENT_SHADER);

    shader_t shader = (shader_t){.program = glCreateProgram()};
    glAttachShader(shader.program, vshader);
    glAttachShader(shader.program, fshader);
    glLinkProgram(shader.program);

    glDeleteShader(vshader);
    glDeleteShader(fshader);

    GLint ok = 0;
    glGetProgramiv(shader.program, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[512] = {0};
        glGetProgramInfoLog(shader.program, 512, NULL, log);
        assert_fatalf(
            false,
            "failed to compile shader program resource/%s resource/%s\n%s",
            vs_path, fs_path, log);
    }

    return shader;
}

