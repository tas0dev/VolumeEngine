#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 vertex_normal;
out vec3 vertex_color;

void main(void) {
    gl_Position = projection * view * model * vec4(position, 1.0);
    vertex_normal = normalize(mat3(model) * normal);
    vertex_color = color;
}