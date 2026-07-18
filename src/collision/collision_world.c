/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "collision/collision_world.h"
#include <stdlib.h>
#include <string.h>

typedef struct collision_entry {
	entity_id_t entity_id;
	collider_t collider;
	vec3_t position;
} collision_entry_t;

struct collision_world {
	collision_entry_t *entries;
	size_t count;
	size_t capacity;
};

static bool collision_world_reserve(collision_world_t *world, size_t capacity);
static unsigned int collision_side_from_normal(vec3_t normal);
static void collision_result_reset(collision_result_t *result);

collision_world_t *collision_world_create(void) {
	return calloc(1, sizeof(collision_world_t));
}

void collision_world_destroy(collision_world_t *world) {
	if (world == NULL) { return; }

	free(world->entries);
	free(world);
}

bool collision_world_add_collider(collision_world_t *world,
				  const entity_id_t entity_id,
				  const collider_t collider,
				  const vec3_t position) {
	size_t capacity;
	size_t index;

	if (world == NULL || entity_id == 0 ||
	    collider.type == COLLIDER_TYPE_NONE) {
		return false;
	    }

	for (index = 0; index < world->count; index++) {
		    if (world->entries[index].entity_id == entity_id) {
			return false;
		    }
	}

	if (world->count == world->capacity) {
		capacity = world->capacity == 0 ? 16 : world->capacity * 2;

		if (capacity < world->capacity ||
		    !collision_world_reserve(world, capacity)) {
			return false;
			    }
	}

	world->entries[world->count].entity_id = entity_id;
	world->entries[world->count].collider =
		collider;
	world->entries[world->count].position = position;
	world->count++;

	return true;
}

size_t collision_world_get_count(const collision_world_t *world) {
	if (world == NULL) { return 0; }

	return world->count;
}

bool collision_world_resolve_aabb(const collision_world_t *world,
				  const aabb_t local_bounds,
				  vec3_t *position,
				  collision_result_t *result) {
	const unsigned int maximum_iterations = 8;
	aabb_collision_t collision;
	aabb_t moving_bounds;
	aabb_t static_bounds;
	vec3_t correction;
	bool iteration_collided;
	bool collided;
	unsigned int iteration;
	size_t index;

	if (world == NULL || position == NULL) { return false; }

	collision_result_reset(result);
	collided = false;

	for (iteration = 0; iteration < maximum_iterations; iteration++) {
		iteration_collided = false;
		moving_bounds = aabb_translate(local_bounds, *position);

		for (index = 0; index < world->count; index++) {
			if (!collider_get_aabb(
				    &world->entries[index].collider,
					       world->entries[index].position,
					       &static_bounds)) {
				continue;
			}

			if (!aabb_get_collision(moving_bounds, static_bounds,
						&collision)) {
				continue;
			}

			correction =
				vec3_scale(collision.normal, collision.depth);
			*position = vec3_add(*position, correction);
			moving_bounds = aabb_translate(local_bounds, *position);

			if (result != NULL) {
				result->correction = vec3_add(
					result->correction, correction);
				result->sides |= collision_side_from_normal(
					collision.normal);
				result->contact_count++;
			}

			collided = true;
			iteration_collided = true;
		}

		if (!iteration_collided) { break; }
	}

	return collided;
}

static bool collision_world_reserve(collision_world_t *world,
				    const size_t capacity) {
	collision_entry_t *entries;

	if (capacity <= world->capacity) { return true; }

	if (capacity > SIZE_MAX / sizeof(*world->entries)) { return false; }

	entries = realloc(world->entries, capacity * sizeof(*world->entries));
	if (entries == NULL) { return false; }

	world->entries = entries;
	world->capacity = capacity;

	return true;
}

static unsigned int collision_side_from_normal(const vec3_t normal) {
	if (normal.x < 0.0f) { return COLLISION_SIDE_NEGATIVE_X; }

	if (normal.x > 0.0f) { return COLLISION_SIDE_POSITIVE_X; }

	if (normal.y < 0.0f) { return COLLISION_SIDE_NEGATIVE_Y; }

	if (normal.y > 0.0f) { return COLLISION_SIDE_POSITIVE_Y; }

	if (normal.z < 0.0f) { return COLLISION_SIDE_NEGATIVE_Z; }

	if (normal.z > 0.0f) { return COLLISION_SIDE_POSITIVE_Z; }

	return COLLISION_SIDE_NONE;
}

static void collision_result_reset(collision_result_t *result) {
	if (result == NULL) { return; }

	result->correction = vec3_create(0.0f, 0.0f, 0.0f);
	result->sides = COLLISION_SIDE_NONE;
	result->contact_count = 0;
}

bool collision_world_remove(collision_world_t *world,
			    const entity_id_t entity_id) {
	size_t index;

	if (world == NULL || entity_id == 0) { return false; }

	for (index = 0; index < world->count; index++) {
		if (world->entries[index].entity_id != entity_id) { continue; }

		if (index + 1 < world->count) {
			memmove(&world->entries[index],
				&world->entries[index + 1],
				(world->count - index - 1) *
					sizeof(*world->entries));
		}

		world->count--;
		world->entries[world->count] = (collision_entry_t){0};

		return true;
	}

	return false;
}