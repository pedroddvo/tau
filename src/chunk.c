#include "chunk.h"
#include "stb_ds.h"
#include <assert.h>

void block_registry_init(block_registry_t* dest) { dest->blocks_size = 0; }

block_instance_t block_register(block_registry_t* registry, block_t block) {
    assert(registry->blocks_size + 1 < UINT8_MAX);
    registry->blocks[registry->blocks_size] = block;
    return registry->blocks_size++;
}

void chunk_init(chunk_t* dest, chunk_pos_t pos, block_registry_t* registry,
                block_instance_t (*fill_func)(ivec3 block_pos)) {
    dest->pos = pos;

    size_t i = 0;
    for (int y = 0; y < 16; y++) {
        bool is_empty = true;

        for (int z = 0; z < 16; z++) {
            for (int x = 0; x < 16; x++) {
                block_instance_t inst = fill_func(
                    (ivec3){x + pos.x * CHUNK_DIM, y + pos.y * CHUNK_DIM,
                            z + pos.z * CHUNK_DIM});
                block_t block = registry->blocks[inst];

                if (block.is_visible) is_empty = false;

                dest->blocks[i++] = inst;
            }
        }

        dest->empty_layers[y] = is_empty;
    }
}

block_instance_t chunk_index_block(const chunk_t* chunk, ivec3 block_pos) {
    return chunk
        ->blocks[(block_coord_clamp(block_pos[1]) * CHUNK_DIM * CHUNK_DIM) +
                 (block_coord_clamp(block_pos[2]) * CHUNK_DIM) +
                 (block_coord_clamp(block_pos[0]))];
}

chunk_mesh_t chunk_mesh_init(block_vertex_t* vertices, GLuint* indices,
                             chunk_pos_t pos) {
    chunk_mesh_t mesh;

    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);
    glGenBuffers(1, &mesh.ebo);

    glBindVertexArray(mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(block_vertex_t) * arrlen(vertices),
                 vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * arrlen(indices),
                 indices, GL_STATIC_DRAW);

    // pos
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(block_vertex_t),
                          (void*)0);
    glEnableVertexAttribArray(0);

    // index, texture, face, ao
    glVertexAttribIPointer(1, 4, GL_UNSIGNED_INT, sizeof(block_vertex_t),
                           (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    mesh.indices_count = arrlen(indices);
    mesh.pos = pos;
    return mesh;
}

void chunk_mesh_deinit(chunk_mesh_t* mesh) {
    glDeleteVertexArrays(1, &mesh->vao);
    glDeleteBuffers(1, &mesh->vbo);
    glDeleteBuffers(1, &mesh->ebo);
}

void chunk_mesh_render(chunk_mesh_t* mesh, const shader_t* chunk_shader) {
    mat4 model = GLM_MAT4_IDENTITY_INIT;
    glm_translate(model,
                  (vec3){mesh->pos.x * CHUNK_DIM, mesh->pos.y * CHUNK_DIM,
                         mesh->pos.z * CHUNK_DIM});
    shader_set_mat4(chunk_shader, "model", model);

    glBindVertexArray(mesh->vao);
    glDrawElements(GL_TRIANGLES, mesh->indices_count, GL_UNSIGNED_INT, 0);
}
