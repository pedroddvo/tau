cmake_minimum_required(VERSION 3.27)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(OPTS -Wall -Werror -std=c++20 -fno-exceptions)

project(tau
    LANGUAGES CXX C)

add_executable(tau
	src/util.h
    src/resource.h src/resource.cpp

    src/shader.h src/shader.cpp
    src/camera.h src/camera.cpp
    src/texture.h src/texture.cpp

    src/chunk.h src/chunk.cpp
    src/world.h src/world.cpp

    src/state.h src/state.cpp
	src/main.cpp)

target_compile_options(tau PRIVATE ${OPTS})
target_link_options(tau PRIVATE ${OPTS})

include_directories(include)

find_package(spdlog CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

add_library(glad deps/glad.c)
add_library(stbi deps/stb_image.c)
target_link_libraries(tau PRIVATE spdlog::spdlog glfw glad glm::glm stbi)
