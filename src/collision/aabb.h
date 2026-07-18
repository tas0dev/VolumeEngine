/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_COLLISION_AABB_H
#define VOLUME_COLLISION_AABB_H

#include "math/vec3.h"
#include <stdbool.h>

typedef struct aabb {
	vec3_t minimum;
	vec3_t maximum;
} aabb_t;

typedef struct aabb_collision {
	vec3_t normal;
	float depth;
} aabb_collision_t;

aabb_t aabb_create(vec3_t center, vec3_t half_extents);
aabb_t aabb_translate(aabb_t aabb, vec3_t offset);
vec3_t aabb_get_center(aabb_t aabb);
vec3_t aabb_get_half_extents(aabb_t aabb);
bool aabb_intersects(aabb_t first, aabb_t second);
bool aabb_get_collision(aabb_t first,
			aabb_t second,
			aabb_collision_t *collision);

#endif