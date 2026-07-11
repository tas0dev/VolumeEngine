#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 fragment_position;
out vec3 fragment_normal;
out vec3 fragment_color;

void main(void) {
    vec4 world_position = model * vec4(position, 1.0);
    vec4 view_position = view * world_position;

    fragment_position = view_position.xyz;
    fragment_normal = normalize(mat3(view * model) * normal);
    fragment_color = color;

    gl_Position = projection * view_position;
}