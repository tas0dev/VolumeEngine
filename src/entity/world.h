/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_ENTITY_WORLD_H
#define VOLUME_ENTITY_WORLD_H

#include "collision/collision_world.h"
#include "entity/entity.h"
#include "renderer/view.h"
#include <stdbool.h>
#include <stddef.h>

world_t *world_create(void);
void world_destroy(world_t *world);
entity_t *world_spawn_entity(world_t *world,
			     const char *classname,
			     const entity_spawn_context_t *context);
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
entity_t *world_find_by_targetname(world_t *world, const char *targetname);
collision_world_t *world_get_collision_world(world_t *world);
const collision_world_t *world_get_const_collision_world(const world_t *world);
size_t world_send_input(world_t *world,
			const char *target_name,
			const char *input_name,
			const char *parameter,
			entity_t *activator,
			entity_t *caller);
bool world_send_input_to_entity(world_t *world,
				entity_t *target,
				const char *input_name,
				const char *parameter,
				entity_t *activator,
				entity_t *caller);
bool world_fire_output(world_t *world,
		       entity_t *caller,
		       const char *output_name,
		       entity_t *activator);
size_t world_get_pending_event_count(const world_t *world);

#endif
