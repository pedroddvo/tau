#if !defined(SHADER_H)
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <optional>
#include <string>

class Shader {
  public:
    static std::optional<Shader> from_files(const std::string& vertex_path,
                                            const std::string& fragment_path);

    void set_mat4(const std::string& name, const glm::mat4& mat) const {
        glUniformMatrix4fv(glGetUniformLocation(program, name.c_str()), 1,
                           GL_FALSE, &mat[0][0]);
    }

    void use() const;

  private:
    Shader(GLuint program) : program(program) {}

    GLuint program;
};

#endif // SHADER_H