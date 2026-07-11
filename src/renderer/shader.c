/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#include "renderer/shader.h"
#include "core/log.h"
#include <epoxy/gl.h>
#include <stdio.h>
#include <stdlib.h>

struct shader {
	GLuint program;
};

static char *shader_read_file(const char *path) {
	FILE *file = fopen(path, "rb");
	if (file == NULL) {
		log_error("Failed to open shader file: %s", path);
		return NULL;
	}

	if (fseek(file, 0, SEEK_END) != 0) {
		log_error("Failed to seek shader file: %s", path);
		fclose(file);
		return NULL;
	}

	const long length = ftell(file);
	if (length < 0) {
		log_error("Failed to determine shader file size: %s", path);
		fclose(file);
		return NULL;
	}

	if (fseek(file, 0, SEEK_SET) != 0) {
		log_error("Failed to rewind shader file: %s", path);
		fclose(file);
		return NULL;
	}

	char *source = malloc((size_t)length + 1);
	if (source == NULL) {
		log_error("Failed to allocate shader source");
		fclose(file);
		return NULL;
	}

	const size_t read_length = fread(source, 1, (size_t)length, file);
	if (read_length != (size_t)length) {
		log_error("Failed to read shader file: %s", path);
		free(source);
		fclose(file);
		return NULL;
	}

	source[length] = '\0';

	fclose(file);

	return source;
}

static GLuint
shader_compile(GLenum type, const char *source, const char *path) {
	GLint success;
	GLint log_length;

	const GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (success == GL_TRUE) { return shader; }

	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);

	if (log_length <= 0) {
		log_error("Shader compilation failed: %s", path);
		glDeleteShader(shader);
		return 0;
	}

	char *log = malloc((size_t)log_length);
	if (log == NULL) {
		log_error("Failed to allocate shader log");
		glDeleteShader(shader);
		return 0;
	}

	glGetShaderInfoLog(shader, log_length, NULL, log);
	log_error("Shader compilation failed in %s: %s", path, log);

	free(log);
	glDeleteShader(shader);

	return 0;
}

static GLuint shader_link(GLuint vertex_shader, GLuint fragment_shader) {
	GLint success;
	GLint log_length;

	const GLuint program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (success == GL_TRUE) { return program; }

	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);

	if (log_length <= 0) {
		log_error("Shader program linking failed");
		glDeleteProgram(program);
		return 0;
	}

	char *log = malloc((size_t)log_length);
	if (log == NULL) {
		log_error("Failed to allocate shader program log");
		glDeleteProgram(program);
		return 0;
	}

	glGetProgramInfoLog(program, log_length, NULL, log);
	log_error("Shader program linking failed: %s", log);

	free(log);
	glDeleteProgram(program);

	return 0;
}

shader_t *shader_create(const char *vertex_path, const char *fragment_path) {
	if (vertex_path == NULL || fragment_path == NULL) {
		log_error("Invalid shader paths");
		return NULL;
	}

	char *vertex_source = shader_read_file(vertex_path);
	if (vertex_source == NULL) { return NULL; }

	char *fragment_source = shader_read_file(fragment_path);
	if (fragment_source == NULL) {
		free(vertex_source);
		return NULL;
	}

	const GLuint vertex_shader =
		shader_compile(GL_VERTEX_SHADER, vertex_source, vertex_path);

	const GLuint fragment_shader = shader_compile(
		GL_FRAGMENT_SHADER, fragment_source, fragment_path);

	free(vertex_source);
	free(fragment_source);

	if (vertex_shader == 0 || fragment_shader == 0) {
		if (vertex_shader != 0) { glDeleteShader(vertex_shader); }

		if (fragment_shader != 0) { glDeleteShader(fragment_shader); }

		return NULL;
	}

	shader_t *shader = calloc(1, sizeof(*shader));
	if (shader == NULL) {
		log_error("Failed to allocate shader");
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);
		return NULL;
	}

	shader->program = shader_link(vertex_shader, fragment_shader);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	if (shader->program == 0) {
		free(shader);
		return NULL;
	}

	return shader;
}

void shader_destroy(shader_t *shader) {
	if (shader == NULL) { return; }

	if (shader->program != 0) { glDeleteProgram(shader->program); }

	free(shader);
}

void shader_bind(const shader_t *shader) {
	if (shader == NULL) { return; }

	glUseProgram(shader->program);
}

void shader_unbind(void) { glUseProgram(0); }
