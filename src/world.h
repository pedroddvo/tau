#pragma once
#include "camera.h"
#include "chunk.h"
#include <future>
#include <glad/glad.h>
#include <list>
#include <spdlog/spdlog.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define RENDER_DISTANCE 16

class world {
  public:
    explicit world(const camera& cam)
        : m_block_registry(), m_loaded_chunks(), m_render_queue(),
          m_camera(cam) {
        texture_array block_txt_array{16, 16, STBI_rgb_alpha};

        block_txt_array.add_texture(
            "blocks/missing_tile.png"); // missing texture = 0

        auto txt_grass =
            block_txt_array.add_texture("blocks/grass_carried.png");
        auto txt_grass_side =
            block_txt_array.add_texture("blocks/grass_side_carried.png");
        auto txt_dirt = block_txt_array.add_texture("blocks/dirt.png");

        m_block_textures = block_txt_array.render_to_object({
            {GL_TEXTURE_MAG_FILTER, GL_NEAREST},
            {GL_TEXTURE_MIN_FILTER, GL_LINEAR},
        });

        block block_air;
        block_air.is_visible = false;
        air = m_block_registry.register_block(block_air);
        grass = m_block_registry.register_block({
            true,
            block_texture_same_sides(txt_grass, txt_dirt, txt_grass_side),
        });
    }

    void load_chunk_at(chunk::pos pos);
    void load_chunks_around(int dim, chunk::pos center);
    bool block_visible_at(chunk::pos block_pos, bool only_if_air = false);

    void render(glm::mat4 projection, glm::mat4 view);

  private:
    struct render_result {
        std::vector<block::vertex> vertices;
        std::vector<GLuint> indices;
        chunk::pos pos;
    };

    render_result render_chunk_at(chunk::pos pos);
    std::vector<render_result> render_all_in_queue();

    block_registry m_block_registry;
    std::unordered_map<chunk::pos, chunk> m_loaded_chunks;
    std::vector<chunk> m_load_queue;

    std::unordered_set<chunk::pos> m_render_queue;
    std::unordered_map<chunk::pos, std::unique_ptr<chunk::mesh>>
        m_rendered_chunks;

    std::atomic<bool> m_render_future_done = false;
    std::future<std::vector<render_result>> m_render_future =
        std::async(&world::render_all_in_queue, this);

    const camera& m_camera;

    texture_array::object m_block_textures;

    block::instance air, grass;

    shader m_block_shader{"tri.vert", "tri.frag"};
};
