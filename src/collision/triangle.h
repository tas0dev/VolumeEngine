/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_COLLISION_TRIANGLE_H
#define VOLUME_COLLISION_TRIANGLE_H
#include "collision/aabb.h"
#include "math/vec3.h"
#include <stdbool.h>

typedef struct triangle {
	vec3_t vertices[3];
	vec3_t normal;
} triangle_t;

triangle_t triangle_create(vec3_t first, vec3_t second, vec3_t third);
aabb_t triangle_get_bounds(triangle_t triangle);
bool aabb_get_triangle_collision(aabb_t aabb,
				 triangle_t triangle,
				 aabb_collision_t *collision);

#endif