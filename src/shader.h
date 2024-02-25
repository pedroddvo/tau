#pragma once
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <string>

class shader {
  public:
    explicit shader(const std::string& vs_path, const std::string& fs_path);

    void use() const { glUseProgram(m_program); }
    void set_mat4(const char* name, glm::mat4 mat) const;

  private:
    GLuint m_program;
};
