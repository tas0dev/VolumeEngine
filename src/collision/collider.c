/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "collision/collider.h"
#include <stddef.h>

collider_t collider_create_none(void) {
	collider_t collider = {0};

	collider.type = COLLIDER_TYPE_NONE;

	return collider;
}

collider_t collider_create_box(const vec3_t center, const vec3_t half_extents) {
	collider_t collider = {0};

	collider.type = COLLIDER_TYPE_BOX;
	collider.shape.box = box_collider_create(center, half_extents);

	return collider;
}

bool collider_get_aabb(const collider_t *collider,
		       const vec3_t position,
		       aabb_t *aabb) {
	if (collider == NULL || aabb == NULL) { return false; }

	switch (collider->type) {
	case COLLIDER_TYPE_BOX:
		*aabb = box_collider_get_aabb(collider->shape.box, position);
		return true;

	case COLLIDER_TYPE_NONE:
	default: return false;
	}
}
