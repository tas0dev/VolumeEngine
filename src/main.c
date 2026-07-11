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
#include "renderer/material.h"
#include "renderer/mesh.h"
#include "renderer/renderer.h"
#include "scene/camera.h"
#include <stdlib.h>

typedef struct example_game {
	mesh_t *mesh;
	material_t material;
	camera_t camera;
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
		indices,
		sizeof(indices) / sizeof(indices[0])
	);
}

static bool initialize(engine_t *engine, void *user_data) {
	(void)engine;

	example_game_t *game = user_data;
	game->mesh = create_test_mesh();
	if (game->mesh == NULL) { return false; }

	game->material = material_create(vec3_create(1.0f, 1.0f, 1.0f));

	game->material.specular_strength = 1.0f;
	game->material.shininess = 128.0f;

	game->camera = camera_create(vec3_create(0.0f, 0.0f, 2.5f));

	return true;
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
	int width;
	int height;

	const example_game_t *game = user_data;
	renderer_t *renderer = engine_get_renderer(engine);

	renderer_get_size(renderer, &width, &height);
	if (width <= 0 || height <= 0) { return; }

	const float aspect_ratio = (float)width / (float)height;

	const mat4_t rotation_x = mat4_rotation_x(game->rotation * 0.7f);
	const mat4_t rotation_y = mat4_rotation_y(game->rotation);
	model = mat4_multiply(rotation_y, rotation_x);
	view = camera_get_view(&game->camera);
	projection = camera_get_projection(&game->camera, aspect_ratio);

	renderer_draw_mesh(renderer, game->mesh, &game->material, &model, &view,
			   &projection);
}

static void shutdown(engine_t *engine, void *user_data) {
	(void)engine;

	example_game_t *game = user_data;
	mesh_destroy(game->mesh);
	game->mesh = NULL;
}