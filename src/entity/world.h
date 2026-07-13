/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#ifndef VOLUME_ENTITY_WORLD_H
#define VOLUME_ENTITY_WORLD_H

#include "entity/entity.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct world world_t;

world_t *world_create(void);
void world_destroy(world_t *world);
bool world_add_entity(world_t *world, entity_t *entity);
bool world_remove_entity(world_t *world, entity_id_t id);
entity_t *world_find_entity(world_t *world, entity_id_t id);
entity_t *world_find_by_classname(world_t *world, const char *classname);
size_t world_get_entity_count(const world_t *world);
entity_t *world_get_entity(world_t *world, size_t index);

#endif