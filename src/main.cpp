#include <glad/glad.h>

#include "camera.hpp"
#include "chunk.hpp"
#include "shader.hpp"
#include <GLFW/glfw3.h>
#include <PerlinNoise.hpp>
#include <cstdlib>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

const int WINDOW_WIDTH = 800, WINDOW_HEIGHT = 600;

Camera camera = Camera();

void process_input(GLFWwindow* window, Camera& camera, float dt) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.process_keyboard(Camera_Movement::Forward, dt);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.process_keyboard(Camera_Movement::Backward, dt);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.process_keyboard(Camera_Movement::Left, dt);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.process_keyboard(Camera_Movement::Right, dt);
}

int main(int argc, char const* argv[]) {
	auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	auto basic_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs.txt");
	std::vector<spdlog::sink_ptr> sinks{console_sink, basic_sink};
	auto logger = std::make_shared<spdlog::logger>("main", sinks.begin(), sinks.end());

	spdlog::register_logger(logger);

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    auto window =
        glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "tau", NULL, NULL);
    if (window == NULL) {
        logger->error("failed to create GLFW window");
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        logger->error("failed to initialize GLAD");
        return -1;
    }

    glViewport(0, 0, WINDOW_WIDTH * 2, WINDOW_HEIGHT * 2);

    glfwSetFramebufferSizeCallback(
        window, [](GLFWwindow* window, int width, int height) {
            glViewport(0, 0, width, height);
        });

    glfwSetCursorPosCallback(
        window, [](GLFWwindow* window, double xpos_in, double ypos_in) {
            float xpos = static_cast<float>(xpos_in);
            float ypos = static_cast<float>(ypos_in);

            static bool first_mouse = true;
            static float lastx = WINDOW_WIDTH / 2, lasty = WINDOW_HEIGHT / 2;

            if (first_mouse) {
                lastx = xpos;
                lasty = ypos;
                first_mouse = false;
            }

            float xoffset = xpos - lastx;
            float yoffset = lasty - ypos;

            lastx = xpos;
            lasty = ypos;

            camera.process_mouse_movement(xoffset, yoffset);
        });

    glEnable(GL_DEPTH_TEST);

    float dt = 0, last_frame = 0;

    auto texture_registry = Texture_Registry();

    const auto texture_grass_top =
        texture_registry.load_texture_from_file("block/grass_carried.png");
    const auto texture_grass_side =
        texture_registry.load_texture_from_file("block/grass_side_carried.png");
    const auto texture_dirt =
        texture_registry.load_texture_from_file("block/dirt.png");
    const auto texture_cobblestone =
        texture_registry.load_texture_from_file("block/cobblestone.png");

    auto texture_array = texture_registry.build_texture_2d_array();

    auto air_block_skel = Block_Skeleton();
    air_block_skel.is_visible = false;

    auto grass_block_skel = Block_Skeleton();
    grass_block_skel.tex_offsets = {texture_grass_top,  texture_dirt,
                                    texture_grass_side, texture_grass_side,
                                    texture_grass_side, texture_grass_side};

    auto cobblestone_block_skel = Block_Skeleton();
    cobblestone_block_skel.tex_offsets.fill(texture_cobblestone);

    auto dirt_block_skel = Block_Skeleton();
    dirt_block_skel.tex_offsets.fill(texture_dirt);

    auto block_registry = Block_Registry();
    auto air_block = block_registry.register_block_skeleton(air_block_skel);
    auto grass_block = block_registry.register_block_skeleton(grass_block_skel);
    auto cobblestone_block =
        block_registry.register_block_skeleton(cobblestone_block_skel);
    auto dirt_block = block_registry.register_block_skeleton(dirt_block_skel);
    (void)grass_block;
	(void)dirt_block;

    const siv::PerlinNoise::seed_type seed = 0u;
    const siv::PerlinNoise noise{seed};

    std::vector<Chunk_Mesh> chunk_meshes;
    for (size_t x = 0; x < 8; x++) {
        for (size_t y = 0; y < 8; y++) {
			for (size_t z = 0; z < 8; z++) {
				auto chunk =
					Chunk(glm::vec3(x * 16, y * 16, z * 16), [=](glm::vec3 pos) {
						const float water_level = 0.5;

						const auto pos_norm = pos * (1.0f / 64.0f);
						const auto i = noise.octave2D(pos_norm.x, pos_norm.z, 2) + water_level;

						const auto land_offset = pos_norm.y * 4.0;

						if (i >= land_offset - 0.035 && i < land_offset + 0.035) return grass_block;
						else if (land_offset < i * 0.5) return cobblestone_block;
						else if (land_offset < i) return dirt_block;
						else return air_block;
					});

				chunk_meshes.push_back(chunk.build_mesh(block_registry));
			}
        }
    }

    auto block_shader = Shader::from_files("block.vert", "block.frag");

    while (!glfwWindowShouldClose(window)) {
        float current_frame = static_cast<float>(glfwGetTime());
        dt = current_frame - last_frame;
        last_frame = current_frame;

        process_input(window, camera, dt);

        glClearColor(0.2, 0.3, 0.3, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(
            glm::radians(camera.zoom),
            (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);

        const auto view = camera.get_view_matrix();
        for (auto&& chunk_mesh : chunk_meshes) {
            chunk_mesh.render(*block_shader, projection, view, texture_array);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}
