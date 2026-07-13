/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#include "game/game.h"
#include "input/input.h"
#include "math/mat4.h"
#include "math/math.h"
#include "math/vec3.h"
#include "renderer/material.h"
#include "renderer/mesh.h"
#include "renderer/renderer.h"
#include "scene/camera.h"
#include "volume.h"

#include <math.h>

typedef struct game_state {
	mesh_t *mesh;
	mesh_t *floor_mesh;
	material_t material;
	material_t floor_material;
	camera_t camera;
	float rotation;
	float yaw;
	float pitch;
} game_state_t;

static mesh_t *create_test_mesh(void);
static mesh_t *create_floor_mesh(void);
static bool initialize(engine_t *engine, void *user_data);
static void update(engine_t *engine, float delta_time, void *user_data);
static void render(engine_t *engine, void *user_data);
static void shutdown(engine_t *engine, void *user_data);

static game_state_t state;

static const game_t game = {
	.initialize = initialize,
	.update = update,
	.render = render,
	.shutdown = shutdown,
	.user_data = &state,
};

const game_t *game_get(void) { return &game; }

static mesh_t *create_test_mesh(void) {
	static const mesh_vertex_t vertices[] = {
		{{-0.5f, -0.5f, 0.5f},  {0.0f, 0.0f, 1.0f},  {1.0f, 0.2f, 0.2f}},
		{{0.5f, -0.5f, 0.5f},   {0.0f, 0.0f, 1.0f},  {1.0f, 0.2f, 0.2f}},
		{{0.5f, 0.5f, 0.5f},    {0.0f, 0.0f, 1.0f},	 {1.0f, 0.2f, 0.2f}},
		{{-0.5f, 0.5f, 0.5f},   {0.0f, 0.0f, 1.0f},  {1.0f, 0.2f, 0.2f}},

		{{0.5f, -0.5f, -0.5f},  {0.0f, 0.0f, -1.0f}, {0.2f, 1.0f, 0.2f}},
		{{-0.5f, -0.5f, -0.5f},
		 {0.0f, 0.0f, -1.0f},
		 {0.2f, 1.0f, 0.2f}					    },
		{{-0.5f, 0.5f, -0.5f},  {0.0f, 0.0f, -1.0f}, {0.2f, 1.0f, 0.2f}},
		{{0.5f, 0.5f, -0.5f},   {0.0f, 0.0f, -1.0f}, {0.2f, 1.0f, 0.2f}},

		{{-0.5f, -0.5f, -0.5f},
		 {-1.0f, 0.0f, 0.0f},
		 {0.2f, 0.4f, 1.0f}					    },
		{{-0.5f, -0.5f, 0.5f},  {-1.0f, 0.0f, 0.0f}, {0.2f, 0.4f, 1.0f}},
		{{-0.5f, 0.5f, 0.5f},   {-1.0f, 0.0f, 0.0f}, {0.2f, 0.4f, 1.0f}},
		{{-0.5f, 0.5f, -0.5f},  {-1.0f, 0.0f, 0.0f}, {0.2f, 0.4f, 1.0f}},

		{{0.5f, -0.5f, 0.5f},   {1.0f, 0.0f, 0.0f},  {1.0f, 1.0f, 0.2f}},
		{{0.5f, -0.5f, -0.5f},  {1.0f, 0.0f, 0.0f},  {1.0f, 1.0f, 0.2f}},
		{{0.5f, 0.5f, -0.5f},   {1.0f, 0.0f, 0.0f},  {1.0f, 1.0f, 0.2f}},
		{{0.5f, 0.5f, 0.5f},    {1.0f, 0.0f, 0.0f},	 {1.0f, 1.0f, 0.2f}},

		{{-0.5f, 0.5f, 0.5f},   {0.0f, 1.0f, 0.0f},  {1.0f, 0.2f, 1.0f}},
		{{0.5f, 0.5f, 0.5f},    {0.0f, 1.0f, 0.0f},	 {1.0f, 0.2f, 1.0f}},
		{{0.5f, 0.5f, -0.5f},   {0.0f, 1.0f, 0.0f},  {1.0f, 0.2f, 1.0f}},
		{{-0.5f, 0.5f, -0.5f},  {0.0f, 1.0f, 0.0f},  {1.0f, 0.2f, 1.0f}},

		{{-0.5f, -0.5f, -0.5f},
		 {0.0f, -1.0f, 0.0f},
		 {0.2f, 1.0f, 1.0f}					    },
		{{0.5f, -0.5f, -0.5f},  {0.0f, -1.0f, 0.0f}, {0.2f, 1.0f, 1.0f}},
		{{0.5f, -0.5f, 0.5f},   {0.0f, -1.0f, 0.0f}, {0.2f, 1.0f, 1.0f}},
		{{-0.5f, -0.5f, 0.5f},  {0.0f, -1.0f, 0.0f}, {0.2f, 1.0f, 1.0f}},
	};

	static const unsigned int indices[] = {
		0,  1,	2,  0,	2,  3,	4,  5,	6,  4,	6,  7,
		8,  9,	10, 8,	10, 11, 12, 13, 14, 12, 14, 15,
		16, 17, 18, 16, 18, 19, 20, 21, 22, 20, 22, 23,
	};

	return mesh_create(vertices, sizeof(vertices) / sizeof(vertices[0]),
			   indices, sizeof(indices) / sizeof(indices[0]));
}

static mesh_t *create_floor_mesh(void) {
	static const mesh_vertex_t vertices[] = {
		{{-5.0f, 0.0f, -5.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
		{{5.0f, 0.0f, -5.0f},  {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
		{{5.0f, 0.0f, 5.0f},   {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
		{{-5.0f, 0.0f, 5.0f},  {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
	};

	static const unsigned int indices[] = {
		0, 2, 1, 0, 3, 2,
	};

	return mesh_create(vertices, sizeof(vertices) / sizeof(vertices[0]),
			   indices, sizeof(indices) / sizeof(indices[0]));
}

static bool initialize(engine_t *engine, void *user_data) {
	game_state_t *game_state;

	(void)engine;

	game_state = user_data;

	game_state->mesh = create_test_mesh();
	if (game_state->mesh == NULL) { return false; }

	game_state->floor_mesh = create_floor_mesh();
	if (game_state->floor_mesh == NULL) {
		mesh_destroy(game_state->mesh);
		game_state->mesh = NULL;
		return false;
	}

	game_state->material = material_create(vec3_create(1.0f, 1.0f, 1.0f));
	game_state->floor_material =
		material_create(vec3_create(0.55f, 0.55f, 0.55f));

	game_state->floor_material.specular_strength = 0.1f;
	game_state->floor_material.shininess = 8.0f;

	game_state->camera = camera_create(vec3_create(0.0f, 1.5f, 4.0f));

	game_state->yaw = -PI * 0.5f;
	game_state->pitch = 0.0f;

	return true;
}

static void update(engine_t *engine, float delta_time, void *user_data) {
	game_state_t *game_state;
	input_t *input;
	vec3_t movement;
	vec3_t forward;
	vec3_t right;
	float mouse_x;
	float mouse_y;
	float mouse_sensitivity;
	float pitch_limit;

	game_state = user_data;
	input = engine_get_input(engine);

	if (input == NULL) { return; }

	game_state->rotation += delta_time;

	mouse_x = 0.0f;
	mouse_y = 0.0f;
	input_get_mouse_delta(input, &mouse_x, &mouse_y);

	mouse_sensitivity = 0.0025f;

	game_state->yaw += mouse_x * mouse_sensitivity;
	game_state->pitch -= mouse_y * mouse_sensitivity;

	pitch_limit = PI * 0.495f;

	if (game_state->pitch > pitch_limit) {
		game_state->pitch = pitch_limit;
	}

	if (game_state->pitch < -pitch_limit) {
		game_state->pitch = -pitch_limit;
	}

	game_state->camera.forward = vec3_normalize(
		vec3_create(cosf(game_state->pitch) * cosf(game_state->yaw),
			    sinf(game_state->pitch),
			    cosf(game_state->pitch) * sinf(game_state->yaw)));

	forward = game_state->camera.forward;
	forward.y = 0.0f;
	forward = vec3_normalize(forward);

	right = vec3_normalize(vec3_cross(forward, game_state->camera.up));

	movement = vec3_create(0.0f, 0.0f, 0.0f);

	if (input_key_down(input, INPUT_KEY_W)) {
		movement = vec3_add(movement, forward);
	}

	if (input_key_down(input, INPUT_KEY_S)) {
		movement = vec3_subtract(movement, forward);
	}

	if (input_key_down(input, INPUT_KEY_D)) {
		movement = vec3_add(movement, right);
	}

	if (input_key_down(input, INPUT_KEY_A)) {
		movement = vec3_subtract(movement, right);
	}

	if (vec3_length(movement) > 0.0f) {
		float move_speed;
		move_speed = 4.0f;
		movement = vec3_normalize(movement);
		movement = vec3_scale(movement, move_speed * delta_time);

		game_state->camera.position =
			vec3_add(game_state->camera.position, movement);
	}
}

static void render(engine_t *engine, void *user_data) {
	game_state_t *game_state;
	renderer_t *renderer;
	mat4_t rotation_x;
	mat4_t rotation_y;
	mat4_t model;
	mat4_t floor_model;
	mat4_t view;
	mat4_t projection;
	mat4_t light_view;
	mat4_t light_projection;
	mat4_t light_view_projection;
	int width;
	int height;
	float aspect_ratio;

	game_state = user_data;
	renderer = engine_get_renderer(engine);

	renderer_get_size(renderer, &width, &height);
	if (width <= 0 || height <= 0) { return; }

	aspect_ratio = (float)width / (float)height;

	rotation_x = mat4_rotation_x(game_state->rotation * 0.7f);
	rotation_y = mat4_rotation_y(game_state->rotation);
	model = mat4_multiply(rotation_y, rotation_x);

	floor_model = mat4_translation(vec3_create(0.0f, -1.0f, 0.0f));

	view = camera_get_view(&game_state->camera);
	projection = camera_get_projection(&game_state->camera, aspect_ratio);

	light_view = mat4_look_at(vec3_create(3.0f, 4.0f, 3.0f),
				  vec3_create(0.0f, 0.0f, 0.0f),
				  vec3_create(0.0f, 1.0f, 0.0f));

	light_projection =
		mat4_orthographic(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 20.0f);

	light_view_projection = mat4_multiply(light_projection, light_view);

	renderer_begin_shadow_pass(renderer, &light_view_projection);
	renderer_draw_shadow_mesh(renderer, game_state->mesh, &model);
	renderer_draw_shadow_mesh(renderer, game_state->floor_mesh,
				  &floor_model);
	renderer_end_shadow_pass(renderer);

	renderer_draw_mesh(renderer, game_state->mesh, &game_state->material,
			   &model, &view, &projection, &light_view_projection);

	renderer_draw_mesh(renderer, game_state->floor_mesh,
			   &game_state->floor_material, &floor_model, &view,
			   &projection, &light_view_projection);
}

static void shutdown(engine_t *engine, void *user_data) {
	game_state_t *game_state;

	(void)engine;

	game_state = user_data;

	mesh_destroy(game_state->floor_mesh);
	mesh_destroy(game_state->mesh);

	game_state->floor_mesh = NULL;
	game_state->mesh = NULL;
}