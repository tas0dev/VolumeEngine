#version 330 core

in vec4 line_color;

out vec4 fragment_color;

void main() {
	fragment_color = line_color;
}