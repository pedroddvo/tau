#pragma once
#define CGLM_DEFINE_PRINTS

#include "camera.h"
#include "shader.h"
#include "world.h"
#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <stdbool.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

typedef struct {
    GLFWwindow* window;
    camera_t camera;
    shader_t shader;
    float delta_time;
    world_t world;
} state_t;

extern state_t* game_state;

void game_init();
void game_deinit();
void game_tick(float dt);

bool game_should_stop();
