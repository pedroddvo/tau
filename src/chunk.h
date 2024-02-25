#pragma once
#include "shader.h"
#include "texture.h"
#include <array>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <span>
#include <spdlog/spdlog.h>

#define block_texture_all_faces(id)                                            \
    { id, id, id, id, id, id }
#define block_texture_same_sides(top, bottom, side)                            \
    { side, side, top, bottom, side, side }

struct block {
    using instance = std::uint8_t;
    struct vertex {
        glm::vec3 pos;
        GLuint index, texture, face;
    };

    bool is_visible = true;
    std::array<texture_array::id, 6> textures = {0};
};

class block_registry {
  public:
    block_registry() : m_registered() {}

    block::instance register_block(block b) {
        assert(m_registered_size < MAX_REGISTERED_BLOCKS);
        m_registered[m_registered_size] = b;
        return m_registered_size++;
    }

    const block& index_block(block::instance bi) const {
        assert(bi < m_registered_size);
        return m_registered[bi];
    }

  private:
    static constexpr block::instance MAX_REGISTERED_BLOCKS = UINT8_MAX;

    std::array<block, MAX_REGISTERED_BLOCKS> m_registered;
    block::instance m_registered_size = 0;
};

class chunk {
  public:
    using pos = glm::ivec3;

    struct mesh {
        mesh(std::span<const block::vertex> vertices,
             std::span<const GLuint> indices, pos p);
        mesh() : m_vao(), m_indices_count(0), m_pos() {}
        ~mesh();

        void render(const shader& chunk_shader) const;

        GLuint m_vao, m_vbo, m_ebo;
        std::size_t m_indices_count;
        pos m_pos;
    };

    static constexpr int DIM = 16, SIZE = DIM * DIM * DIM;
    using blocks = std::array<block::instance, SIZE>;
    using fill_function = std::function<block::instance(pos p)>;

    explicit chunk(pos p, fill_function fill);
    explicit chunk() : m_blocks(), m_pos() {}

    chunk(chunk&& other)
        : m_blocks(std::move(other.m_blocks)), m_pos(other.m_pos) {}
    chunk& operator=(chunk&& other) {
        m_blocks = std::move(other.m_blocks);
        return *this;
    }

    // from a block position within the chunk, find the chunk it belongs to
    static chunk::pos of_block(chunk::pos block_pos) {
        const auto round = [](int a) {
            return (a / chunk::DIM) + ((a % chunk::DIM) >> 31);
        };

        return {round(block_pos.x), round(block_pos.y), round(block_pos.z)};
    }

    const pos& get_pos() const { return m_pos; }
    const blocks& get_blocks() const { return m_blocks; }
    inline const block::instance& get_block(pos p) const {
        const auto clamp = [](int i) {
            return (chunk::DIM + (i % chunk::DIM)) % chunk::DIM;
        };

        return m_blocks[(clamp(p.z) * DIM * DIM) + (clamp(p.y) * DIM) +
                        clamp(p.x)];
    }

  private:
    blocks m_blocks;
    const pos m_pos;
};

template <> struct std::hash<chunk::pos> {
    std::size_t operator()(const chunk::pos& p) const {
        std::size_t h = 17;
        h = h * 31 + hash<int>()(p.x);
        h = h * 31 + hash<int>()(p.y);
        h = h * 31 + hash<int>()(p.z);
        return h;
    }
};

#define CHUNK_EACH(x, y, z)                                                    \
    for (int x = 0; x < chunk::DIM; x++)                                       \
        for (int y = 0; y < chunk::DIM; y++)                                   \
            for (int z = 0; z < chunk::DIM; z++)
