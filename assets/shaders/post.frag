#version 330 core

in vec2 fragment_texture_coordinate;

uniform sampler2D hdr_texture;
uniform float exposure;

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

void main(void) {
    vec3 hdr_color = texture(
            hdr_texture,
            fragment_texture_coordinate
    ).rgb;

    hdr_color *= exposure;

    vec3 mapped = aces_tonemap(hdr_color);
    mapped = pow(mapped, vec3(1.0 / 2.2));

    output_color = vec4(mapped, 1.0);
}