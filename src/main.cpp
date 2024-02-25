#include "state.h"
#include "texture.h"
#include "util.h"
#include "resource.h"

int main(int argc, char const* argv[]) {
    spdlog::set_level(spdlog::level::debug);

    assert_fatal(glfwInit(), "failed to initialize glfw");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(
        state::WINDOW_WIDTH, state::WINDOW_HEIGHT, "tau", NULL, NULL);
    assert_fatal(window != NULL, "failed to create window");
    glfwMakeContextCurrent(window);

    assert_fatal(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress),
                 "failed to initialize glad/opengl");

    state game_state(window);
    float delta_time = 0.0f, last_frame = 0.0f;

    while (!game_state.should_stop()) {
        float current_frame = static_cast<float>(glfwGetTime());
        delta_time = current_frame - last_frame;
        last_frame = current_frame;

        game_state.m_delta_time = delta_time;
        game_state.input();
        game_state.render();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
