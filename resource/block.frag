#version 330 core
out vec4 frag_color;

in vec2 tex_coord;
flat in uint tex_index;

uniform sampler2DArray sampler_unit;

void main()
{
    frag_color = texture(sampler_unit, vec3(tex_coord, tex_index));
}
