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
#include "game/game.h"
#include <stdbool.h>

typedef struct engine_config {
	const char *application_name;
	int window_width;
	int window_height;
	const game_t *game;
} engine_config_t;

engine_t *engine_create(const engine_config_t *config);
void engine_destroy(engine_t *engine);
bool engine_run(engine_t *engine);
renderer_t *engine_get_renderer(engine_t *engine);

#endif