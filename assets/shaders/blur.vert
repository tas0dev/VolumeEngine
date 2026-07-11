#version 330 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texture_coordinate;

out vec2 fragment_texture_coordinate;

void main(void) {
    fragment_texture_coordinate = texture_coordinate;
    gl_Position = vec4(position, 0.0, 1.0);
}