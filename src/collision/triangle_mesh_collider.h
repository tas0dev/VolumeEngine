/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_COLLISION_TRIANGLE_MESH_COLLIDER_H
#define VOLUME_COLLISION_TRIANGLE_MESH_COLLIDER_H
#include "collision/aabb.h"
#include "collision/triangle.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct triangle_mesh_collider triangle_mesh_collider_t;

triangle_mesh_collider_t *
triangle_mesh_collider_create(const triangle_t *triangles,
			      size_t triangle_count);
void triangle_mesh_collider_destroy(triangle_mesh_collider_t *collider);
size_t triangle_mesh_collider_get_triangle_count(
	const triangle_mesh_collider_t *collider);
bool triangle_mesh_collider_get_triangle(
	const triangle_mesh_collider_t *collider,
	size_t index,
	triangle_t *triangle);
bool triangle_mesh_collider_get_bounds(const triangle_mesh_collider_t *collider,
				       aabb_t *bounds);

#endif