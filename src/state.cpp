#include "state.h"

void state::render() {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 projection = glm::perspective(
        glm::radians(m_camera.m_zoom),
        (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 1000.0f);
    glm::mat4 view = m_camera.view_matrix();

    chunk::pos current_chunk =
        chunk::of_block({m_camera.m_pos.x, 0, m_camera.m_pos.z});

    m_world.load_chunks_around(RENDER_DISTANCE, current_chunk);

    m_world.render(projection, view);

    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

state::state(GLFWwindow* window) : m_window(window), m_world(m_camera) {
    glEnable(GL_DEPTH_TEST);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetWindowUserPointer(window, this);

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glfwSetFramebufferSizeCallback(m_window,
                                   [](auto window, int width, int height) {
                                       glViewport(0, 0, width, height);
                                   });

    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetScrollCallback(window, scroll_callback);
}

void state::input() {
    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(m_window, true);

    if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
        m_camera.input_keyboard(camera::movement::forward, m_delta_time);
    if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
        m_camera.input_keyboard(camera::movement::backward, m_delta_time);
    if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
        m_camera.input_keyboard(camera::movement::left, m_delta_time);
    if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
        m_camera.input_keyboard(camera::movement::right, m_delta_time);
}

void state::cursor_pos_callback(GLFWwindow* window, double xposi,
                                double yposi) {
    float xpos = static_cast<float>(xposi), ypos = static_cast<float>(yposi);
    static bool first_mouse = true;
    static float lastx = state::WINDOW_HEIGHT / 2.0f,
                 lasty = state::WINDOW_WIDTH / 2.0f;

    if (first_mouse) {
        lastx = xpos;
        lasty = ypos;
        first_mouse = false;
    }

    float xoffset = xpos - lastx, yoffset = lasty - ypos;
    lastx = xpos;
    lasty = ypos;

    state* s = reinterpret_cast<state*>(glfwGetWindowUserPointer(window));
    s->m_camera.input_mouse(xoffset, yoffset);
}

void state::scroll_callback(GLFWwindow* window, double xoffset,
                            double yoffset) {
    state* s = reinterpret_cast<state*>(glfwGetWindowUserPointer(window));
    s->m_camera.input_scroll(yoffset);
}
