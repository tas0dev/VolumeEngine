/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#include "asset/manager.h"
#include "asset/mesh_loader.h"
#include "core/log.h"
#include "entity/world.h"
#include "game/game.h"
#include "input/input.h"
#include "map/spawn.h"
#include "math/mat4.h"
#include "math/math.h"
#include "math/vec3.h"
#include "renderer/material.h"
#include "renderer/mesh.h"
#include "renderer/renderer.h"
#include "scene/camera.h"
#include "scene/transform.h"
#include "volume.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

typedef struct game_state {
	asset_manager_t *assets;
	world_t *world;
	mesh_t *mesh;
	mesh_t *floor_mesh;
	material_t material;
	material_t floor_material;
	entity_t *mesh_entity;
	camera_t camera;
	float yaw;
	float pitch;
} game_state_t;

static bool initialize(engine_t *engine, void *user_data);
static void update(engine_t *engine, float delta_time, void *user_data);
static void render(engine_t *engine, void *user_data);
static void shutdown(engine_t *engine, void *user_data);
static void destroy_game_resources(game_state_t *game_state);

static game_state_t state;

static const game_t game = {
	.initialize = initialize,
	.update = update,
	.render = render,
	.shutdown = shutdown,
	.user_data = &state,
};

int main(void) {
	engine_config_t config = {0};
	engine_t *engine;
	bool result;

	config.application_name = "Volume";
	config.window_width = 1280;
	config.window_height = 720;
	config.capture_mouse = true;
	config.game = &game;

	engine = engine_create(&config);
	if (engine == NULL) { return EXIT_FAILURE; }

	result = engine_run(engine);
	engine_destroy(engine);

	return result ? EXIT_SUCCESS : EXIT_FAILURE;
}

static bool initialize(engine_t *engine, void *user_data) {
	game_state_t *game_state;
	char error[512];

	(void)engine;

	game_state = user_data;

	game_state->mesh = mesh_load(VOLUME_ASSET_DIR "/models/test_cube.obj",
				     error, sizeof(error));
	if (game_state->mesh == NULL) {
		log_error("Failed to load test cube: %s", error);
		return false;
	}

	game_state->floor_mesh =
		mesh_load(VOLUME_ASSET_DIR "/models/test_floor.obj", error,
			  sizeof(error));
	if (game_state->floor_mesh == NULL) {
		log_error("Failed to load test floor: %s", error);
		destroy_game_resources(game_state);
		return false;
	}

	game_state->material = material_create(vec3_create(1.0f, 1.0f, 1.0f));
	game_state->floor_material =
		material_create(
			vec3_create(0.55f, 0.55f, 0.55f));

	game_state->floor_material.specular_strength = 0.1f;
	game_state->floor_material.shininess = 8.0f;

	game_state->assets = asset_manager_create();
	if (game_state->assets == NULL) {
		destroy_game_resources(game_state);
		return false;
	}

	if (!asset_manager_register_mesh(game_state->assets,
					 "models/test_cube.obj",
					 game_state->mesh) ||
	    !asset_manager_register_mesh(game_state->assets,
					 "models/test_floor.obj",
					 game_state->floor_mesh) ||
	    !asset_manager_register_material(game_state->assets,
					     "materials/test_cube",
					     &game_state->material) ||
	    !asset_manager_register_material(game_state->assets,
					     "materials/floor",
					     &game_state->floor_material)) {
		log_error("Failed to register test assets");
		destroy_game_resources(game_state);
		return false;
	}

	game_state->world = world_create();
	if (game_state->world == NULL) {
		destroy_game_resources(game_state);
		return false;
	}

	if (!world_load_map(
		    game_state->world,
		    game_state->assets,
			    VOLUME_ASSET_DIR "/maps/test.volmap", error,
			    sizeof(error))) {
		log_error("Failed to load test map: %s", error);
		destroy_game_resources(game_state);
		return false;
	}

	game_state->mesh_entity =
		world_find_by_targetname(
			game_state->world,
			"rotating_box"
		);
	if (game_state->mesh_entity == NULL) {
		log_error("Map entity \"rotating_box\" was not found"
		);
		destroy_game_resources(game_state);
		return false;
	}

	game_state->camera = camera_create(
			vec3_create(0.0f, 1.5f, 4.0f)
		);

	game_state->yaw = -PI * 0.5f;
	game_state->pitch = 0.0f;

	return true;
}

static void update(engine_t *engine, const float delta_time, void *user_data) {
	game_state_t *game_state;
	input_t *input;
	vec3_t movement;
	vec3_t forward;
	vec3_t right;
	float mouse_x;
	float mouse_y;
	float pitch_limit;
	const float move_speed = 4.0f;
	const float mouse_sensitivity = 0.0025f;

	game_state = user_data;
	input = engine_get_input(engine);

	if (input == NULL) { return; }

	if (input_key_pressed(input, INPUT_KEY_ESCAPE)) {
		engine_set_mouse_captured(engine, false);
	}

	if (!engine_is_mouse_captured(engine) &&
	    input_mouse_button_pressed(input, INPUT_MOUSE_BUTTON_LEFT)) {
		engine_set_mouse_captured(engine, true);
	}

	mouse_x = 0.0f;
	mouse_y = 0.0f;

	if (engine_is_mouse_captured(engine)) {
		input_get_mouse_delta(input, &mouse_x, &mouse_y);

		game_state->yaw += mouse_x * mouse_sensitivity;
		game_state->pitch -= mouse_y * mouse_sensitivity;
	}

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
		movement = vec3_normalize(movement);
		movement = vec3_scale(movement, move_speed * delta_time);

		game_state->camera.position =
			vec3_add(game_state->camera.position, movement);
	}

	game_state->mesh_entity->transform.rotation.x += delta_time * 0.7f;
	game_state->mesh_entity->transform.rotation.y += delta_time;

	world_update(game_state->world, delta_time);
}

static void render(engine_t *engine, void *user_data) {
	game_state_t *game_state;
	renderer_t *renderer;
	render_view_t render_view;
	mat4_t light_view;
	mat4_t light_projection;
	int width;
	int height;
	float aspect_ratio;

	game_state = user_data;
	renderer = engine_get_renderer(engine);

	renderer_get_size(renderer, &width, &height);
	if (width <= 0 || height <= 0) { return; }

	aspect_ratio = (float)width / (float)height;

	render_view.view = camera_get_view(&game_state->camera);
	render_view.projection =
		camera_get_projection(&game_state->camera, aspect_ratio);

	light_view = mat4_look_at(vec3_create(3.0f, 4.0f, 3.0f),
				  vec3_create(0.0f, 0.0f, 0.0f),
				  vec3_create(0.0f, 1.0f, 0.0f));

	light_projection =
		mat4_orthographic(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 20.0f);

	render_view.light_view_projection =
		mat4_multiply(light_projection, light_view);

	renderer_begin_shadow_pass(renderer,
				   &render_view.light_view_projection);

	world_draw_shadows(game_state->world, renderer);
	renderer_end_shadow_pass(renderer);
	world_draw(game_state->world, renderer, &render_view);
}

static void shutdown(engine_t *engine, void *user_data) {
	(void)engine;

	destroy_game_resources(user_data);
}

static void destroy_game_resources(game_state_t *game_state) {
	if (game_state == NULL) { return; }

	world_destroy(game_state->world);
	asset_manager_destroy(game_state->assets);
	mesh_destroy(game_state->floor_mesh);
	mesh_destroy(game_state->mesh);

	game_state->mesh_entity = NULL;
	game_state->world = NULL;
	game_state->assets = NULL;
	game_state->floor_mesh = NULL;
	game_state->mesh = NULL;
}