/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_COLLISION_COLLISION_WORLD_H
#define VOLUME_COLLISION_COLLISION_WORLD_H

#include "collision/aabb.h"
#include "collision/collider.h"
#include "core/types.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define COLLISION_RESULT_MAX_CONTACTS 16

typedef uint32_t collision_layer_t;

#define COLLISION_LAYER_NONE ((collision_layer_t)0)
#define COLLISION_LAYER_WORLD_STATIC ((collision_layer_t)1u << 0)
#define COLLISION_LAYER_DYNAMIC ((collision_layer_t)1u << 1)
#define COLLISION_LAYER_PLAYER ((collision_layer_t)1u << 2)
#define COLLISION_LAYER_TRIGGER ((collision_layer_t)1u << 3)
#define COLLISION_LAYER_ALL UINT32_MAX

typedef struct collision_filter {
	collision_layer_t layer;
	collision_layer_t mask;
	entity_id_t ignored_entity_id;
} collision_filter_t;

typedef struct collision_contact {
	vec3_t normal;
	float depth;
	entity_id_t entity_id;
} collision_contact_t;

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
	collision_contact_t contacts[COLLISION_RESULT_MAX_CONTACTS];
} collision_result_t;

typedef struct collision_trace {
	bool hit;
	bool started_inside;
	float fraction;
	vec3_t position;
	vec3_t normal;
	entity_id_t entity_id;
} collision_trace_t;

typedef struct collision_world collision_world_t;

collision_world_t *collision_world_create(void);
void collision_world_destroy(collision_world_t *world);
bool collision_world_add_collider(collision_world_t *world,
				  entity_id_t entity_id,
				  collider_t collider,
				  vec3_t position);
bool collision_world_add_collider_filtered(collision_world_t *world,
					   entity_id_t entity_id,
					   collider_t collider,
					   vec3_t position,
					   collision_layer_t layer,
					   collision_layer_t mask);
bool collision_world_update_collider(collision_world_t *world,
				     entity_id_t entity_id,
				     collider_t collider,
				     vec3_t position);
bool collision_world_update_collider_filtered(collision_world_t *world,
					      entity_id_t entity_id,
					      collider_t collider,
					      vec3_t position,
					      collision_layer_t layer,
					      collision_layer_t mask);
bool collision_world_remove(collision_world_t *world, entity_id_t entity_id);
size_t collision_world_get_count(const collision_world_t *world);
bool collision_world_resolve_aabb(const collision_world_t *world,
				  aabb_t local_bounds,
				  vec3_t *position,
				  collision_result_t *result);
bool collision_world_resolve_aabb_ignoring(const collision_world_t *world,
					   aabb_t local_bounds,
					   vec3_t *position,
					   entity_id_t ignored_entity_id,
					   collision_result_t *result);
bool collision_world_resolve_aabb_filtered(const collision_world_t *world,
					   aabb_t local_bounds,
					   vec3_t *position,
					   collision_filter_t filter,
					   collision_result_t *result);
bool collision_world_trace_aabb(const collision_world_t *world,
				aabb_t local_bounds,
				vec3_t start,
				vec3_t end,
				collision_trace_t *trace);
bool collision_world_trace_aabb_ignoring(const collision_world_t *world,
					 aabb_t local_bounds,
					 vec3_t start,
					 vec3_t end,
					 entity_id_t ignored_entity_id,
					 collision_trace_t *trace);
bool collision_world_trace_aabb_filtered(const collision_world_t *world,
					 aabb_t local_bounds,
					 vec3_t start,
					 vec3_t end,
					 collision_filter_t filter,
					 collision_trace_t *trace);

#endif
