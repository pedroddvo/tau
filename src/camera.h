#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class camera {
  public:
    enum class movement { forward, backward, left, right };

    explicit camera(glm::vec3 pos) : m_pos(pos) { update_vectors(); }

    glm::mat4 view_matrix() const {
        return glm::lookAt(m_pos, m_pos + m_front, m_up);
    }

    void input_keyboard(movement m, float dt);
    void input_mouse(float xoffset, float yoffset);
    void input_scroll(float yoffset);

    glm::vec3 m_pos, m_right, m_up;
    glm::vec3 m_world_up{0.0f, 1.0f, 0.0f}, m_front{0.0f, 0.0f, -1.0f};

    float m_yaw = -90.0f, m_pitch = 0.0f, m_speed = 10.0f, m_sensitivity = 0.1f,
          m_zoom = 45.0f;

  private:
    void update_vectors();
};
