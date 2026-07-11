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
#include "platform/platform.h"
#include "renderer/mesh.h"
#include "renderer/shader.h"
#include "shadow_map.h"
#include <SDL3/SDL.h>
#include <epoxy/gl.h>
#include <stdlib.h>

struct renderer {
	platform_t *platform;
	void *context;
	shader_t *shader;
	shader_t *shadow_shader;
	shadow_map_t *shadow_map;
};

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

	renderer->shadow_map = shadow_map_create(2048, 2048);
	if (renderer->shadow_map == NULL) {
		shader_destroy(renderer->shader);
		platform_gl_destroy_context(renderer->context);
		free(renderer);
		return NULL;
	}

	renderer->shadow_shader =
		shader_create(VOLUME_ASSET_DIR "/shaders/shadow.vert",
			      VOLUME_ASSET_DIR "/shaders/shadow.frag");
	if (renderer->shadow_shader == NULL) {
		shadow_map_destroy(renderer->shadow_map);
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

	shader_destroy(renderer->shadow_shader);
	shadow_map_destroy(renderer->shadow_map);
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

void renderer_get_size(const renderer_t *renderer, int *width, int *height) {
	if (width != NULL) { *width = 0; }

	if (height != NULL) { *height = 0; }

	if (renderer == NULL) { return; }

	platform_get_drawable_size(renderer->platform, width, height);
}

void renderer_begin_shadow_pass(renderer_t *renderer,
				const mat4_t *light_view_projection) {
	if (renderer == NULL || light_view_projection == NULL) { return; }

	shadow_map_begin(renderer->shadow_map);
	shader_bind(renderer->shadow_shader);
	shader_set_mat4(renderer->shadow_shader, "light_view_projection",
			light_view_projection);
}

void renderer_draw_mesh(renderer_t *renderer,
			const mesh_t *mesh,
			const material_t *material,
			const mat4_t *model,
			const mat4_t *view,
			const mat4_t *projection,
			const mat4_t *light_view_projection) {
	if (renderer == NULL || mesh == NULL || material == NULL ||
	    model == NULL || view == NULL || projection == NULL ||
	    light_view_projection == NULL) {
		return;
	}

	const vec3_t light_direction = vec3_create(-1.0f, -1.0f, -1.0f);
	const vec3_t light_color = vec3_create(1.0f, 1.0f, 1.0f);

	shader_bind(renderer->shader);
	shader_set_mat4(renderer->shader, "model", model);
	shader_set_mat4(renderer->shader, "view", view);
	shader_set_mat4(renderer->shader, "projection", projection);
	shader_set_mat4(renderer->shader, "light_view_projection",
			light_view_projection);
	shader_set_vec3(renderer->shader, "light_direction", light_direction);
	shader_set_vec3(renderer->shader, "light_color", light_color);
	shader_set_vec3(renderer->shader, "material_color", material->color);
	shader_set_float(renderer->shader, "ambient_strength",
			 material->ambient_strength);
	shader_set_float(renderer->shader, "specular_strength",
			 material->specular_strength);
	shader_set_float(renderer->shader, "shininess", material->shininess);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,
		      shadow_map_get_texture(renderer->shadow_map));
	shader_set_int(renderer->shader, "shadow_map", 0);

	mesh_draw(mesh);

	glBindTexture(GL_TEXTURE_2D, 0);
	shader_unbind();
}

void renderer_end_shadow_pass(renderer_t *renderer) {
	int width;
	int height;

	if (renderer == NULL) { return; }

	shader_unbind();
	shadow_map_end();

	platform_get_drawable_size(renderer->platform, &width, &height);

	glViewport(0, 0, width, height);
}

void renderer_draw_shadow_mesh(renderer_t *renderer,
			       const mesh_t *mesh,
			       const mat4_t *model) {
	if (renderer == NULL || mesh == NULL || model == NULL) { return; }

	shader_set_mat4(renderer->shadow_shader, "model", model);
	mesh_draw(mesh);
}