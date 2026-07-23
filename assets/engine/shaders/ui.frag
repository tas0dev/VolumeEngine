#version 330 core

in vec2 fragment_texture_coordinate;
in vec4 fragment_color;

uniform sampler2D font_texture;

out vec4 output_color;

void main() {
	float coverage = texture(font_texture, fragment_texture_coordinate).r;
	output_color = vec4(fragment_color.rgb, fragment_color.a * coverage);
}
