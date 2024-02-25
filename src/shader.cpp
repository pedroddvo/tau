#include "shader.h"
#include "resource.h"
#include "util.h"

#define INFO_LOG_SIZE 512

static GLuint compile_shader(const std::string& path, GLenum kind) {
    auto res = resource::read_bytes(path);
    assert_fatalf(res.has_value(), "failed to read shader source {}", path);

    assert(kind == GL_VERTEX_SHADER || kind == GL_FRAGMENT_SHADER);
    GLuint shader = glCreateShader(kind);

    const GLchar* src = res->data();
    const GLint src_size = res->size();
    glShaderSource(shader, 1, &src, &src_size);
    glCompileShader(shader);

    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        std::string buf(INFO_LOG_SIZE, '\0');
        glGetShaderInfoLog(shader, INFO_LOG_SIZE, NULL, buf.data());

        assert_fatalf(ok, "failed to compile shader {}\n{}", path, buf);
    }

    return shader;
}

shader::shader(const std::string& vs_path, const std::string& fs_path) {
    GLuint vshader = compile_shader(vs_path, GL_VERTEX_SHADER);
    GLuint fshader = compile_shader(fs_path, GL_FRAGMENT_SHADER);

    m_program = glCreateProgram();
    glAttachShader(m_program, vshader);
    glAttachShader(m_program, fshader);
    glLinkProgram(m_program);

    GLint ok = 0;
    glGetProgramiv(m_program, GL_LINK_STATUS, &ok);
    if (!ok) {
        std::string buf(INFO_LOG_SIZE, '\0');
        glGetProgramInfoLog(m_program, INFO_LOG_SIZE, NULL, buf.data());

        assert_fatalf(ok, "failed to link shader program {}, {}\n{}", vs_path,
                      fs_path, buf);
    }
}

void shader::set_mat4(const char* name, glm::mat4 mat) const {
    glUniformMatrix4fv(glGetUniformLocation(m_program, name), 1, GL_FALSE,
                       &mat[0][0]);
}
