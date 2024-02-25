#include "chunk.h"
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>

chunk::chunk(pos p, fill_function fill) : m_pos(p) {
    std::size_t i = 0;
    CHUNK_EACH(x, y, z) {
        m_blocks[i++] = fill({p.x * DIM + x, p.y * DIM + y, p.z * DIM + z});
    }
}

chunk::mesh::mesh(std::span<const block::vertex> vertices,
                  std::span<const GLuint> indices, pos p)
    : m_indices_count(indices.size()), m_pos(p) {
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(block::vertex) * vertices.size(),
                 vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(),
                 indices.data(), GL_STATIC_DRAW);

    // pos
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(block::vertex),
                          (void*)0);
    glEnableVertexAttribArray(0);

    // index, texture, face
    glVertexAttribIPointer(1, 3, GL_UNSIGNED_INT, sizeof(block::vertex),
                           (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

chunk::mesh::~mesh() {
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ebo);
}

void chunk::mesh::render(const shader& chunk_shader) const {
    chunk_shader.set_mat4(
        "model", glm::translate(glm::mat4(1.0f),
                                {m_pos.x * DIM, m_pos.y * DIM, m_pos.z * DIM}));

    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_indices_count, GL_UNSIGNED_INT, 0);
}
