/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_MAP_MAP_H
#define VOLUME_MAP_MAP_H

#include "math/vec3.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct map map_t;
typedef struct map_entity map_entity_t;

map_t *map_parse(const char *source, char *error, size_t error_size);
map_t *map_load(const char *path, char *error, size_t error_size);
void map_destroy(map_t *map);
const map_entity_t *map_get_world(const map_t *map);
size_t map_get_entity_count(const map_t *map);
const map_entity_t *map_get_entity(const map_t *map, size_t index);
const char *map_entity_get_property(const map_entity_t *entity,
				    const char *key);
bool map_entity_get_vec3(const map_entity_t *entity,
			 const char *key,
			 vec3_t *value);
bool map_entity_get_bool(const map_entity_t *entity,
			 const char *key,
			 bool *value);
bool map_entity_get_float(const map_entity_t *entity,
			  const char *key,
			  float *value);

#endif