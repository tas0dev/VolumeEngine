#version 330 core

in vec3 fragment_position;
in vec3 fragment_normal;
in vec3 fragment_color;

uniform mat4 view;
uniform vec3 light_direction;
uniform vec3 light_color;
uniform float ambient_strength;
uniform vec3 material_color;
uniform float specular_strength;
uniform float shininess;

out vec4 output_color;

void main(void) {
    vec3 normal = normalize(fragment_normal);
    vec3 light = normalize(mat3(view) * -light_direction);
    vec3 view_direction = normalize(-fragment_position);
    vec3 halfway_direction = normalize(light + view_direction);

    float diffuse_factor = max(dot(normal, light), 0.0);
    float specular_factor = 0.0;

    if (diffuse_factor > 0.0) {
        specular_factor = pow(
                max(dot(normal, halfway_direction), 0.0),
                shininess
        );
    }

    vec3 ambient = ambient_strength * light_color;
    vec3 diffuse = diffuse_factor * light_color;
    vec3 specular =
    specular_strength *
    specular_factor *
    light_color;

    vec3 base_color = fragment_color * material_color;

    vec3 color =
    base_color * (ambient + diffuse) +
    specular;

    output_color = vec4(color, 1.0);
}