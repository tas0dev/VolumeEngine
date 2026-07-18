/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "collision/aabb.h"
#include <math.h>
#include <stddef.h>

aabb_t aabb_create(const vec3_t center, vec3_t half_extents) {
	aabb_t aabb;

	half_extents.x = fabsf(half_extents.x);
	half_extents.y = fabsf(half_extents.y);
	half_extents.z = fabsf(half_extents.z);

	aabb.minimum = vec3_subtract(center, half_extents);
	aabb.maximum = vec3_add(center, half_extents);

	return aabb;
}

aabb_t aabb_translate(aabb_t aabb, const vec3_t offset) {
	aabb.minimum = vec3_add(aabb.minimum, offset);
	aabb.maximum = vec3_add(aabb.maximum, offset);

	return aabb;
}

vec3_t aabb_get_center(const aabb_t aabb) {
	return vec3_scale(vec3_add(aabb.minimum, aabb.maximum), 0.5f);
}

vec3_t aabb_get_half_extents(const aabb_t aabb) {
	return vec3_scale(vec3_subtract(aabb.maximum, aabb.minimum), 0.5f);
}

bool aabb_intersects(const aabb_t first, const aabb_t second) {
	if (first.maximum.x <= second.minimum.x ||
	    first.minimum.x >= second.maximum.x) {
		return false;
	}

	if (first.maximum.y <= second.minimum.y ||
	    first.minimum.y >= second.maximum.y) {
		return false;
	}

	if (first.maximum.z <= second.minimum.z ||
	    first.minimum.z >= second.maximum.z) {
		return false;
	}

	return true;
}

bool aabb_get_collision(const aabb_t first,
			const aabb_t second,
			aabb_collision_t *collision) {
	float negative_depth;
	float positive_depth;
	float axis_depth;
	vec3_t axis_normal;

	if (!aabb_intersects(first, second)) { return false; }

	if (collision == NULL) { return true; }

	negative_depth = first.maximum.x - second.minimum.x;
	positive_depth = second.maximum.x - first.minimum.x;

	if (negative_depth <= positive_depth) {
		collision->depth = negative_depth;
		collision->normal = vec3_create(-1.0f, 0.0f, 0.0f);
	} else {
		collision->depth = positive_depth;
		collision->normal = vec3_create(1.0f, 0.0f, 0.0f);
	}

	negative_depth = first.maximum.y - second.minimum.y;
	positive_depth = second.maximum.y - first.minimum.y;

	if (negative_depth <= positive_depth) {
		axis_depth = negative_depth;
		axis_normal = vec3_create(0.0f, -1.0f, 0.0f);
	} else {
		axis_depth = positive_depth;
		axis_normal = vec3_create(0.0f, 1.0f, 0.0f);
	}

	if (axis_depth < collision->depth) {
		collision->depth = axis_depth;
		collision->normal = axis_normal;
	}

	negative_depth = first.maximum.z - second.minimum.z;
	positive_depth = second.maximum.z - first.minimum.z;

	if (negative_depth <= positive_depth) {
		axis_depth = negative_depth;
		axis_normal = vec3_create(0.0f, 0.0f, -1.0f);
	} else {
		axis_depth = positive_depth;
		axis_normal = vec3_create(0.0f, 0.0f, 1.0f);
	}

	if (axis_depth < collision->depth) {
		collision->depth = axis_depth;
		collision->normal = axis_normal;
	}

	return true;
}