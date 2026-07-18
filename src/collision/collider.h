/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_COLLISION_COLLIDER_H
#define VOLUME_COLLISION_COLLIDER_H

#include "collision/aabb.h"
#include "collision/box_collider.h"
#include "math/vec3.h"
#include <stdbool.h>

typedef enum collider_type {
	COLLIDER_TYPE_NONE,
	COLLIDER_TYPE_BOX,
} collider_type_t;

typedef union collider_shape {
	box_collider_t box;
} collider_shape_t;

typedef struct collider {
	collider_type_t type;
	collider_shape_t shape;
} collider_t;

collider_t collider_create_none(void);
collider_t collider_create_box(vec3_t center, vec3_t half_extents);
bool collider_get_aabb(const collider_t *collider,
		       vec3_t position,
		       aabb_t *aabb);

#endif