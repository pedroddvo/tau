#pragma once
#include "shader.h"
#include <cglm/cglm.h>
#include <glad/glad.h>
#include <stdbool.h>

#define block_texture_all_faces(t)                                             \
    { t, t, t, t, t, t }

#define block_texture_same_sides(top, bot, side)                               \
    { side, side, top, bot, side, side }

typedef struct {
    bool is_visible;
    GLuint textures[6];
} block_t;

typedef struct {
    vec3 pos;
    GLuint texture_index, texture, face, ao;
} block_vertex_t;

typedef uint8_t block_instance_t;

#define CHUNK_DIM 16
#define CHUNK_SIZE (CHUNK_DIM * CHUNK_DIM * CHUNK_DIM)

#define CHUNK_EACH(x, y, z)                                                    \
    for (int y = 0; y < CHUNK_DIM; y++)                                        \
        for (int z = 0; z < CHUNK_DIM; z++)                                    \
            for (int x = 0; x < CHUNK_DIM; x++)

typedef struct {
    int x, y, z;
} chunk_pos_t;

typedef struct {
    block_instance_t blocks[CHUNK_SIZE];
    bool empty_layers[CHUNK_DIM];
    chunk_pos_t pos;
} chunk_t;

typedef struct {
    chunk_pos_t pos;
    size_t indices_count;
    GLuint vao, vbo, ebo;
} chunk_mesh_t;

typedef struct {
    block_t blocks[UINT8_MAX];
    size_t blocks_size;
} block_registry_t;

void block_registry_init(block_registry_t* dest);
block_instance_t block_register(block_registry_t* registry, block_t block);

void chunk_init(chunk_t* dest, chunk_pos_t pos, block_registry_t* registry,
                block_instance_t (*fill_func)(ivec3 block_pos));
block_instance_t chunk_index_block(const chunk_t* chunk, ivec3 block_pos);

chunk_mesh_t chunk_mesh_init(block_vertex_t* vertices, GLuint* indices,
                             chunk_pos_t pos);
void chunk_mesh_deinit(chunk_mesh_t* mesh);
void chunk_mesh_render(chunk_mesh_t* mesh, const shader_t* chunk_shader);

static inline int block_coord_clamp(int i) {
    return (CHUNK_DIM + (i % CHUNK_DIM)) % CHUNK_DIM;
}

static inline int chunk_coord_of_block_coord(int i) {
    return (i / CHUNK_DIM) + ((i % CHUNK_DIM) >> 31);
}

static inline chunk_pos_t chunk_pos_of_block_pos(ivec3 block_pos) {
    return (chunk_pos_t){chunk_coord_of_block_coord(block_pos[0]),
                         chunk_coord_of_block_coord(block_pos[1]),
                         chunk_coord_of_block_coord(block_pos[2])};
}

static inline chunk_pos_t chunk_pos_of_vec3(vec3 continuous_pos) {
    return (chunk_pos_t){chunk_coord_of_block_coord((int)continuous_pos[0]),
                         chunk_coord_of_block_coord((int)continuous_pos[1]),
                         chunk_coord_of_block_coord((int)continuous_pos[2])};
}
