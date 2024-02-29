#version 400 core
layout (location = 0) in vec3 in_pos;
layout (location = 1) in uvec4 in_texture_data_face_ao;

uniform mat4 projection, view, model;

out vec4 v_frag_pos;
out vec3 v_uv;
out vec3 v_normal;
out float v_ao;

const vec2 uv_coords[4] = vec2[](
    vec2(0.0f, 1.0f),
    vec2(0.0f, 0.0f),
    vec2(1.0f, 0.0f),
    vec2(1.0f, 1.0f)
);

const vec3 normals[6] = vec3[](
    vec3( 0.0f,  0.0f,  1.0f),
    vec3( 0.0f,  0.0f, -1.0f),
    vec3( 0.0f,  1.0f,  0.0f),
    vec3( 0.0f, -1.0f,  0.0f),
    vec3( 1.0f,  0.0f,  0.0f),
    vec3(-1.0f,  0.0f,  0.0f)
);


void main() {
    v_uv = vec3(uv_coords[in_texture_data_face_ao[0]].xy, in_texture_data_face_ao[1]);
    v_normal = normals[in_texture_data_face_ao[2]];
    v_frag_pos = model * vec4(in_pos, 1.0f);
    v_ao = in_texture_data_face_ao[3];
    gl_Position = projection * view * model * vec4(in_pos, 1.0f);
}
