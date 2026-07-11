#version 330 core

in vec3 vertex_normal;
in vec3 vertex_color;

uniform vec3 light_direction;
uniform vec3 light_color;
uniform float ambient_strength;

out vec4 fragment_color;

void main(void) {
    vec3 normal = normalize(vertex_normal);
    vec3 direction = normalize(-light_direction);
    float diffuse_strength = max(dot(normal, direction), 0.0);
    vec3 ambient = ambient_strength * light_color;
    vec3 diffuse = diffuse_strength * light_color;
    vec3 result = vertex_color * (ambient + diffuse);

    fragment_color = vec4(result, 1.0);
}