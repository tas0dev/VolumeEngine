#version 330 core

in vec2 fragment_texture_coordinate;

uniform vec3 sprite_color;
uniform float sprite_alpha;
uniform float bloom_strength;

layout(location = 0) out vec4 output_color;
layout(location = 1) out vec4 output_brightness;

void main(void) {
    vec2 centered;
    float distance_from_center;
    float coverage;
    float alpha;

    centered = fragment_texture_coordinate * 2.0 - 1.0;
    distance_from_center = length(centered);
    coverage = 1.0 - smoothstep(0.2, 1.0, distance_from_center);
    alpha = sprite_alpha * coverage;
    if (alpha <= 0.001) {
        discard;
    }
    output_color = vec4(sprite_color, alpha);
    output_brightness = vec4(sprite_color * bloom_strength * coverage,
                             alpha);
}
