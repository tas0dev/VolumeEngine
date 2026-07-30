/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "renderer/renderer.h"

#include "core/log.h"
#include "core/path.h"
#include "math/mat4.h"
#include "math/math.h"
#include "platform/platform.h"
#include "renderer/bloom_buffer.h"
#include "renderer/hdr_buffer.h"
#include "renderer/mesh.h"
#include "renderer/shader.h"
#include "renderer/texture.h"
#include "renderer/ui_renderer.h"
#include "shadow_map.h"
#include <SDL3/SDL.h>
#include <epoxy/gl.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct debug_line_vertex {
	float x;
	float y;
	float z;
	float red;
	float green;
	float blue;
	float alpha;
} debug_line_vertex_t;

struct renderer {
	platform_t *platform;
	void *context;
	shader_t *shader;
	shader_t *shadow_shader;
	shadow_map_t *shadow_map;
	hdr_buffer_t *hdr_buffer;
	shader_t *post_shader;
	GLuint screen_vertex_array;
	GLuint screen_vertex_buffer;
	bloom_buffer_t *bloom_buffer;
	shader_t *blur_shader;
	int framebuffer_width;
	int framebuffer_height;
	ui_renderer_t *ui_renderer;
	renderer_frame_stats_t frame_stats;
	shader_t *debug_line_shader;
	GLuint debug_line_vertex_array;
	GLuint debug_line_vertex_buffer;
	shader_t *sprite_shader;
	GLuint sprite_vertex_array;
	GLuint sprite_vertex_buffer;
	debug_line_vertex_t *debug_line_vertices;
	size_t debug_line_vertex_count;
	size_t debug_line_vertex_capacity;
	mat4_t view_model_projection;
	bool view_model_pass_active;
};

static bool renderer_create_debug_lines(renderer_t *renderer);
static bool renderer_create_sprite(renderer_t *renderer);
static bool renderer_reserve_debug_line_vertices(renderer_t *renderer,
						 size_t capacity);
static void renderer_draw_mesh_internal(renderer_t *renderer,
					const mesh_t *mesh,
					const material_t *material,
					const mat4_t *model,
					const render_view_t *view,
					bool shadows_enabled,
					const mat4_t *bone_matrices,
					size_t bone_count);
static void renderer_draw_sprite_internal(renderer_t *renderer,
					  vec3_t position,
					  vec3_t right,
					  vec3_t up,
					  float half_width,
					  float half_height,
					  renderer_color_t color,
					  renderer_blend_mode_t blend_mode,
					  const mat4_t *view,
					  const mat4_t *projection);

static bool renderer_create_screen_quad(renderer_t *renderer) {
	static const float vertices[] = {
		-1.0f, -1.0f, 0.0f, 0.0f, 1.0f,	 -1.0f, 1.0f, 0.0f,
		1.0f,  1.0f,  1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
		1.0f,  1.0f,  1.0f, 1.0f, -1.0f, 1.0f,	0.0f, 1.0f,
	};

	glGenVertexArrays(1, &renderer->screen_vertex_array);
	glGenBuffers(1, &renderer->screen_vertex_buffer);

	glBindVertexArray(renderer->screen_vertex_array);
	glBindBuffer(GL_ARRAY_BUFFER, renderer->screen_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
		     GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
			      (const void *)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
			      (const void *)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return true;
}

static shader_t *renderer_load_shader(const char *vertex_path,
				      const char *fragment_path) {
	char *resolved_vertex_path;
	char *resolved_fragment_path;
	shader_t *shader;

	resolved_vertex_path = path_from_executable(vertex_path);
	resolved_fragment_path = path_from_executable(fragment_path);

	if (resolved_vertex_path == NULL || resolved_fragment_path == NULL) {
		free(resolved_vertex_path);
		free(resolved_fragment_path);
		return NULL;
	}

	shader = shader_create(resolved_vertex_path, resolved_fragment_path);

	free(resolved_vertex_path);
	free(resolved_fragment_path);

	return shader;
}

static bool renderer_create_debug_lines(renderer_t *renderer) {
	const GLsizei stride = sizeof(debug_line_vertex_t);

	if (renderer == NULL) { return false; }

	glGenVertexArrays(1, &renderer->debug_line_vertex_array);
	glGenBuffers(1, &renderer->debug_line_vertex_buffer);

	if (renderer->debug_line_vertex_array == 0 ||
	    renderer->debug_line_vertex_buffer == 0) {
		return false;
	}

	glBindVertexArray(renderer->debug_line_vertex_array);
	glBindBuffer(GL_ARRAY_BUFFER, renderer->debug_line_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride,
			      (const void *)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride,
			      (const void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return true;
}

static bool renderer_create_sprite(renderer_t *renderer) {
	static const float vertices[] = {
		-1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f,  0.0f, 1.0f, 1.0f, 1.0f,  1.0f, 1.0f,
	};

	if (renderer == NULL) { return false; }
	glGenVertexArrays(1, &renderer->sprite_vertex_array);
	glGenBuffers(1, &renderer->sprite_vertex_buffer);
	if (renderer->sprite_vertex_array == 0 ||
	    renderer->sprite_vertex_buffer == 0) {
		return false;
	}
	glBindVertexArray(renderer->sprite_vertex_array);
	glBindBuffer(GL_ARRAY_BUFFER, renderer->sprite_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
		     GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
			      (const void *)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
			      (const void *)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	return true;
}

static bool renderer_reserve_debug_line_vertices(renderer_t *renderer,
						 const size_t capacity) {
	debug_line_vertex_t *vertices;
	size_t new_capacity;

	if (renderer == NULL) { return false; }

	if (capacity <= renderer->debug_line_vertex_capacity) { return true; }

	new_capacity = renderer->debug_line_vertex_capacity;
	if (new_capacity == 0) { new_capacity = 256; }

	while (new_capacity < capacity) {
		if (new_capacity > SIZE_MAX / 2) {
			new_capacity = capacity;
			break;
		}

		new_capacity *= 2;
	}

	vertices = realloc(renderer->debug_line_vertices,
			   new_capacity * sizeof(*vertices));
	if (vertices == NULL) { return false; }

	renderer->debug_line_vertices = vertices;
	renderer->debug_line_vertex_capacity = new_capacity;

	return true;
}

renderer_t *renderer_create(platform_t *platform) {
	renderer_t *renderer;
	int width;
	int height;

	if (platform == NULL) {
		log_error("Cannot create renderer without a platform");
		return NULL;
	}

	renderer = calloc(1, sizeof(*renderer));
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

	renderer->shader =
		renderer_load_shader("assets/engine/shaders/basic.vert",
				     "assets/engine/shaders/basic.frag");
	if (renderer->shader == NULL) {
		renderer_destroy(renderer);
		return NULL;
	}

	renderer->shadow_shader =
		renderer_load_shader("assets/engine/shaders/shadow.vert",
				     "assets/engine/shaders/shadow.frag");
	if (renderer->shadow_shader == NULL) {
		renderer_destroy(renderer);
		return NULL;
	}

	renderer->post_shader =
		renderer_load_shader("assets/engine/shaders/post.vert",
				     "assets/engine/shaders/post.frag");
	if (renderer->post_shader == NULL) {
		renderer_destroy(renderer);
		return NULL;
	}

	renderer->blur_shader =
		renderer_load_shader("assets/engine/shaders/blur.vert",
				     "assets/engine/shaders/blur.frag");
	if (renderer->blur_shader == NULL) {
		renderer_destroy(renderer);
		return NULL;
	}

	renderer->shadow_map = shadow_map_create(2048, 2048);
	if (renderer->shadow_map == NULL) {
		renderer_destroy(renderer);
		return NULL;
	}

	platform_get_drawable_size(platform, &width, &height);
	if (width <= 0 || height <= 0) {
		log_error("Invalid drawable size: %dx%d", width, height);
		renderer_destroy(renderer);
		return NULL;
	}

	renderer->hdr_buffer = hdr_buffer_create(width, height);
	if (renderer->hdr_buffer == NULL) {
		renderer_destroy(renderer);
		return NULL;
	}

	renderer->bloom_buffer = bloom_buffer_create(width, height);
	if (renderer->bloom_buffer == NULL) {
		renderer_destroy(renderer);
		return NULL;
	}

	if (!renderer_create_screen_quad(renderer)) {
		renderer_destroy(renderer);
		return NULL;
	}

	renderer->debug_line_shader =
		renderer_load_shader("assets/engine/shaders/debug_line.vert",
				     "assets/engine/shaders/debug_line.frag");
	if (renderer->debug_line_shader == NULL) {
		renderer_destroy(renderer);
		return NULL;
	}

	if (!renderer_create_debug_lines(renderer)) {
		renderer_destroy(renderer);
		return NULL;
	}
	renderer->sprite_shader =
		renderer_load_shader("assets/engine/shaders/sprite.vert",
				     "assets/engine/shaders/sprite.frag");
	if (renderer->sprite_shader == NULL ||
	    !renderer_create_sprite(renderer)) {
		renderer_destroy(renderer);
		return NULL;
	}

	renderer->ui_renderer = ui_renderer_create();
	if (renderer->ui_renderer == NULL) {
		renderer_destroy(renderer);
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

	if (renderer->screen_vertex_buffer != 0) {
		glDeleteBuffers(1, &renderer->screen_vertex_buffer);
	}

	if (renderer->screen_vertex_array != 0) {
		glDeleteVertexArrays(1, &renderer->screen_vertex_array);
	}

	if (renderer->debug_line_vertex_buffer != 0) {
		glDeleteBuffers(1, &renderer->debug_line_vertex_buffer);
	}

	if (renderer->debug_line_vertex_array != 0) {
		glDeleteVertexArrays(1, &renderer->debug_line_vertex_array);
	}

	free(renderer->debug_line_vertices);
	shader_destroy(renderer->debug_line_shader);
	if (renderer->sprite_vertex_buffer != 0) {
		glDeleteBuffers(1, &renderer->sprite_vertex_buffer);
	}
	if (renderer->sprite_vertex_array != 0) {
		glDeleteVertexArrays(1, &renderer->sprite_vertex_array);
	}
	shader_destroy(renderer->sprite_shader);

	ui_renderer_destroy(renderer->ui_renderer);

	bloom_buffer_destroy(renderer->bloom_buffer);
	hdr_buffer_destroy(renderer->hdr_buffer);
	shader_destroy(renderer->blur_shader);
	shader_destroy(renderer->post_shader);
	shader_destroy(renderer->shadow_shader);
	shader_destroy(renderer->shader);
	shadow_map_destroy(renderer->shadow_map);
	platform_gl_destroy_context(renderer->context);

	free(renderer);

	log_info("Renderer shut down");
}

void renderer_begin_frame(renderer_t *renderer) {
	int width;
	int height;

	if (renderer == NULL) { return; }

	platform_get_drawable_size(renderer->platform, &width, &height);

	if (width <= 0 || height <= 0) { return; }

	if (!hdr_buffer_resize(renderer->hdr_buffer, width, height)) { return; }

	if (!bloom_buffer_resize(renderer->bloom_buffer, width, height)) {
		return;
	}

	renderer->framebuffer_width = width;
	renderer->framebuffer_height = height;
	renderer->frame_stats = (renderer_frame_stats_t){0};
	renderer->view_model_pass_active = false;
	ui_renderer_begin_frame(renderer->ui_renderer);

	hdr_buffer_bind(renderer->hdr_buffer);

	glViewport(0, 0, width, height);
	glEnable(GL_DEPTH_TEST);

	glClearColor(0.08f, 0.09f, 0.11f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
		GL_STENCIL_BUFFER_BIT);
}

static unsigned int renderer_blur_bloom(const renderer_t *renderer,
					const int pass_count) {
	bool horizontal;
	bool first_pass;
	int pass;
	int target_index;
	int source_index;

	horizontal = true;
	first_pass = true;

	shader_bind(renderer->blur_shader);
	shader_set_int(renderer->blur_shader, "source_texture", 0);

	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(renderer->screen_vertex_array);

	for (pass = 0; pass < pass_count; pass++) {
		target_index = horizontal ? 1 : 0;

		bloom_buffer_bind(renderer->bloom_buffer, target_index);

		glViewport(0, 0, renderer->framebuffer_width,
			   renderer->framebuffer_height);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		shader_set_int(renderer->blur_shader, "horizontal",
			       horizontal ? 1 : 0);

		if (first_pass) {
			glBindTexture(GL_TEXTURE_2D,
				      hdr_buffer_get_brightness_texture(
					      renderer->hdr_buffer));
		} else {
			source_index = horizontal ? 0 : 1;

			glBindTexture(
				GL_TEXTURE_2D,
				bloom_buffer_get_texture(renderer->bloom_buffer,
							 source_index));
		}

		glDrawArrays(GL_TRIANGLES, 0, 6);

		horizontal = !horizontal;
		first_pass = false;
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	shader_unbind();

	hdr_buffer_unbind();

	return bloom_buffer_get_texture(renderer->bloom_buffer,
					horizontal ? 0 : 1);
}

void renderer_end_frame(const renderer_t *renderer) {
	unsigned int bloom_texture;

	if (renderer == NULL) { return; }

	glDisable(GL_DEPTH_TEST);

	bloom_texture = renderer_blur_bloom(renderer, 10);

	hdr_buffer_unbind();

	glViewport(0, 0, renderer->framebuffer_width,
		   renderer->framebuffer_height);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	shader_bind(renderer->post_shader);

	shader_set_int(renderer->post_shader, "hdr_texture", 0);
	shader_set_int(renderer->post_shader, "bloom_texture", 1);
	shader_set_float(renderer->post_shader, "exposure", 1.0f);
	shader_set_float(renderer->post_shader, "bloom_strength", 0.08f);
	shader_set_int(renderer->post_shader, "bloom_enabled", 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,
		      hdr_buffer_get_texture(renderer->hdr_buffer));
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, bloom_texture);
	glBindVertexArray(renderer->screen_vertex_array);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	shader_unbind();
	ui_renderer_flush(renderer->ui_renderer, renderer->framebuffer_width,
			  renderer->framebuffer_height);

	glEnable(GL_DEPTH_TEST);

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

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	shadow_map_begin(renderer->shadow_map);
	shader_bind(renderer->shadow_shader);
	shader_set_mat4(renderer->shadow_shader, "light_view_projection",
			light_view_projection);
}

void renderer_draw_mesh(renderer_t *renderer,
			const mesh_t *mesh,
			const material_t *material,
			const mat4_t *model,
			const render_view_t *view) {
	if (renderer == NULL || mesh == NULL || material == NULL ||
	    model == NULL || view == NULL) {
		return;
	}
	renderer_draw_mesh_internal(renderer, mesh, material, model, view, true,
				    NULL, 0);
}

bool renderer_begin_view_model_pass(renderer_t *renderer,
				    const float field_of_view,
				    const float near_plane,
				    const float far_plane) {
	float aspect_ratio;

	if (renderer == NULL || !isfinite(field_of_view) ||
	    field_of_view <= 1.0f || field_of_view >= 179.0f ||
	    !isfinite(near_plane) || !isfinite(far_plane) ||
	    near_plane <= 0.0f || far_plane <= near_plane ||
	    renderer->framebuffer_width <= 0 ||
	    renderer->framebuffer_height <= 0) {
		return false;
	}
	aspect_ratio = (float)renderer->framebuffer_width /
		       (float)renderer->framebuffer_height;
	renderer->view_model_projection =
		mat4_perspective(field_of_view * PI / 180.0f, aspect_ratio,
				 near_plane, far_plane);
	renderer->view_model_pass_active = true;
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glClear(GL_DEPTH_BUFFER_BIT);
	return true;
}

void renderer_draw_view_model_mesh(renderer_t *renderer,
				   const mesh_t *mesh,
				   const material_t *material,
				   const mat4_t *model,
				   const render_view_t *world_view) {
	render_view_t view;

	if (renderer == NULL || !renderer->view_model_pass_active ||
	    world_view == NULL) {
		return;
	}
	view = *world_view;
	view.view = mat4_identity();
	view.projection = renderer->view_model_projection;
	view.light_view_projection = mat4_identity();
	renderer_draw_mesh_internal(renderer, mesh, material, model, &view,
				    false, NULL, 0);
}

void renderer_draw_view_model_animated_mesh(renderer_t *renderer,
					    const mesh_t *mesh,
					    const material_t *material,
					    const mat4_t *model,
					    const render_view_t *world_view,
					    const animator_t *animator) {
	render_view_t view;
	const mat4_t *bone_matrices;
	size_t bone_count;

	if (renderer == NULL || !renderer->view_model_pass_active ||
	    mesh == NULL || material == NULL || model == NULL ||
	    world_view == NULL || animator == NULL) {
		return;
	}
	bone_matrices = animator_get_bone_matrices(animator, &bone_count);
	if (bone_matrices == NULL || bone_count == 0 ||
	    bone_count > ANIMATION_MAX_BONES) {
		return;
	}
	view = *world_view;
	view.view = mat4_identity();
	view.projection = renderer->view_model_projection;
	view.light_view_projection = mat4_identity();
	renderer_draw_mesh_internal(renderer, mesh, material, model, &view,
				    false, bone_matrices, bone_count);
}

void renderer_end_view_model_pass(renderer_t *renderer) {
	if (renderer == NULL) { return; }
	renderer->view_model_pass_active = false;
}

void renderer_draw_world_billboard(renderer_t *renderer,
				   const vec3_t position,
				   const float size,
				   const renderer_color_t color,
				   const renderer_blend_mode_t blend_mode,
				   const render_view_t *view) {
	vec3_t right;
	vec3_t up;

	if (view == NULL) { return; }
	right = vec3_create(view->view.elements[0], view->view.elements[4],
			    view->view.elements[8]);
	up = vec3_create(view->view.elements[1], view->view.elements[5],
			 view->view.elements[9]);
	renderer_draw_sprite_internal(renderer, position, right, up, size, size,
				      color, blend_mode, &view->view,
				      &view->projection);
}

void renderer_draw_world_sprite(renderer_t *renderer,
				const vec3_t position,
				const vec3_t right,
				const vec3_t up,
				const float size,
				const renderer_color_t color,
				const renderer_blend_mode_t blend_mode,
				const render_view_t *view) {
	if (view == NULL) { return; }
	renderer_draw_sprite_internal(renderer, position, right, up, size, size,
				      color, blend_mode, &view->view,
				      &view->projection);
}

void renderer_draw_world_beam(renderer_t *renderer,
			      const vec3_t start,
			      const vec3_t end,
			      const float width,
			      const renderer_color_t color,
			      const renderer_blend_mode_t blend_mode,
			      const render_view_t *view) {
	vec3_t center;
	vec3_t direction;
	vec3_t view_forward;
	vec3_t width_axis;
	float length;

	if (view == NULL || !isfinite(width) || width <= 0.0f) { return; }
	direction = vec3_subtract(end, start);
	length = vec3_length(direction);
	if (length <= 0.000001f) { return; }
	direction = vec3_scale(direction, 1.0f / length);
	view_forward =
		vec3_create(-view->view.elements[2], -view->view.elements[6],
			    -view->view.elements[10]);
	width_axis = vec3_cross(direction, view_forward);
	if (vec3_length(width_axis) <= 0.000001f) {
		width_axis = vec3_create(view->view.elements[1],
					 view->view.elements[5],
					 view->view.elements[9]);
	}
	center = vec3_scale(vec3_add(start, end), 0.5f);
	renderer_draw_sprite_internal(renderer, center, direction, width_axis,
				      length * 0.5f, width * 0.5f, color,
				      blend_mode, &view->view,
				      &view->projection);
}

void renderer_draw_view_model_sprite(renderer_t *renderer,
				     const vec3_t position,
				     const float size,
				     const renderer_color_t color,
				     const renderer_blend_mode_t blend_mode) {
	mat4_t view;

	if (renderer == NULL || !renderer->view_model_pass_active) { return; }
	view = mat4_identity();
	renderer_draw_sprite_internal(
		renderer, position, vec3_create(1.0f, 0.0f, 0.0f),
		vec3_create(0.0f, 1.0f, 0.0f), size, size, color, blend_mode,
		&view, &renderer->view_model_projection);
}

static void
renderer_draw_sprite_internal(renderer_t *renderer,
			      const vec3_t position,
			      vec3_t right,
			      vec3_t up,
			      const float half_width,
			      const float half_height,
			      const renderer_color_t color,
			      const renderer_blend_mode_t blend_mode,
			      const mat4_t *view,
			      const mat4_t *projection) {
	GLboolean blend_enabled;
	GLboolean depth_test_enabled;
	GLboolean depth_mask;
	GLint previous_blend_source;
	GLint previous_blend_destination;

	if (renderer == NULL || renderer->sprite_shader == NULL ||
	    view == NULL || projection == NULL || !isfinite(half_width) ||
	    half_width <= 0.0f || !isfinite(half_height) ||
	    half_height <= 0.0f || vec3_length(right) <= 0.000001f ||
	    vec3_length(up) <= 0.000001f) {
		return;
	}
	right = vec3_normalize(right);
	up = vec3_normalize(up);
	blend_enabled = glIsEnabled(GL_BLEND);
	depth_test_enabled = glIsEnabled(GL_DEPTH_TEST);
	glGetBooleanv(GL_DEPTH_WRITEMASK, &depth_mask);
	glGetIntegerv(GL_BLEND_SRC_RGB, &previous_blend_source);
	glGetIntegerv(GL_BLEND_DST_RGB, &previous_blend_destination);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glBlendFunc(GL_SRC_ALPHA, blend_mode == RENDERER_BLEND_ADDITIVE
					  ? GL_ONE
					  : GL_ONE_MINUS_SRC_ALPHA);
	shader_bind(renderer->sprite_shader);
	shader_set_mat4(renderer->sprite_shader, "view", view);
	shader_set_mat4(renderer->sprite_shader, "projection", projection);
	shader_set_vec3(renderer->sprite_shader, "center", position);
	shader_set_vec3(renderer->sprite_shader, "right", right);
	shader_set_vec3(renderer->sprite_shader, "up", up);
	shader_set_vec2(renderer->sprite_shader, "half_size", half_width,
			half_height);
	shader_set_vec3(renderer->sprite_shader, "sprite_color",
			vec3_create(color.r, color.g, color.b));
	shader_set_float(renderer->sprite_shader, "sprite_alpha", color.a);
	shader_set_float(renderer->sprite_shader, "bloom_strength",
			 blend_mode == RENDERER_BLEND_ADDITIVE ? 1.5f : 0.0f);
	glBindVertexArray(renderer->sprite_vertex_array);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
	shader_unbind();
	glBlendFunc(previous_blend_source, previous_blend_destination);
	glDepthMask(depth_mask);
	if (!blend_enabled) { glDisable(GL_BLEND); }
	if (!depth_test_enabled) { glDisable(GL_DEPTH_TEST); }
}

static void renderer_draw_mesh_internal(renderer_t *renderer,
					const mesh_t *mesh,
					const material_t *material,
					const mat4_t *model,
					const render_view_t *view,
					const bool shadows_enabled,
					const mat4_t *bone_matrices,
					const size_t bone_count) {
	vec3_t light_color;

	light_color = vec3_scale(view->light_color, view->light_intensity);

	shader_bind(renderer->shader);

	shader_set_mat4(renderer->shader, "model", model);
	shader_set_mat4(renderer->shader, "view", &view->view);
	shader_set_mat4(renderer->shader, "projection", &view->projection);
	shader_set_mat4(renderer->shader, "light_view_projection",
			&view->light_view_projection);

	shader_set_vec3(renderer->shader, "light_direction",
			view->light_direction);
	shader_set_vec3(renderer->shader, "light_color", light_color);
	shader_set_vec3(renderer->shader, "material_color", material->color);

	shader_set_float(renderer->shader, "ambient_strength",
			 material->ambient_strength);
	shader_set_float(renderer->shader, "specular_strength",
			 material->specular_strength);
	shader_set_float(renderer->shader, "shininess", material->shininess);
	shader_set_int(renderer->shader, "shadows_enabled",
		       shadows_enabled ? 1 : 0);
	shader_set_int(renderer->shader, "skinning_enabled",
		       bone_matrices != NULL && bone_count > 0);
	if (bone_matrices != NULL && bone_count > 0) {
		shader_set_mat4_array(renderer->shader, "bone_matrices[0]",
				      bone_matrices, bone_count);
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,
		      shadow_map_get_texture(renderer->shadow_map));
	shader_set_int(renderer->shader, "shadow_map", 0);

	shader_set_int(renderer->shader, "albedo_texture", 1);
	shader_set_int(renderer->shader, "has_albedo_texture",
		       material->albedo_texture != NULL);

	if (material->albedo_texture != NULL) {
		texture_bind(material->albedo_texture, 1);
	}

	shader_set_int(renderer->shader, "normal_texture", 2);
	shader_set_int(renderer->shader, "has_normal_texture",
		       material->normal_texture != NULL);

	if (material->normal_texture != NULL) {
		texture_bind(material->normal_texture, 2);
	}

	mesh_draw(mesh);
	renderer->frame_stats.mesh_draw_calls++;

	if (material->normal_texture != NULL) { texture_unbind(2); }

	if (material->albedo_texture != NULL) { texture_unbind(1); }

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	shader_unbind();
}

void renderer_end_shadow_pass(renderer_t *renderer) {
	int width;
	int height;

	if (renderer == NULL) { return; }

	shader_unbind();
	shadow_map_end();

	glCullFace(GL_BACK);
	glDisable(GL_CULL_FACE);

	platform_get_drawable_size(renderer->platform, &width, &height);

	hdr_buffer_bind(renderer->hdr_buffer);
	glViewport(0, 0, width, height);
}

void renderer_draw_shadow_mesh(renderer_t *renderer,
			       const mesh_t *mesh,
			       const mat4_t *model) {
	if (renderer == NULL || mesh == NULL || model == NULL) { return; }

	shader_set_mat4(renderer->shadow_shader, "model", model);
	mesh_draw(mesh);
	renderer->frame_stats.shadow_draw_calls++;
}

void renderer_draw_rectangle(renderer_t *renderer,
			     const float x,
			     const float y,
			     const float width,
			     const float height,
			     const renderer_color_t color) {
	if (renderer == NULL) { return; }

	ui_renderer_draw_rectangle(renderer->ui_renderer, x, y, width, height,
				   color);
}

void renderer_draw_text(const renderer_t *renderer,
			const float x,
			const float y,
			const float scale,
			const renderer_color_t color,
			const char *text) {
	if (renderer == NULL) { return; }

	ui_renderer_draw_text(renderer->ui_renderer, x, y, scale, color, text);
}

void renderer_draw_text_with_font(const renderer_t *renderer,
				  const renderer_font_t *font,
				  const float x,
				  const float y,
				  const float height,
				  const renderer_color_t color,
				  const char *text) {
	if (renderer == NULL || font == NULL) { return; }
	ui_renderer_draw_text_with_font(renderer->ui_renderer, font, x, y,
					height, color, text);
}

renderer_frame_stats_t renderer_get_frame_stats(const renderer_t *renderer) {
	if (renderer == NULL) { return (renderer_frame_stats_t){0}; }

	return renderer->frame_stats;
}

void renderer_begin_debug_lines(renderer_t *renderer) {
	if (renderer == NULL) { return; }

	renderer->debug_line_vertex_count = 0;
}

bool renderer_add_debug_line(renderer_t *renderer,
			     const vec3_t start,
			     const vec3_t end,
			     const renderer_color_t color) {
	debug_line_vertex_t *vertices;
	size_t first_index;

	if (renderer == NULL) { return false; }

	if (!renderer_reserve_debug_line_vertices(
		    renderer, renderer->debug_line_vertex_count + 2)) {
		return false;
	}

	first_index = renderer->debug_line_vertex_count;
	vertices = renderer->debug_line_vertices;

	vertices[first_index] = (debug_line_vertex_t){
		.x = start.x,
		.y = start.y,
		.z = start.z,
		.red = color.r,
		.green = color.g,
		.blue = color.b,
		.alpha = color.a,
	};

	vertices[first_index + 1] = (debug_line_vertex_t){
		.x = end.x,
		.y = end.y,
		.z = end.z,
		.red = color.r,
		.green = color.g,
		.blue = color.b,
		.alpha = color.a,
	};

	renderer->debug_line_vertex_count += 2;

	return true;
}

void renderer_flush_debug_lines(renderer_t *renderer,
				const render_view_t *view) {
	GLboolean depth_test_enabled;
	GLboolean blend_enabled;
	GLint previous_blend_source;
	GLint previous_blend_destination;
	size_t data_size;

	if (renderer == NULL || view == NULL ||
	    renderer->debug_line_shader == NULL ||
	    renderer->debug_line_vertex_count == 0) {
		return;
	}

	data_size = renderer->debug_line_vertex_count *
		    sizeof(*renderer->debug_line_vertices);

	depth_test_enabled = glIsEnabled(GL_DEPTH_TEST);
	blend_enabled = glIsEnabled(GL_BLEND);
	glGetIntegerv(GL_BLEND_SRC_RGB, &previous_blend_source);
	glGetIntegerv(GL_BLEND_DST_RGB, &previous_blend_destination);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	shader_bind(renderer->debug_line_shader);
	shader_set_mat4(renderer->debug_line_shader, "view", &view->view);
	shader_set_mat4(renderer->debug_line_shader, "projection",
			&view->projection);

	glBindVertexArray(renderer->debug_line_vertex_array);
	glBindBuffer(GL_ARRAY_BUFFER, renderer->debug_line_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, data_size, renderer->debug_line_vertices,
		     GL_DYNAMIC_DRAW);

	glDrawArrays(GL_LINES, 0, (GLsizei)renderer->debug_line_vertex_count);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	shader_unbind();

	glBlendFunc(previous_blend_source, previous_blend_destination);

	if (!blend_enabled) { glDisable(GL_BLEND); }

	if (!depth_test_enabled) { glDisable(GL_DEPTH_TEST); }

	renderer->debug_line_vertex_count = 0;
}
