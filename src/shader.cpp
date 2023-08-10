#include "shader.hpp"

#include <spdlog/spdlog.h>

#include "resource.hpp"

static std::optional<GLuint> compile_shader(GLenum kind,
                                            const std::string& src) {

    GLuint shader = glCreateShader(kind);

    const char* src_c = src.c_str();
    glShaderSource(shader, 1, &src_c, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint log_len;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);

        std::string log(log_len, '\0');
        glGetShaderInfoLog(shader, log_len, &log_len, log.data());

        spdlog::error(log);
        return {};
    }

    return shader;
}

std::optional<Shader> Shader::from_files(const std::string& vertex_path,
                                         const std::string& fragment_path) {
    auto vertex_src = resource::read_resource_to_string(vertex_path);
    auto fragment_src = resource::read_resource_to_string(fragment_path);
    if (!vertex_src.has_value() || !fragment_src.has_value()) return {};

    auto vertex_shader = compile_shader(GL_VERTEX_SHADER, *vertex_src);
    auto fragment_shader = compile_shader(GL_FRAGMENT_SHADER, *fragment_src);
    if (!vertex_shader.has_value() || !fragment_shader.has_value()) return {};


    GLuint program = glCreateProgram();
    glAttachShader(program, *vertex_shader);
    glAttachShader(program, *fragment_shader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLint log_len;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_len);

        std::string log(log_len, '\0');
        glGetProgramInfoLog(program, log_len, &log_len, log.data());

        spdlog::error(log);
        return {};
    }

    return Shader(program);
}

void Shader::use() const { glUseProgram(program); }
