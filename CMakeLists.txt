cmake_minimum_required(VERSION 3.27)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(OPTS -Wall -Werror -fsanitize=address -g -Wno-deprecated-declarations)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(OPTS -Wall -Werror -Wno-deprecated-declarations -O3)
endif()

project(tau LANGUAGES C)

add_executable(tau
    src/glad.c 
    src/stb_ds.h src/stb_ds.c
    src/stb_image.h src/stb_image.c
    src/FastNoiseLite.h src/FastNoiseLite.c
    src/log.h src/log.c
    
    src/util.h
    src/resource.h src/resource.c
    src/texture.h src/texture.c
    src/shader.h src/shader.c

    src/camera.h src/camera.c

    src/chunk.h src/chunk.c
    src/world.h src/world.c
    
    src/state.h src/state.c
    src/main.c)

target_link_options(tau PRIVATE ${OPTS})
target_compile_options(tau PRIVATE ${OPTS})
target_include_directories(tau PRIVATE include)

add_subdirectory(cglm)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(tau PRIVATE glfw cglm)
