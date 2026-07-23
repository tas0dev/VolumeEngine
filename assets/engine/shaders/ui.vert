#version 330 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texture_coordinate;
layout(location = 2) in vec4 color;

uniform vec2 screen_size;

out vec2 fragment_texture_coordinate;
out vec4 fragment_color;

void main() {
	vec2 normalized = position / screen_size;
	gl_Position = vec4(normalized.x * 2.0 - 1.0,
			   1.0 - normalized.y * 2.0, 0.0, 1.0);
	fragment_texture_coordinate = texture_coordinate;
	fragment_color = color;
}
