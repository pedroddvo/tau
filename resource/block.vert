#version 330 core

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec2 in_tex_coord;
layout (location = 2) in uint in_tex_index;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 tex_coord;
flat out uint tex_index;

void main() {
     tex_coord = in_tex_coord;
     tex_index = in_tex_index;
     gl_Position = projection * view * model * vec4(in_pos, 1.0f);
}
