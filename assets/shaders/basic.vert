#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;
layout(location = 3) in vec2 texture_coordinate;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 light_view_projection;

out vec3 fragment_position;
out vec3 fragment_normal;
out vec3 fragment_color;
out vec2 fragment_texture_coordinate;
out vec4 fragment_light_position;

void main(void) {
    vec4 world_position;
    vec4 view_position;

    world_position = model * vec4(position, 1.0);
    view_position = view * world_position;

    fragment_position = view_position.xyz;
    fragment_normal =
    normalize(mat3(view * model) * normal);
    fragment_color = color;
    fragment_texture_coordinate = texture_coordinate;
    fragment_light_position =
    light_view_projection * world_position;

    gl_Position = projection * view_position;
}