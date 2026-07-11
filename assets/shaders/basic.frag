#version 330 core

in vec3 fragment_position;
in vec3 fragment_normal;
in vec3 fragment_color;

uniform mat4 view;
uniform vec3 light_direction;
uniform vec3 light_color;
uniform float ambient_strength;

out vec4 output_color;

void main(void) {
    const float specular_strength = 0.5;
    const float shininess = 32.0;

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

    vec3 color =
    fragment_color * (ambient + diffuse) +
    specular;

    output_color = vec4(color, 1.0);
}