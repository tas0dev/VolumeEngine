#version 330 core

in vec2 fragment_texture_coordinate;

uniform sampler2D source_texture;
uniform bool horizontal;

out vec4 output_color;

void main(void) {
    const float weights[5] = float[](
            0.227027,
            0.1945946,
            0.1216216,
            0.054054,
            0.016216
    );

    vec2 texel_size = 1.0 / vec2(textureSize(source_texture, 0));
    vec3 result = texture(
            source_texture,
            fragment_texture_coordinate
    ).rgb * weights[0];

    for (int index = 1; index < 5; index++) {
        vec2 offset;

        if (horizontal) {
            offset = vec2(texel_size.x * float(index), 0.0);
        } else {
            offset = vec2(0.0, texel_size.y * float(index));
        }

        result += texture(
                source_texture,
                fragment_texture_coordinate + offset
        ).rgb * weights[index];

        result += texture(
                source_texture,
                fragment_texture_coordinate - offset
        ).rgb * weights[index];
    }

    output_color = vec4(result, 1.0);
}