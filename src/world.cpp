#include "world.h"
#include <chrono>
#include <future>
#include <spdlog/spdlog.h>

constexpr std::array<GLuint, 6> INDICES = {0, 1, 2, 2, 3, 0};

// clang-format off
using face_vertices = std::array<glm::vec3, 4>;
constexpr face_vertices FACE_FRONT = {{
    { 0.5f,  0.5f, -0.5f},
    { 0.5f, -0.5f, -0.5f},
    {-0.5f, -0.5f, -0.5f},
    {-0.5f,  0.5f, -0.5f},
}};

constexpr face_vertices FACE_BACK = {{
    {-0.5f,  0.5f,  0.5f},
    {-0.5f, -0.5f,  0.5f},
    { 0.5f, -0.5f,  0.5f},
    { 0.5f,  0.5f,  0.5f},
}};

constexpr face_vertices FACE_UP = {{
    { 0.5f,  0.5f,  0.5f},
    { 0.5f,  0.5f, -0.5f},
    {-0.5f,  0.5f, -0.5f},
    {-0.5f,  0.5f,  0.5f},
}};

constexpr face_vertices FACE_DOWN = {{
    {-0.5f, -0.5f,  0.5f},
    {-0.5f, -0.5f, -0.5f},
    { 0.5f, -0.5f, -0.5f},
    { 0.5f, -0.5f,  0.5f},
}};

constexpr face_vertices FACE_LEFT = {{
    {-0.5f,  0.5f, -0.5f},
    {-0.5f, -0.5f, -0.5f},
    {-0.5f, -0.5f,  0.5f},
    {-0.5f,  0.5f,  0.5f},
}};

constexpr face_vertices FACE_RIGHT = {{
    { 0.5f,  0.5f,  0.5f},
    { 0.5f, -0.5f,  0.5f},
    { 0.5f, -0.5f, -0.5f},
    { 0.5f,  0.5f, -0.5f},
}};

constexpr std::array<face_vertices, 6> FACE_VERTICES = {
    FACE_BACK, FACE_FRONT,
    FACE_UP, FACE_DOWN,
    FACE_RIGHT, FACE_LEFT,
};

constexpr std::array<glm::vec3, 6> FACES_OFFSETS = {{
    {  0.0f,  0.0f,  1.0f}, {  0.0f,  0.0f, -1.0f},
    {  0.0f,  1.0f,  0.0f}, {  0.0f, -1.0f,  0.0f},
    {  1.0f,  0.0f,  0.0f}, { -1.0f,  0.0f,  0.0f},
}};
// clang-format on

world::render_result world::render_chunk_at(chunk::pos pos) {
    for (auto&& chunk : m_load_queue) {
        m_loaded_chunks[chunk.get_pos()] = std::move(chunk);
    }
    m_load_queue.clear();

    assert(m_loaded_chunks.contains(pos));

    render_result result;
    result.pos = pos;

    chunk& current_chunk = m_loaded_chunks[pos];

    std::size_t indices_offset = 0;
    CHUNK_EACH(x, y, z) {
        chunk::pos block_pos = pos * chunk::DIM + chunk::pos(x, y, z);
        auto this_block_inst = current_chunk.get_block(block_pos);
        const block& this_block = m_block_registry.index_block(this_block_inst);

        if (!this_block.is_visible) continue;

        for (std::size_t face = 0; face < 6; face++) {
            glm::vec3 face_offset = FACES_OFFSETS[face];
            glm::ivec3 adjacent_block_pos{block_pos.x + face_offset.x,
                                          block_pos.y + face_offset.y,
                                          block_pos.z + face_offset.z};

            if (block_visible_at(adjacent_block_pos)) continue;

            for (std::size_t i = 0; i < 4; i++) {
                glm::vec3 vertex = FACE_VERTICES[face][i];
                block::vertex block_vertex;

                block_vertex.pos = {x + vertex.x, y + vertex.y, z + vertex.z};
                block_vertex.index = i;
                block_vertex.texture = this_block.textures[face];
                block_vertex.face = face;
                result.vertices.push_back(block_vertex);
            }

            for (const auto& index : INDICES) {
                result.indices.push_back(index + indices_offset);
            }
            indices_offset += 4;
        }
    }

    return result;
}

bool world::block_visible_at(chunk::pos block_pos, bool only_if_air) {
    chunk::pos belonging_chunk = chunk::of_block(block_pos);

    auto chunk_it = m_loaded_chunks.find(belonging_chunk);
    if (chunk_it == m_loaded_chunks.end()) {
        return only_if_air;
    }

    auto inst = chunk_it->second.get_block(block_pos);
    return m_block_registry.index_block(inst).is_visible;
}

void world::load_chunks_around(int dim, chunk::pos center) {
    for (int x = -dim / 2; x < dim / 2; x++) {
        for (int z = -dim / 2; z < dim / 2; z++) {
            load_chunk_at(center + chunk::pos(x, 0, z));
        }
    }
}

void world::load_chunk_at(chunk::pos pos) {
    if (m_rendered_chunks.contains(pos)) return;

    m_load_queue.push_back(chunk(pos, [this](chunk::pos p) {
        int rdm = std::rand() / ((RAND_MAX + 1u) / 2);
        return rdm ? air : grass;
    }));
    m_render_queue.insert(pos);
}

std::vector<world::render_result> world::render_all_in_queue() {
    std::vector<render_result> results;

    auto queue_copy = m_render_queue;
    for (const auto& pos : queue_copy) {
        results.push_back(render_chunk_at(pos));
        m_render_queue.erase(pos);
    }

    m_render_future_done = true;
    return results;
}

void world::render(glm::mat4 projection, glm::mat4 view) {
    if (m_render_future_done && m_render_queue.size() > 0) {
        m_render_future_done = false;

        std::vector<render_result> results = m_render_future.get();
        for (const auto& result : results) {
            m_rendered_chunks[result.pos] = std::make_unique<chunk::mesh>(
                result.vertices, result.indices, result.pos);
        }

        m_render_future = std::async(&world::render_all_in_queue, this);
    }

    m_block_textures.bind();

    m_block_shader.use();
    m_block_shader.set_mat4("projection", projection);
    m_block_shader.set_mat4("view", view);

    for (int x = -RENDER_DISTANCE; x < RENDER_DISTANCE; x++) {
        for (int z = -RENDER_DISTANCE; z < RENDER_DISTANCE; z++) {
            auto it = m_rendered_chunks.find({
                x + m_camera.m_pos.x / chunk::DIM,
                0,
                z + m_camera.m_pos.z / chunk::DIM,
            });
            if (it == m_rendered_chunks.end()) continue;

            it->second->render(m_block_shader);
        }
    }

    for (auto first = m_rendered_chunks.begin(), last = m_rendered_chunks.end();
         first != last;) {
        chunk::pos p = first->second->m_pos;
        const int bounds_x1 =
            m_camera.m_pos.x / chunk::DIM + RENDER_DISTANCE * 2;
        const int bounds_x2 =
            m_camera.m_pos.x / chunk::DIM - RENDER_DISTANCE * 2;
        const int bounds_z1 =
            m_camera.m_pos.z / chunk::DIM + RENDER_DISTANCE * 2;
        const int bounds_z2 =
            m_camera.m_pos.z / chunk::DIM - RENDER_DISTANCE * 2;

        if (p.x < bounds_x2 || p.x > bounds_x1 || p.z < bounds_z2 ||
            p.z > bounds_z1) {
            first = m_rendered_chunks.erase(first);
        } else {
            ++first;
        }
    }
}
