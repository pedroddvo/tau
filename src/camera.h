#pragma once
#include <cglm/cglm.h>

typedef enum {
    CAMERA_FORWARD,
    CAMERA_BACKWARD,
    CAMERA_LEFT,
    CAMERA_RIGHT,
} camera_movement_t;

typedef struct {
    vec3 position, front, up, right, world_up;

    float yaw, pitch, speed, sensitivity, zoom;
} camera_t;

camera_t camera_init(float x, float y, float z);
void camera_move(camera_t* camera, camera_movement_t movement, float dt);
void camera_mouse(camera_t* camera, float xoffset, float yoffset);

void camera_view_matrix(camera_t* camera, mat4 mat);
