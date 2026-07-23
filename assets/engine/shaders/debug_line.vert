#version 330 core

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec4 vertex_color;

uniform mat4 view;
uniform mat4 projection;

out vec4 line_color;

void main() {
	line_color = vertex_color;
	gl_Position = projection * view * vec4(vertex_position, 1.0);
}