#include "state.h"
#include "log.h"
#include "resource.h"
#include "util.h"

state_t* game_state = NULL;

void mouse_callback(GLFWwindow* window, double xpos_in, double ypos_in);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void game_init() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    game_state = malloc(sizeof(state_t));
    assert_fatal(game_state != NULL, "failed to initialize game state");

    game_state->window =
        glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "tau", NULL, NULL);
    assert_fatal(game_state->window != NULL, "failed to initialize glfw");
    glfwMakeContextCurrent(game_state->window);

    glfwSetInputMode(game_state->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetCursorPosCallback(game_state->window, mouse_callback);
    glfwSetScrollCallback(game_state->window, scroll_callback);

    assert_fatal(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress),
                 "failed to initialize opengl/glad");

    game_state->shader = shader_from_files("tri.vert", "tri.frag");

    game_state->camera = camera_init(0.0f, 0.0f, 0.0f);
    game_state->delta_time = 0.0f;

    world_init(&game_state->world, &game_state->camera);

    glEnable(GL_DEPTH_TEST);
}

void game_deinit() {
    glfwDestroyWindow(game_state->window);
    glfwTerminate();

    free(game_state);
}

bool game_should_stop() { return glfwWindowShouldClose(game_state->window); }

static void handle_keyboard_input() {
    if (glfwGetKey(game_state->window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(game_state->window, true);

    if (glfwGetKey(game_state->window, GLFW_KEY_W) == GLFW_PRESS)
        camera_move(&game_state->camera, CAMERA_FORWARD,
                    game_state->delta_time);
    if (glfwGetKey(game_state->window, GLFW_KEY_S) == GLFW_PRESS)
        camera_move(&game_state->camera, CAMERA_BACKWARD,
                    game_state->delta_time);
    if (glfwGetKey(game_state->window, GLFW_KEY_A) == GLFW_PRESS)
        camera_move(&game_state->camera, CAMERA_LEFT, game_state->delta_time);
    if (glfwGetKey(game_state->window, GLFW_KEY_D) == GLFW_PRESS)
        camera_move(&game_state->camera, CAMERA_RIGHT, game_state->delta_time);
}

static void render() {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader_use(&game_state->shader);

    mat4 projection = {0};
    glm_perspective(glm_rad(game_state->camera.zoom),
                    (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 1000.0f,
                    projection);
    shader_set_mat4(&game_state->shader, "projection", projection);

    mat4 view = {0};
    camera_view_matrix(&game_state->camera, view);
    shader_set_mat4(&game_state->shader, "view", view);

    world_render(&game_state->world, projection, view);
}

static void update() {
    world_update(&game_state->world);
}

void game_tick(float dt) {
    game_state->delta_time = dt;

    glfwPollEvents();
    handle_keyboard_input();

    update();
    render();
    glfwSwapBuffers(game_state->window);
}

void mouse_callback(GLFWwindow* window, double xpos_in, double ypos_in) {
    float xpos = (float)xpos_in, ypos = (float)ypos_in;

    static bool first_mouse = false;
    static float last_x = (float)WINDOW_WIDTH / 2.0f,
                 last_y = (float)WINDOW_HEIGHT / 2.0f;

    if (first_mouse) {
        last_x = xpos;
        last_y = ypos;
        first_mouse = false;
    }

    float xoffset = xpos - last_x, yoffset = last_y - ypos;

    last_x = xpos;
    last_y = ypos;

    camera_mouse(&game_state->camera, xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    game_state->camera.speed += yoffset;
    if (game_state->camera.speed < 2.0f) game_state->camera.speed = 2.0f;
}
