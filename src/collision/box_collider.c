/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "collision/box_collider.h"
#include <math.h>

box_collider_t box_collider_create(const vec3_t center, vec3_t half_extents) {
	box_collider_t collider;

	half_extents.x = fabsf(half_extents.x);
	half_extents.y = fabsf(half_extents.y);
	half_extents.z = fabsf(half_extents.z);

	collider.center = center;
	collider.half_extents = half_extents;

	return collider;
}

aabb_t box_collider_get_aabb(const box_collider_t collider,
			     const vec3_t position) {
	const vec3_t center = vec3_add(position, collider.center);

	return aabb_create(center, collider.half_extents);
}