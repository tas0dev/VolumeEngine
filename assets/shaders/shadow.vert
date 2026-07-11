#version 330 core

layout(location = 0) in vec3 position;

uniform mat4 model;
uniform mat4 light_view_projection;

void main(void) {
    gl_Position = light_view_projection * model * vec4(position, 1.0);
}