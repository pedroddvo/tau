#include "world.h"
#include "FastNoiseLite.h"
#include "log.h"
#include "stb_ds.h"
#include <assert.h>
#include <pthread.h>

block_instance_t air = 0, dirt = 0, grass = 0;
fnl_state world_noise;

void world_init(world_t* world, camera_t* camera) {
    world_noise = fnlCreateState();
    world_noise.noise_type = FNL_NOISE_PERLIN;

    world->camera = camera;

    world->loaded_chunks = NULL;
    world->rendered_chunks = NULL;
    world->render_queue = NULL;

    world->block_shader = shader_from_files("tri.vert", "tri.frag");

    texture_array_t textures = texture_array_init(16, 16, STBI_rgb_alpha);
    texture_instance_t bad_texture =
        texture_array_add(&textures, "blocks/missing_tile.png");
    texture_instance_t dirt_t = texture_array_add(&textures, "blocks/dirt.png");
    texture_instance_t grass_top_t =
        texture_array_add(&textures, "blocks/grass_carried.png");
    texture_instance_t grass_side_t =
        texture_array_add(&textures, "blocks/grass_side_carried.png");
    world->block_textures = texture_array_to_object(&textures);
    texture_array_deinit(&textures);

    shader_set_texture(&world->block_shader, "u_textures",
                       &world->block_textures);

    block_registry_init(&world->block_registry);
    air = block_register(&world->block_registry,
                         (block_t){
                             .is_visible = false,
                             .textures = block_texture_all_faces(bad_texture),
                         });
    dirt = block_register(&world->block_registry,
                          (block_t){
                              .is_visible = true,
                              .textures = block_texture_all_faces(dirt_t),
                          });
    grass = block_register(&world->block_registry,
                           (block_t){
                               .is_visible = true,
                               .textures = block_texture_same_sides(
                                   grass_top_t, dirt_t, grass_side_t),
                           });
}

static bool is_air(ivec3 block_pos) {
    const int mountain_height = CHUNK_DIM * 1.0f;
    float noise =
        fnlGetNoise2D(&world_noise, block_pos[0] * 2.0f, block_pos[2] * 2.0f);
    float terrain_height = fabs(noise) * mountain_height - CHUNK_DIM * 3.0f;
    return block_pos[1] > terrain_height;
}

static block_instance_t generate_chunk(ivec3 block_pos) {
    return is_air(block_pos) ? air
           : is_air((ivec3){block_pos[0], block_pos[1] + 1, block_pos[2]})
               ? grass
               : dirt;
}

void world_load_chunk_at(world_t* world, chunk_pos_t chunk_pos) {
    if (hmgeti(world->loaded_chunks, chunk_pos) != -1) return;

    chunk_t chunk = {0};
    chunk_init(&chunk, chunk_pos, &world->block_registry, generate_chunk);
    hmput(world->loaded_chunks, chunk_pos, chunk);
}

void world_load_chunks_around(world_t* world, int dim, chunk_pos_t chunk_pos) {
    for (int x = -dim; x < dim; x++) {
        for (int y = -4; y < 4; y++) {
            for (int z = -dim; z < dim; z++) {
                world_load_chunk_at(world, (chunk_pos_t){chunk_pos.x + x,
                                                         chunk_pos.y + y,
                                                         chunk_pos.z + z});
            }
        }
    }
}

static bool block_visible_at(world_t* world, ivec3 block_pos);
static render_result_t render_chunk_at(world_t* world, chunk_pos_t pos);

static bool is_updating = false, is_rendering = false;
static pthread_t update_thread = NULL;

static void* update_async(void* worldp) {
    world_t* world = (world_t*)worldp;
    chunk_pos_t camera_chunk = chunk_pos_of_vec3(world->camera->position);

    world_load_chunks_around(world, RENDER_DISTANCE + 1,
                             (chunk_pos_t){camera_chunk.x, 0, camera_chunk.z});

    clock_t start = clock();
    size_t rendered = 0;

    for (int x = -RENDER_DISTANCE; x < RENDER_DISTANCE; x++) {
        for (int y = -4; y < 4; y++) {
            for (int z = -RENDER_DISTANCE; z < RENDER_DISTANCE; z++) {
                chunk_pos_t chunk_pos = {camera_chunk.x + x, y,
                                         camera_chunk.z + z};

                while (is_rendering) {
                }
                ptrdiff_t mesh_i = hmgeti(world->rendered_chunks, chunk_pos);
                if (mesh_i != -1)
                    continue; // already rendered and loaded - skip

                render_result_t res = render_chunk_at(world, chunk_pos);
                rendered++;

                while (is_rendering) {
                }
                arrpush(world->render_queue, res);
            }
        }
    }

    clock_t end = clock();
    double elapsed_time = (end - start) / (double)CLOCKS_PER_SEC;

    log_info("rendered %d chunks: avg %fms", rendered,
             elapsed_time * 1000 / rendered);

    is_updating = false;
    pthread_exit(NULL);
}

void world_update(world_t* world) {
    static chunk_pos_t last_camera_chunk = {-1};
    chunk_pos_t current_camera_chunk =
        chunk_pos_of_vec3(world->camera->position);

    if (!is_updating && (current_camera_chunk.x != last_camera_chunk.x ||
                         current_camera_chunk.z != last_camera_chunk.z)) {
        is_updating = true;
        last_camera_chunk = current_camera_chunk;

        if (update_thread != NULL) pthread_join(update_thread, NULL);

        pthread_create(&update_thread, NULL, update_async, world);
    }
}

static bool out_of_render_distance(world_t* world, chunk_pos_t pos) {
    const int boundx = RENDER_DISTANCE + CHUNK_DIM / 2 +
                       (int)world->camera->position[0] / CHUNK_DIM;
    const int boundz = RENDER_DISTANCE + CHUNK_DIM / 2 +
                       (int)world->camera->position[2] / CHUNK_DIM;
    const int boundxn = -RENDER_DISTANCE - CHUNK_DIM / 2 +
                        (int)world->camera->position[0] / CHUNK_DIM;
    const int boundzn = -RENDER_DISTANCE - CHUNK_DIM / 2 +
                        (int)world->camera->position[2] / CHUNK_DIM;
    return (pos.x > boundx || pos.x < boundxn) ||
           (pos.z > boundz || pos.z < boundzn);
}

void world_render(world_t* world, mat4 projection, mat4 view) {
    shader_use(&world->block_shader);
    shader_set_mat4(&world->block_shader, "projection", projection);
    shader_set_mat4(&world->block_shader, "view", view);

    if (arrlen(world->render_queue) > 0) {
        is_rendering = true;
        for (size_t i = 0; i < arrlen(world->render_queue); i++) {
            render_result_t result = world->render_queue[i];
            chunk_mesh_t mesh =
                chunk_mesh_init(result.vertices, result.indices, result.pos);
            hmput(world->rendered_chunks, mesh.pos, mesh);

            arrfree(result.vertices);
            arrfree(result.indices);
        }

        arrfree(world->render_queue);
        is_rendering = false;
    }

    // (void)out_of_render_distance;
    if (!is_updating) {
        for (size_t i = 0; i < hmlen(world->rendered_chunks); i++) {
            const chunk_pos_t pos = world->rendered_chunks[i].key;
            if (out_of_render_distance(world, pos)) {
                chunk_mesh_deinit(&world->rendered_chunks[i].value);
                hmdel(world->rendered_chunks, pos);
            }
        }
    }

    for (int x = -RENDER_DISTANCE; x < RENDER_DISTANCE; x++) {
        for (int y = -4; y < 4; y++) {
            for (int z = -RENDER_DISTANCE; z < RENDER_DISTANCE; z++) {
                chunk_pos_t chunk_pos = {
                    x + world->camera->position[0] / CHUNK_DIM, y,
                    z + world->camera->position[2] / CHUNK_DIM};
                ptrdiff_t mesh_i = hmgeti(world->rendered_chunks, chunk_pos);

                chunk_mesh_t* mesh = NULL;
                if (mesh_i == -1) {
                    continue;
                } else
                    mesh = &world->rendered_chunks[mesh_i].value;

                chunk_mesh_render(mesh, &world->block_shader);
            }
        }
    }
}

// clang-format off
#define FACE_FRONT { \
    { 0.5f,  0.5f, -0.5f}, \
    { 0.5f, -0.5f, -0.5f}, \
    {-0.5f, -0.5f, -0.5f}, \
    {-0.5f,  0.5f, -0.5f}, \
}

#define FACE_BACK { \
    {-0.5f,  0.5f,  0.5f}, \
    {-0.5f, -0.5f,  0.5f}, \
    { 0.5f, -0.5f,  0.5f}, \
    { 0.5f,  0.5f,  0.5f}, \
}

#define FACE_UP { \
    { 0.5f,  0.5f,  0.5f}, \
    { 0.5f,  0.5f, -0.5f}, \
    {-0.5f,  0.5f, -0.5f}, \
    {-0.5f,  0.5f,  0.5f}, \
}

#define FACE_DOWN { \
    {-0.5f, -0.5f,  0.5f}, \
    {-0.5f, -0.5f, -0.5f}, \
    { 0.5f, -0.5f, -0.5f}, \
    { 0.5f, -0.5f,  0.5f}, \
}

#define FACE_LEFT { \
    {-0.5f,  0.5f, -0.5f}, \
    {-0.5f, -0.5f, -0.5f}, \
    {-0.5f, -0.5f,  0.5f}, \
    {-0.5f,  0.5f,  0.5f}, \
}

#define FACE_RIGHT { \
    { 0.5f,  0.5f,  0.5f}, \
    { 0.5f, -0.5f,  0.5f}, \
    { 0.5f, -0.5f, -0.5f}, \
    { 0.5f,  0.5f, -0.5f}, \
}

typedef vec3 face_vertices_t[4];

static const face_vertices_t FACE_VERTICES[6] = {
    FACE_BACK, FACE_FRONT,
    FACE_UP, FACE_DOWN,
    FACE_RIGHT, FACE_LEFT,
};

static const vec3 FACES_OFFSETS[6] = {
    {  0.0f,  0.0f,  1.0f}, {  0.0f,  0.0f, -1.0f},
    {  0.0f,  1.0f,  0.0f}, {  0.0f, -1.0f,  0.0f},
    {  1.0f,  0.0f,  0.0f}, { -1.0f,  0.0f,  0.0f},
};

static const GLuint INDICES[6] = {0, 1, 2, 2, 3, 0};

// clang-format on

static bool block_visible_at(world_t* world, ivec3 block_pos) {
    chunk_pos_t belonging_chunk = chunk_pos_of_block_pos(block_pos);

    ptrdiff_t chunk = hmgeti(world->loaded_chunks, belonging_chunk);
    if (chunk == -1) return false;

    block_instance_t inst =
        chunk_index_block(&world->loaded_chunks[chunk].value, block_pos);
    return world->block_registry.blocks[inst].is_visible;
}

static render_result_t render_chunk_at(world_t* world, chunk_pos_t pos) {
    render_result_t result = (render_result_t){
        .pos = pos,
        .indices = NULL,
        .vertices = NULL,
    };

    ptrdiff_t current_chunk_i = hmgeti(world->loaded_chunks, pos);
    assert(current_chunk_i != -1);
    const chunk_t* current_chunk = &world->loaded_chunks[current_chunk_i].value;

    size_t indices_offset = 0;
    for (int y = 0; y < 16; y++) {
        if (current_chunk->empty_layers[y]) continue;
        for (int z = 0; z < 16; z++) {
            for (int x = 0; x < 16; x++) {
                ivec3 block_pos = {pos.x * CHUNK_DIM + x, pos.y * CHUNK_DIM + y,
                                   pos.z * CHUNK_DIM + z};
                block_instance_t block_inst =
                    chunk_index_block(current_chunk, block_pos);
                const block_t* this_block =
                    &world->block_registry.blocks[block_inst];
                if (!this_block->is_visible) continue;

                for (unsigned face = 0; face < 6; face++) {
                    ivec3 adjacent_block_pos = {
                        block_pos[0] + FACES_OFFSETS[face][0],
                        block_pos[1] + FACES_OFFSETS[face][1],
                        block_pos[2] + FACES_OFFSETS[face][2]};

                    if (block_visible_at(world, adjacent_block_pos)) continue;

                    for (unsigned i = 0; i < 4; i++) {
                        block_vertex_t block_vertex = {0};
                        glm_vec3_add((vec3){x, y, z},
                                     (float*)FACE_VERTICES[face][i],
                                     block_vertex.pos);
                        block_vertex.texture = this_block->textures[face];
                        block_vertex.texture_index = i;
                        block_vertex.face = face;

                        ivec3 side1 = {
                            block_pos[0] +
                                (int)(FACE_VERTICES[face][i][0] * 2.0),
                            block_pos[1] +
                                (int)(FACE_VERTICES[face][i][1] * 2.0),
                            block_pos[2],
                        };
                        ivec3 side2 = {
                            block_pos[0],
                            block_pos[1] +
                                (int)(FACE_VERTICES[face][i][1] * 2.0),
                            block_pos[2] +
                                (int)(FACE_VERTICES[face][i][2] * 2.0),
                        };
                        ivec3 corner = {
                            block_pos[0] +
                                (int)(FACE_VERTICES[face][i][0] * 2.0),
                            block_pos[1] +
                                (int)(FACE_VERTICES[face][i][1] * 2.0),
                            block_pos[2] +
                                (int)(FACE_VERTICES[face][i][2] * 2.0),
                        };

                        bool bside1 = block_visible_at(world, side1),
                             bside2 = block_visible_at(world, side2),
                             bcorner = block_visible_at(world, corner);

                        block_vertex.ao = bside1 && bside2
                                              ? 0
                                              : 3 - (bside1 + bside2 + bcorner);

                        arrpush(result.vertices, block_vertex);
                    }

                    for (unsigned i = 0; i < 6; i++) {
                        arrpush(result.indices, INDICES[i] + indices_offset);
                    }
                    indices_offset += 4;
                }
            }
        }
    }
    return result;
}
