#include "camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

void Camera::update_camera_vectors() {
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    front = glm::normalize(front);

    right = glm::normalize(glm::cross(front, world_up));
    up = glm::normalize(glm::cross(right, front));
}

glm::mat4 Camera::get_view_matrix() const {
    return glm::lookAt(position, position + front, up);
}

void Camera::process_keyboard(Camera_Movement direction, float dt) {
    float velocity = movement_speed * dt;
    if (direction == Camera_Movement::Forward) position += front * velocity;
    if (direction == Camera_Movement::Backward) position -= front * velocity;
    if (direction == Camera_Movement::Right) position += right * velocity;
    if (direction == Camera_Movement::Left) position -= right * velocity;
}

void Camera::process_mouse_movement(float xoffset, float yoffset,
                                    bool constrain_pitch) {
    xoffset *= mouse_sensitivity;
    yoffset *= mouse_sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (constrain_pitch) {
        if (pitch > 89.0) pitch = 89.0;
        if (pitch < -89.0) pitch = -89.0;
    }

    update_camera_vectors();
}
