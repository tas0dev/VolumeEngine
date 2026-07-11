#version 330 core

in vec3 fragment_position;
in vec3 fragment_normal;
in vec3 fragment_color;
in vec4 fragment_light_position;

uniform mat4 view;
uniform sampler2D shadow_map;
uniform vec3 light_direction;
uniform vec3 light_color;
uniform vec3 material_color;
uniform float ambient_strength;
uniform float specular_strength;
uniform float shininess;

out vec4 output_color;

float calculate_shadow(vec4 light_position, vec3 normal, vec3 light) {
    vec3 projected = light_position.xyz / light_position.w;
    projected = projected * 0.5 + 0.5;

    if (projected.z > 1.0) {
        return 0.0;
    }

    float current_depth = projected.z;
    float bias = max(
            0.005 * (1.0 - dot(normal, light)),
            0.0005
    );

    vec2 texel_size = 1.0 / textureSize(shadow_map, 0);
    float shadow = 0.0;

    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            float closest_depth = texture(
                    shadow_map,
                    projected.xy + vec2(x, y) * texel_size
            ).r;

            if (current_depth - bias > closest_depth) {
                shadow += 1.0;
            }
        }
    }

    return shadow / 9.0;
}

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

    float shadow = calculate_shadow(
            fragment_light_position,
            normal,
            light
    );

    vec3 ambient = ambient_strength * light_color;
    vec3 diffuse =
    (1.0 - shadow) *
    diffuse_factor *
    light_color;
    vec3 specular =
    (1.0 - shadow) *
    specular_strength *
    specular_factor *
    light_color;

    vec3 base_color = fragment_color * material_color;
    vec3 color = base_color * (ambient + diffuse) + specular;

    output_color = vec4(color, 1.0);
}