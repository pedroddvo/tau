#pragma once
#include "camera.h"
#include "shader.h"
#include "world.h"
#include <glad/glad.h>

#include <GLFW/glfw3.h>

class state {
  public:
    state(GLFWwindow* window);

    bool should_stop() const { return glfwWindowShouldClose(m_window); }

    void render();
    void input();

    static constexpr int WINDOW_WIDTH = 800, WINDOW_HEIGHT = 600;

    float m_delta_time = 0.0f;

  private:
    GLFWwindow* m_window;

    camera m_camera{{0.0f, 0.0f, 0.0f}};

    world m_world;

    static void cursor_pos_callback(GLFWwindow* window, double xposi,
                                    double yposi);
    static void scroll_callback(GLFWwindow* window, double xoffset,
                                double yoffset);
};
