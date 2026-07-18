/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_COLLISION_COLLISION_WORLD_H
#define VOLUME_COLLISION_COLLISION_WORLD_H

#include "collision/aabb.h"
#include "collision/box_collider.h"
#include "entity/entity.h"
#include <stdbool.h>
#include <stddef.h>

typedef enum collision_side {
	COLLISION_SIDE_NONE = 0,
	COLLISION_SIDE_NEGATIVE_X = 1u << 0,
	COLLISION_SIDE_POSITIVE_X = 1u << 1,
	COLLISION_SIDE_NEGATIVE_Y = 1u << 2,
	COLLISION_SIDE_POSITIVE_Y = 1u << 3,
	COLLISION_SIDE_NEGATIVE_Z = 1u << 4,
	COLLISION_SIDE_POSITIVE_Z = 1u << 5,
} collision_side_t;

typedef struct collision_result {
	vec3_t correction;
	unsigned int sides;
	size_t contact_count;
} collision_result_t;

typedef struct collision_world collision_world_t;

collision_world_t *collision_world_create(void);
void collision_world_destroy(collision_world_t *world);
bool collision_world_add_box(collision_world_t *world,
			     entity_id_t entity_id,
			     box_collider_t collider,
			     vec3_t position);
bool collision_world_remove(collision_world_t *world, entity_id_t entity_id);
size_t collision_world_get_count(const collision_world_t *world);
bool collision_world_resolve_aabb(const collision_world_t *world,
				  aabb_t local_bounds,
				  vec3_t *position,
				  collision_result_t *result);

#endif
