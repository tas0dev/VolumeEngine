/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#ifndef VOLUME_ENGINE_H
#define VOLUME_ENGINE_H

#include "core/types.h"
#include <stdbool.h>

typedef bool (*engine_initialize_fn)(engine_t *engine, void *user_data);
typedef void (*engine_update_fn)(engine_t *engine,
				 float delta_time,
				 void *user_data);
typedef void (*engine_render_fn)(engine_t *engine, void *user_data);
typedef void (*engine_shutdown_fn)(engine_t *engine, void *user_data);

typedef struct engine_config {
	const char *application_name;
	int window_width;
	int window_height;
	engine_initialize_fn initialize;
	engine_update_fn update;
	engine_render_fn render;
	engine_shutdown_fn shutdown;
	void *user_data;
} engine_config_t;

engine_t *engine_create(const engine_config_t *config);
void engine_destroy(engine_t *engine);
bool engine_run(engine_t *engine);
renderer_t *engine_get_renderer(engine_t *engine);

#endif
