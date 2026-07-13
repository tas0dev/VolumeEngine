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
#include "renderer/view.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct world world_t;

world_t *world_create(void);
void world_destroy(world_t *world);
entity_t *world_spawn_entity(world_t *world,
			     const char *classname,
			     const entity_properties_t *properties);
bool world_add_entity(world_t *world, entity_t *entity);
bool world_remove_entity(world_t *world, entity_id_t id);
entity_t *world_find_entity(world_t *world, entity_id_t id);
entity_t *world_find_by_classname(world_t *world, const char *classname);
size_t world_get_entity_count(const world_t *world);
entity_t *world_get_entity(world_t *world, size_t index);
void world_update(world_t *world, float delta_time);
void world_draw_shadows(world_t *world, renderer_t *renderer);
void world_draw(world_t *world,
		renderer_t *renderer,
		const render_view_t *view);

#endif