#if !defined(CAMERA_H)
#define CAMERA_H

#include <glm/glm.hpp>

enum class Camera_Movement { Forward, Backward, Left, Right };

class Camera {
  public:
    static constexpr float YAW = -90.0, PITCH = 0.0, SPEED = 8.5,
                           SENSITIVITY = 0.1, ZOOM = 45.0;

    glm::vec3 position, front, up, right, world_up;
    float yaw, pitch, movement_speed, mouse_sensitivity, zoom;

    Camera(glm::vec3 position = glm::vec3(0.0, 0.0, 0.0),
           glm::vec3 up = glm::vec3(0.0, 1.0, 0.0), float yaw = YAW,
           float pitch = PITCH)
        : position(position), front(glm::vec3(0.0, 0.0, -1.0)), world_up(up),
          yaw(yaw), pitch(pitch), movement_speed(SPEED),
          mouse_sensitivity(SENSITIVITY), zoom(ZOOM) {
        update_camera_vectors();
    }

    glm::mat4 get_view_matrix() const;

    void process_keyboard(Camera_Movement direction, float dt);
    void process_mouse_movement(float xoffset, float yoffset,
                                bool constrain_pitch = true);

  private:
    void update_camera_vectors();
};

#endif // CAMERA_H
