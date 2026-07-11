#version 330 core

in vec2 fragment_texture_coordinate;

uniform sampler2D hdr_texture;
uniform sampler2D bloom_texture;
uniform float exposure;
uniform float bloom_strength;
uniform bool bloom_enabled;

out vec4 output_color;

vec3 aces_tonemap(vec3 color) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;

    return clamp(
            (color * (a * color + b)) /
            (color * (c * color + d) + e),
            0.0,
            1.0
    );
}

vec3 linear_to_srgb(vec3 color) {
    return pow(
            max(color, vec3(0.0)),
            vec3(1.0 / 2.2)
    );
}

void main(void) {
    vec3 hdr_color;
    vec3 bloom_color;
    vec3 combined_color;
    vec3 mapped_color;

    hdr_color = texture(
            hdr_texture,
            fragment_texture_coordinate
    ).rgb;

    bloom_color = texture(
            bloom_texture,
            fragment_texture_coordinate
    ).rgb;

    combined_color = hdr_color;

    if (bloom_enabled) {
        combined_color += bloom_color * bloom_strength;
    }

    combined_color *= exposure;
    mapped_color = aces_tonemap(combined_color);
    mapped_color = linear_to_srgb(mapped_color);

    output_color = vec4(mapped_color, 1.0);
}