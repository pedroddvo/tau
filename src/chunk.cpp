#include "chunk.hpp"

Block_Index Block_Registry::register_block_skeleton(Block_Skeleton skeleton) {
    skeletons.push_back(skeleton);
    return skeletons.size() - 1;
}

const Block_Skeleton&
Block_Registry::index_block_skeleton(Block_Index index) const {
    return skeletons.at(index);
}

inline int to_1d(int x, int y, int z) {
    return (z * Chunk::DIM * Chunk::DIM) + (y * Chunk::DIM) + x;
}

Chunk::Chunk(glm::vec3 position,
             std::function<Block_Index(glm::vec3)> fill_function) {
    this->position = position;

    for (size_t x = 0; x < DIM; x++) {
        for (size_t y = 0; y < DIM; y++) {
            for (size_t z = 0; z < DIM; z++) {
                const auto i = to_1d(x, y, z);
                blocks[i] = fill_function(
                    glm::vec3(x + position.x, y + position.y, z + position.z));
            }
        }
    }
}

// clang-format off

static const std::array<GLfloat, 12> CUBE_NORTH_VERTICES = {
     0.5,  0.5, -0.5,
    -0.5,  0.5, -0.5,
    -0.5,  0.5,  0.5,
     0.5,  0.5,  0.5,
};

static const std::array<GLfloat, 12> CUBE_SOUTH_VERTICES = {
     0.5, -0.5,  0.5,
    -0.5, -0.5,  0.5,
    -0.5, -0.5, -0.5,
     0.5, -0.5, -0.5,
};

static const std::array<GLfloat, 12> CUBE_EAST_VERTICES = {
     0.5, -0.5,  0.5,
     0.5, -0.5, -0.5,
     0.5,  0.5, -0.5,
     0.5,  0.5,  0.5,
};

static const std::array<GLfloat, 12> CUBE_WEST_VERTICES = {
    -0.5,  0.5,  0.5,
    -0.5,  0.5, -0.5,
    -0.5, -0.5, -0.5,
    -0.5, -0.5,  0.5,
};

static const std::array<GLfloat, 12> CUBE_FRONT_VERTICES = {
     0.5, -0.5,  -0.5,
    -0.5, -0.5,  -0.5,
    -0.5,  0.5,  -0.5,
     0.5,  0.5,  -0.5,
};

static const std::array<GLfloat, 12> CUBE_BACK_VERTICES = {
     0.5,  0.5,  0.5,
    -0.5,  0.5,  0.5,
    -0.5, -0.5,  0.5,
     0.5, -0.5,  0.5,
};

static const std::array<GLuint, 6> FACE_INDICES = {
    0, 1, 3,
    1, 2, 3,
};

static const std::array<std::array<int, 3>, 6> FACE_OFFSETS = {
     0,  1,  0,
     0, -1,  0,
     1,  0,  0,
    -1,  0,  0,
     0,  0,  1,
     0,  0, -1,
};

typedef std::array<GLfloat, 8> Tex_Coords;

static const Tex_Coords FACE_TEX_COORDS_UP = {
    1.0, 0.0,
    0.0, 0.0,
    0.0, 1.0,
    1.0, 1.0,
};

static const Tex_Coords FACE_TEX_COORDS_DN = {
    1.0, 1.0,
    0.0, 1.0,
    0.0, 0.0,
    1.0, 0.0,
};

static const std::array<std::array<GLfloat, 12>, 6> CUBE_VERTICES = {
    CUBE_NORTH_VERTICES,
    CUBE_SOUTH_VERTICES,
    CUBE_EAST_VERTICES,
    CUBE_WEST_VERTICES,
    CUBE_BACK_VERTICES,
    CUBE_FRONT_VERTICES,
};

static const std::array<Tex_Coords, 6> CUBE_TEX_COORDS = {
    FACE_TEX_COORDS_UP,
    FACE_TEX_COORDS_DN,
    FACE_TEX_COORDS_DN,
    FACE_TEX_COORDS_UP,
    FACE_TEX_COORDS_UP,
    FACE_TEX_COORDS_DN,
};

// clang-format on

#include <spdlog/spdlog.h>

Chunk_Mesh Chunk::build_mesh(const Block_Registry& registry) {
    std::vector<Block_Vertex> vertices;
    std::vector<GLuint> indices;

    for (size_t x = 0; x < DIM; x++) {
        for (size_t y = 0; y < DIM; y++) {
            for (size_t z = 0; z < DIM; z++) {
                const auto i = to_1d(x, y, z);
                const auto skeleton = registry.index_block_skeleton(blocks[i]);

                if (!skeleton.is_visible) continue;

                for (size_t current_face = 0; current_face < 6;
                     current_face++) {
                    const auto [x_face_offset, y_face_offset, z_face_offset] =
                        FACE_OFFSETS[current_face];

                    const int x_offset = x + x_face_offset,
                              y_offset = y + y_face_offset,
                              z_offset = z + z_face_offset;

                    if ((x_offset >= 0 && x_offset < DIM) &&
                        (y_offset >= 0 && y_offset < DIM) &&
                        (z_offset >= 0 && z_offset < DIM)) {
                        const int j = to_1d(x_offset, y_offset, z_offset);

                        const auto offset_skeleton =
                            registry.index_block_skeleton(blocks[j]);

                        if (offset_skeleton.is_visible) continue;
                    }

                    const auto face_vertices = CUBE_VERTICES[current_face];
                    const auto face_tex_coords = CUBE_TEX_COORDS[current_face];
                    for (size_t face_vertex = 0; face_vertex < 4;
                         face_vertex++) {
                        Block_Vertex vert;

                        vert.vertices = {face_vertices[face_vertex * 3 + 0] + x,
                                         face_vertices[face_vertex * 3 + 1] + y,
                                         face_vertices[face_vertex * 3 + 2] +
                                             z};
                        vert.tex_coord = {face_tex_coords[face_vertex * 2 + 0],
                                          face_tex_coords[face_vertex * 2 + 1]};
                        vert.tex_offset = skeleton.tex_offsets[current_face];
                        vertices.push_back(vert);
                    }

                    const auto index_offset =
                        indices.size() == 0 ? 0
                                            : indices[indices.size() - 1] + 1;
                    for (auto&& index : FACE_INDICES) {
                        indices.push_back(index + index_offset);
                    }
                }
            }
        }
    }

    return Chunk_Mesh(position, vertices, indices);
}

void Chunk_Mesh::render(const Shader& face_shader, const glm::mat4& projection,
                        const glm::mat4& view,
                        const Texture_2D_Array& texture_array) {
    face_shader.use();
    face_shader.set_mat4("projection", projection);
    face_shader.set_mat4("view", view);
    face_shader.set_mat4("model", model);

    texture_array.bind();

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, vertices.size() * 3, GL_UNSIGNED_INT, 0);
}
