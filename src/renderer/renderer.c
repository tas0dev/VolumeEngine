/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#include "renderer/renderer.h"
#include "core/log.h"
#include "math/mat4.h"
#include "math/vec3.h"
#include "platform/platform.h"
#include "renderer/mesh.h"
#include "renderer/shader.h"
#include <SDL3/SDL.h>
#include <epoxy/gl.h>
#include <stdlib.h>

struct renderer {
	platform_t *platform;
	void *context;
	GLuint shader_program;
	shader_t *shader;
	mesh_t *test_mesh;
	float rotation;
};

static mesh_t *renderer_create_test_mesh(void);

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

	renderer->shader = shader_create("assets/shaders/basic.vert",
					 "assets/shaders/basic.frag");

	if (renderer->shader == NULL) {
		platform_gl_destroy_context(renderer->context);
		free(renderer);
		return NULL;
	}

	renderer->test_mesh = renderer_create_test_mesh();
	if (renderer->test_mesh == NULL) {
		shader_destroy(renderer->shader);
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
	shader_destroy(renderer->shader);
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

void renderer_draw(renderer_t *renderer) {
	mat4_t model;
	mat4_t view;
	mat4_t projection;
	int width;
	int height;
	const float pi = 3.14159265358979323846f;

	if (renderer == NULL) { return; }

	platform_get_drawable_size(renderer->platform, &width, &height);

	if (width <= 0 || height <= 0) { return; }

	const float aspect_ratio = (float)width / (float)height;

	model = mat4_rotation_y(renderer->rotation);
	view = mat4_translation(vec3_create(0.0f, 0.0f, -2.5f));
	projection = mat4_perspective(60.0f * pi / 180.0f, aspect_ratio, 0.1f,
				      100.0f);

	shader_bind(renderer->shader);
	shader_set_mat4(
		renderer->shader, "model", &model);
	shader_set_mat4(renderer->shader, "view", &view);
	shader_set_mat4(renderer->shader, "projection", &projection);

	mesh_draw(renderer->test_mesh);
	shader_unbind();

	renderer->rotation += 0.01f;
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