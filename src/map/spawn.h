/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#ifndef VOLUME_MAP_SPAWN_H
#define VOLUME_MAP_SPAWN_H

#include "asset/manager.h"
#include "entity/world.h"
#include "map/map.h"
#include <stdbool.h>
#include <stddef.h>

bool map_spawn_entities(const map_t *map,
			world_t *world,
			const asset_manager_t *assets,
			char *error,
			size_t error_size);

#endif