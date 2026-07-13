/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#ifndef VOLUME_GAME_GAME_H
#define VOLUME_GAME_GAME_H

#include "core/types.h"
#include <stdbool.h>

typedef struct game {
	bool (*initialize)(engine_t *engine, void *user_data);
	void (*update)(engine_t *engine, float delta_time, void *user_data);
	void (*render)(engine_t *engine, void *user_data);
	void (*shutdown)(engine_t *engine, void *user_data);
	void *user_data;
} game_t;

const game_t *game_get(void);

#endif