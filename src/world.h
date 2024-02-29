#pragma once
#include "camera.h"
#include "chunk.h"

#define RENDER_DISTANCE 16

typedef struct {
    chunk_pos_t pos;

    block_vertex_t* vertices;
    GLuint* indices;
} render_result_t;

typedef struct {
    struct {
        chunk_pos_t key;
        chunk_t value;
    }* loaded_chunks;

    struct {
        chunk_pos_t key;
        chunk_mesh_t value;
    }* rendered_chunks;

    render_result_t* render_queue;

    block_registry_t block_registry;

    shader_t block_shader;
    texture_object_t block_textures;

    camera_t* camera;
} world_t;

void world_init(world_t* world, camera_t* camera);
void world_load_chunk_at(world_t* world, chunk_pos_t chunk_pos);
void world_load_chunks_around(world_t* world, int dim, chunk_pos_t chunk_pos);

void world_update(world_t* world);
void world_render(world_t* world, mat4 projection, mat4 view);
