#version 400 core
out vec4 frag_color;

in vec4 v_frag_pos;
in vec3 v_uv;
in vec3 v_normal;
in float v_ao;

uniform sampler2DArray u_textures;

const float near = 0.1f;
const float far = 100.0f;

float linearize_depth(float depth) {
    float z = depth * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main() {
    float ambient_strength = 0.7f;
    const vec3 light_color = vec3(1.0f, 1.0f, 1.0f);

    vec3 norm = normalize(v_normal);
    vec3 light_dir = normalize(vec3(0.0f, 100.0f, 0.0f) - v_frag_pos.xyz);

    float diff = max(dot(norm, light_dir), 0.0f);
    vec3 diffuse = diff * light_color * 0.6f;

    vec3 ambient = ambient_strength * light_color;
    
    float depth = linearize_depth(gl_FragCoord.z);
    vec4 fog_color = vec4(0.2f, 0.3f, 0.3f, 1.0f);
    float fog_factor = (far - depth * 1.25f) / (far - near);
    fog_factor = clamp(fog_factor, 0.0f, 1.0f);

    float ao = v_ao / 3.0;
    float ao_max = (2.0 / (ao + 1.0)) - 1.0;
    frag_color = vec4((ambient + diffuse) * (ao + ao_max * 0.4f), 1.0f) * texture(u_textures, v_uv);
    frag_color = mix(fog_color, frag_color, fog_factor);
}
