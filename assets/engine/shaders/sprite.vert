#version 330 core

layout(location = 0) in vec2 corner;
layout(location = 1) in vec2 texture_coordinate;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 center;
uniform vec3 right;
uniform vec3 up;
uniform vec2 half_size;

out vec2 fragment_texture_coordinate;

void main(void) {
    vec3 world_position;

    world_position = center +
        right * corner.x * half_size.x +
        up * corner.y * half_size.y;
    fragment_texture_coordinate = texture_coordinate;
    gl_Position = projection * view * vec4(world_position, 1.0);
}
