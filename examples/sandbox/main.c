/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "asset/manager.h"
#include "core/log.h"
#include "core/path.h"
#include "entity/light_environment.h"
#include "entity/world.h"
#include "game/game.h"
#include "input/input.h"
#include "map/spawn.h"
#include "math/mat4.h"
#include "math/math.h"
#include "math/vec3.h"
#include "physics/character_controller.h"
#include "renderer/renderer.h"
#include "scene/camera.h"
#include "scene/transform.h"
#include "volume.h"
#include <math.h>
#include <stdlib.h>

typedef struct game_state {
	asset_manager_t *assets;
	world_t *world;
	entity_t *mesh_entity;
	light_environment_t *environment_light;
	camera_t camera;
	character_controller_t player;
	vec3_t movement_input;
	float yaw;
	float pitch;
	bool jump_requested;
} game_state_t;

static const float player_move_speed = 4.0f;
static const float player_eye_height = 1.55f;

static bool initialize(engine_t *engine, void *user_data);
static void update(engine_t *engine, float delta_time, void *user_data);
static void render(engine_t *engine, void *user_data);
static void shutdown(engine_t *engine, void *user_data);
static void destroy_game_resources(game_state_t *game_state);
static void fixed_update(engine_t *engine, float delta_time, void *user_data);

static game_state_t state;

static const game_t game = {
	.initialize = initialize,
	.update = update,
	.fixed_update = fixed_update,
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
	config.fixed_delta_time = 1.0f / 60.0f;

	engine = engine_create(&config);
	if (engine == NULL) { return EXIT_FAILURE; }

	result = engine_run(engine);
	engine_destroy(engine);

	return result ? EXIT_SUCCESS : EXIT_FAILURE;
}

static bool initialize(engine_t *engine, void *user_data) {
	game_state_t *game_state;
	entity_t *light_entity;
	char *asset_root;
	char *map_path;
	char error[512];

	(void)engine;

	game_state = user_data;

	asset_root = path_from_executable("assets/game");
	map_path = path_from_executable("assets/game/maps/test.volmap");

	if (asset_root == NULL || map_path == NULL) {
		log_error("Failed to resolve game asset paths");
		free(asset_root);
		free(map_path);
		return false;
	}

	game_state->assets = asset_manager_create_at(asset_root);
	free(asset_root);

	if (game_state->assets == NULL) {
		log_error("Failed to create asset manager");
		free(map_path);
		return false;
	}

	game_state->world = world_create();
	if (game_state->world == NULL) {
		free(map_path);
		destroy_game_resources(game_state);
		return false;
	}

	if (!world_load_map(game_state->world, game_state->assets, map_path,
			    error, sizeof(error))) {
		log_error("Failed to load test map: %s", error);
		free(map_path);
		destroy_game_resources(game_state);
		return false;
	}

	free(map_path);

	game_state->mesh_entity =
		world_find_by_targetname(game_state->world, "rotating_box");

	if (game_state->mesh_entity == NULL) {
		log_error("Map entity \"rotating_box\" was not found");
		destroy_game_resources(game_state);
		return false;
	}

	light_entity =
		world_find_by_classname(game_state->world, "light_environment");
	game_state->environment_light =
		light_environment_from_entity(light_entity);

	if (game_state->environment_light == NULL) {
		log_error("Map has no light_environment entity");
		destroy_game_resources(game_state);
		return false;
	}

	game_state->player = character_controller_create(
		vec3_create(0.0f, -0.9f, 2.0f), 0.35f, 1.7f);
	game_state->movement_input = vec3_create(0.0f, 0.0f, 0.0f);
	game_state->jump_requested = false;

	game_state->camera = camera_create(
		vec3_add(game_state->player.position,
			 vec3_create(0.0f, player_eye_height, 0.0f)));
	game_state->yaw = -PI * 0.5f;
	game_state->pitch = 0.0f;
	log_info("Press E to toggle the example door");

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
	const float mouse_sensitivity = 0.0025f;

	(void)delta_time;

	game_state = user_data;
	input = engine_get_input(engine);

	if (game_state == NULL || input == NULL) { return; }

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

	if (vec3_length(forward) > 0.0f) { forward = vec3_normalize(forward); }

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
		movement =
			vec3_scale(vec3_normalize(movement), player_move_speed);
	}

	game_state->movement_input = movement;

	if (input_key_pressed(input, INPUT_KEY_SPACE)) {
		game_state->jump_requested = true;
	}

	if (input_key_pressed(input, INPUT_KEY_E)) {
		world_send_input(game_state->world, "example_door", "Toggle",
				 "", NULL, NULL);
	}
}

static void
fixed_update(engine_t *engine, const float delta_time, void *user_data) {
	game_state_t *game_state;
	character_move_input_t move_input;
	const collision_world_t *collision_world;
	float wish_speed;

	(void)engine;

	game_state = user_data;

	if (game_state == NULL || game_state->world == NULL) { return; }

	collision_world = world_get_const_collision_world(game_state->world);

	wish_speed = vec3_length(game_state->movement_input);

	move_input.wish_direction = game_state->movement_input;
	move_input.wish_speed = wish_speed;
	move_input.jump = game_state->jump_requested;

	character_controller_move(&game_state->player, collision_world,
				  &move_input, delta_time);

	game_state->jump_requested = false;

	game_state->camera.position =
		vec3_add(game_state->player.position,
			 vec3_create(0.0f, player_eye_height, 0.0f));

	if (game_state->mesh_entity != NULL) {
		game_state->mesh_entity->transform.rotation.x +=
			delta_time * 0.7f;
		game_state->mesh_entity->transform.rotation.y += delta_time;
	}

	world_update(game_state->world, delta_time);
}

static void render(engine_t *engine, void *user_data) {
	game_state_t *game_state;
	renderer_t *renderer;
	render_view_t render_view;
	mat4_t light_view;
	mat4_t light_projection;
	vec3_t light_position;
	vec3_t light_target;
	vec3_t light_up;
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

	render_view.light_direction =
		light_environment_get_direction(game_state->environment_light);
	render_view.light_color =
		light_environment_get_color(game_state->environment_light);
	render_view.light_intensity =
		light_environment_get_intensity(game_state->environment_light);

	light_target = vec3_create(0.0f, 0.0f, 0.0f);
	light_position = vec3_subtract(
		light_target, vec3_scale(render_view.light_direction, 10.0f));

	if (fabsf(render_view.light_direction.y) > 0.99f) {
		light_up = vec3_create(0.0f, 0.0f, 1.0f);
	} else {
		light_up = vec3_create(0.0f, 1.0f, 0.0f);
	}

	light_view = mat4_look_at(light_position, light_target, light_up);
	light_projection =
		mat4_orthographic(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 25.0f);

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

	game_state->mesh_entity = NULL;
	game_state->environment_light = NULL;
	game_state->world = NULL;
	game_state->assets = NULL;
}
