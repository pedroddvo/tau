#if !defined(CHUNK_H)
#define CHUNK_H

#include "shader.hpp"
#include "texture.hpp"
#include <array>
#include <functional>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <vector>

struct Block_Skeleton {
    Block_Skeleton() {}

    bool is_visible = true;
    std::array<Texture_Offset, 6> tex_offsets = {0};
};

typedef std::size_t Block_Index;

class Block_Registry {
  public:
    Block_Registry() : skeletons() {}

    Block_Index register_block_skeleton(Block_Skeleton skeleton);
    const Block_Skeleton& index_block_skeleton(Block_Index index) const;

  private:
    std::vector<Block_Skeleton> skeletons;
};

struct Block_Vertex {
    glm::vec3 vertices;
    glm::vec2 tex_coord;
    GLuint tex_offset;
};

class Chunk_Mesh {
  public:
    glm::mat4 model;
    std::vector<Block_Vertex> vertices;
    std::vector<GLuint> indices;

    void render(const Shader& face_shader, const glm::mat4& projection,
                const glm::mat4& view, const Texture_2D_Array& texture_array);

    Chunk_Mesh(glm::vec3 position, std::vector<Block_Vertex> vertices,
               std::vector<GLuint> indices)
        : vertices(vertices), indices(indices) {
        model = glm::translate(glm::mat4(1.0f), position);

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Block_Vertex) * vertices.size(),
                     vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(),
                     indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Block_Vertex),
                              (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Block_Vertex),
                              (void*)(sizeof(glm::vec3)));
        glEnableVertexAttribArray(1);

        glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(Block_Vertex),
                               (void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

  private:
    GLuint VAO, VBO, EBO;
};

class Chunk {
  public:
    static constexpr size_t DIM = 16, SIZE = DIM * DIM * DIM;

    Chunk(glm::vec3 position,
          std::function<Block_Index(glm::vec3)> fill_function);

    Chunk_Mesh build_mesh(const Block_Registry& registry);

  private:
    std::array<Block_Index, SIZE> blocks;
    glm::vec3 position;
};

#endif // CHUNK_H
