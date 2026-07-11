/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#include "renderer/renderer.h"
#include "core/log.h"
#include "platform/platform.h"
#include "renderer/mesh.h"
#include <SDL3/SDL.h>
#include <epoxy/gl.h>
#include <stdlib.h>

struct renderer {
	platform_t *platform;
	void *context;
	GLuint shader_program;
	mesh_t *test_mesh;
};

static GLuint renderer_compile_shader(GLenum type, const char *source);
static GLuint renderer_create_shader_program(void);
static mesh_t *renderer_create_test_mesh(void);

static const char *vertex_shader_source =
	"#version 330 core\n"
	"layout(location = 0) in vec3 position;\n"
	"layout(location = 1) in vec3 color;\n"
	"out vec3 vertex_color;\n"
	"void main(void) {\n"
	"\tgl_Position = vec4(position, 1.0);\n"
	"\tvertex_color = color;\n"
	"}\n";

static const char *fragment_shader_source =
	"#version 330 core\n"
	"in vec3 vertex_color;\n"
	"out vec4 fragment_color;\n"
	"void main(void) {\n"
	"\tfragment_color = vec4(vertex_color, 1.0);\n"
	"}\n";

renderer_t *renderer_create(platform_t *platform) {
	if (platform == NULL) {
		log_error("Cannot create renderer without a platform");
		return NULL;
	}

	renderer_t *renderer = calloc(1, sizeof(*renderer));
	if (renderer == NULL) {
		log_error("Failed to allocate renderer");
		return NULL;
	}

	renderer->platform = platform;
	renderer->context = platform_gl_create_context(platform);

	if (renderer->context == NULL) {
		free(renderer);
		return NULL;
	}

	if (!platform_gl_make_current(platform, renderer->context)) {
		platform_gl_destroy_context(renderer->context);
		free(renderer);
		return NULL;
	}

	if (!SDL_GL_SetSwapInterval(1)) {
		log_info("Failed to enable VSync: %s", SDL_GetError());
	}

	renderer->shader_program = renderer_create_shader_program();
	if (renderer->shader_program == 0) {
		platform_gl_destroy_context(renderer->context);
		free(renderer);
		return NULL;
	}

	renderer->test_mesh = renderer_create_test_mesh();
	if (renderer->test_mesh == NULL) {
		glDeleteProgram(renderer->shader_program);
		platform_gl_destroy_context(renderer->context);
		free(renderer);
		return NULL;
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	log_info("OpenGL renderer initialized: %s",
		 (const char *)glGetString(GL_VERSION));

	return renderer;
}

void renderer_destroy(renderer_t *renderer) {
	if (renderer == NULL) { return; }

	mesh_destroy(renderer->test_mesh);

	if (renderer->shader_program != 0) {
		glDeleteProgram(renderer->shader_program);
	}

	platform_gl_destroy_context(renderer->context);
	free(renderer);

	log_info("Renderer shut down");
}

void renderer_begin_frame(const renderer_t *renderer) {
	int width;
	int height;

	if (renderer == NULL) { return; }

	platform_get_drawable_size(renderer->platform, &width, &height);

	glViewport(0, 0, width, height);
	glClearColor(0.08f, 0.09f, 0.11f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void renderer_end_frame(const renderer_t *renderer) {
	if (renderer == NULL) { return; }

	platform_gl_swap_buffers(renderer->platform);
}

static GLuint renderer_compile_shader(const GLenum type, const char *source) {
	GLint success;
	GLint log_length;

	const GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (success == GL_TRUE) { return shader; }

	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
	if (log_length <= 0) {
		log_error("Shader compilation failed");
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
	log_error("Shader compilation failed: %s", log);

	free(log);
	glDeleteShader(shader);

	return 0;
}

static GLuint renderer_create_shader_program(void) {
	GLint success;
	GLint log_length;

	const GLuint vertex_shader =
		renderer_compile_shader(GL_VERTEX_SHADER, vertex_shader_source);

	if (vertex_shader == 0) { return 0; }

	const GLuint fragment_shader = renderer_compile_shader(
		GL_FRAGMENT_SHADER, fragment_shader_source);

	if (fragment_shader == 0) {
		glDeleteShader(vertex_shader);
		return 0;
	}

	const GLuint program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

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

void renderer_draw(const renderer_t *renderer) {
	if (renderer == NULL) { return; }

	glUseProgram(renderer->shader_program);
	mesh_draw(renderer->test_mesh);
	glUseProgram(0);
}

static mesh_t *renderer_create_test_mesh(void) {
	static const mesh_vertex_t vertices[] = {
		{
			.position = {0.0f, 0.6f, 0.0f},
			.color = {1.0f, 0.2f, 0.2f},
		 },
		{
			.position = {-0.6f, -0.6f, 0.0f},
			.color = {0.2f, 1.0f, 0.2f},
		 },
		{
			.position = {0.6f, -0.6f, 0.0f},
			.color = {0.2f, 0.4f, 1.0f},
		 },
	};

	return mesh_create(vertices, sizeof(vertices) / sizeof(vertices[0]));
}