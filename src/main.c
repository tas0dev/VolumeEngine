/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#include "engine.h"
#include "math/mat4.h"
#include "math/vec3.h"
#include "renderer/mesh.h"
#include "renderer/renderer.h"
#include <stdlib.h>

typedef struct exampleGame {
	mesh_t *mesh;
	float rotation;
} example_game_t;

static mesh_t *create_test_mesh(void);
static bool initialize(engine_t *engine, void *user_data);
static void update(engine_t *engine, float delta_time, void *user_data);
static void render(engine_t *engine, void *user_data);
static void shutdown(engine_t *engine, void *user_data);

int main(void) {
	engine_config_t config = {0};
	example_game_t application = {0};

	config.application_name = "Volume";
	config.window_width = 1280;
	config.window_height = 720;
	config.initialize = initialize;
	config.update = update;
	config.render = render;
	config.shutdown = shutdown;
	config.user_data = &application;

	engine_t *engine = engine_create(&config);
	if (engine == NULL) { return EXIT_FAILURE; }

	if (!engine_run(engine)) {
		engine_destroy(engine);
		return EXIT_FAILURE;
	}

	engine_destroy(engine);

	return EXIT_SUCCESS;
}

static mesh_t *create_test_mesh(void) {
	static const mesh_vertex_t vertices[] = {
		{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
		{{0.5f, -0.5f, -0.5f},  {0.0f, 1.0f, 0.0f}},
		{{0.5f, 0.5f, -0.5f},   {0.0f, 0.0f, 1.0f}},
		{{-0.5f, 0.5f, -0.5f},  {1.0f, 1.0f, 0.0f}},
		{{-0.5f, -0.5f, 0.5f},  {1.0f, 0.0f, 1.0f}},
		{{0.5f, -0.5f, 0.5f},   {0.0f, 1.0f, 1.0f}},
		{{0.5f, 0.5f, 0.5f},    {1.0f, 1.0f, 1.0f}},
		{{-0.5f, 0.5f, 0.5f},   {0.3f, 0.3f, 0.3f}},
	};

	static const unsigned int indices[] = {
		0, 1, 2, 0, 2, 3,

		4, 6, 5, 4, 7, 6,

		0, 3, 7, 0, 7, 4,

		1, 5, 6, 1, 6, 2,

		3, 2, 6, 3, 6, 7,

		0, 4, 5, 0, 5, 1,
	};

	return mesh_create(vertices, sizeof(vertices) / sizeof(vertices[0]),
			   indices, sizeof(indices) / sizeof(indices[0])
	);
}

static bool initialize(engine_t *engine, void *user_data) {
	(void)engine;

	example_game_t *game = user_data;
	game->mesh = create_test_mesh();

	return game->mesh != NULL;
}

static void update(engine_t *engine, const float delta_time, void *user_data) {

	(void)engine;

	example_game_t *game = user_data;
	game->rotation += delta_time;
}

static void render(engine_t *engine, void *user_data) {
	mat4_t model;
	mat4_t view;
	mat4_t projection;
	const float pi = 3.14159265358979323846f;
	const float aspect_ratio = 1280.0f / 720.0f;

	const example_game_t *game = user_data;
	const renderer_t *renderer = engine_get_renderer(engine);

	const mat4_t rotation_x = mat4_rotation_x(game->rotation * 0.7f);
	const mat4_t rotation_y = mat4_rotation_y(game->rotation);
	model = mat4_multiply(rotation_y, rotation_x);
	view = mat4_translation(vec3_create(0.0f, 0.0f, -2.5f));
	projection = mat4_perspective(60.0f * pi / 180.0f, aspect_ratio, 0.1f,
				      100.0f);

	renderer_draw_mesh(renderer, game->mesh, &model, &view, &projection);
}

static void shutdown(engine_t *engine, void *user_data) {
	(void)engine;

	example_game_t *game = user_data;
	mesh_destroy(game->mesh);
	game->mesh = NULL;
}