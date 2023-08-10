cmake_minimum_required(VERSION 3.0.0)
project(tau VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

add_executable(tau
  src/resource.hpp src/resource.cpp
  src/texture.hpp src/texture.cpp
  src/shader.hpp src/shader.cpp
  src/camera.hpp src/camera.cpp
  src/chunk.hpp src/chunk.cpp

  src/main.cpp
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

set(COMPILER_ARGS -Wall -Werror -fsanitize=address -Wno-missing-braces)

include_directories(include)

find_package(glfw3 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
add_library(glad src/glad.c)
add_library(stbi_image src/stb_image.c)

target_link_libraries(${PROJECT_NAME} PRIVATE stbi_image glfw spdlog::spdlog glad glm::glm)

target_compile_options(${PROJECT_NAME} PRIVATE ${COMPILER_ARGS})
target_link_options(${PROJECT_NAME} PRIVATE ${COMPILER_ARGS})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ../${PROJECT_NAME}
)
