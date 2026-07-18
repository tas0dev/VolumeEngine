/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_COLLISION_BOX_COLLIDER_H
#define VOLUME_COLLISION_BOX_COLLIDER_H

#include "collision/aabb.h"
#include "math/vec3.h"

typedef struct box_collider {
	vec3_t center;
	vec3_t half_extents;
} box_collider_t;

box_collider_t box_collider_create(vec3_t center, vec3_t half_extents);
aabb_t box_collider_get_aabb(box_collider_t collider, vec3_t position);

#endif