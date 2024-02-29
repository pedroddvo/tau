#include "camera.h"

static void update_vectors(camera_t* camera) {
    vec3 front = {0};
    front[0] = cos(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch));
    front[1] = sin(glm_rad(camera->pitch));
    front[2] = sin(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch));

    glm_normalize_to(front, camera->front);
    glm_vec3_crossn(camera->front, camera->world_up, camera->right);
    glm_vec3_crossn(camera->right, camera->front, camera->up);
}

camera_t camera_init(float x, float y, float z) {
    camera_t camera = (camera_t){
        .yaw = -90.0f,
        .pitch = 0.0f,
        .speed = 10.5f,
        .sensitivity = 0.1f,
        .zoom = 45.0f,
    };

    glm_vec3_copy((vec3){x, y, z}, camera.position);
    glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, camera.world_up);
    glm_vec3_copy((vec3){0.0f, 0.0f, -1.0f}, camera.front);

    update_vectors(&camera);
    return camera;
}

void camera_move(camera_t* camera, camera_movement_t movement, float dt) {
    float velocity = camera->speed * dt;
    switch (movement) { // clang-format off
    case CAMERA_FORWARD:  glm_vec3_muladds(camera->front, velocity, camera->position); break;
    case CAMERA_BACKWARD: glm_vec3_mulsubs(camera->front, velocity, camera->position); break;
    case CAMERA_LEFT:     glm_vec3_mulsubs(camera->right, velocity, camera->position); break;
    case CAMERA_RIGHT:    glm_vec3_muladds(camera->right, velocity, camera->position); break;
    } // clang-format on
}

void camera_view_matrix(camera_t* camera, mat4 mat) {
    vec3 center = {0};
    glm_vec3_add(camera->position, camera->front, center);

    glm_lookat(camera->position, center, camera->up, mat);
}

void camera_mouse(camera_t* camera, float xoffset, float yoffset) {
    xoffset *= camera->sensitivity;
    yoffset *= camera->sensitivity;

    camera->yaw += xoffset;
    camera->pitch += yoffset;

    if (camera->pitch > 89.0f) camera->pitch = 89.0f;
    else if (camera->pitch < -89.0f)
        camera->pitch = -89.0f;

    update_vectors(camera);
}
