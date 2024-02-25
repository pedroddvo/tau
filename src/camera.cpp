#include "camera.h"

void camera::update_vectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.y = sin(glm::radians(m_pitch));
    front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));

    m_front = glm::normalize(front);
    m_right = glm::normalize(glm::cross(m_front, m_world_up));
    m_up = glm::normalize(glm::cross(m_right, m_front));
}

void camera::input_keyboard(movement m, float dt) {
    float velocity = m_speed * dt;

    switch (m) { // clang-format off
    case movement::forward:  m_pos += m_front * velocity; break;
    case movement::backward: m_pos -= m_front * velocity; break;
    case movement::left:     m_pos -= m_right * velocity; break;
    case movement::right:    m_pos += m_right * velocity; break;
    } // clang-format on
}

void camera::input_mouse(float xoffset, float yoffset) {
    xoffset *= m_sensitivity;
    yoffset *= m_sensitivity;

    m_yaw += xoffset;
    m_pitch += yoffset;

    if (m_pitch > 89.0f) m_pitch = 89.0f;
    else if (m_pitch < -89.0f)
        m_pitch = -89.0f;

    update_vectors();
}

void camera::input_scroll(float yoffset) {
    m_speed += yoffset;

    if (m_speed < 2.0f) m_speed = 2.0f;
}
