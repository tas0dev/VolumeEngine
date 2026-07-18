/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "collision/triangle_mesh_collider.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct triangle_mesh_collider {
	triangle_t *triangles;
	size_t triangle_count;
	aabb_t bounds;
};

triangle_mesh_collider_t *
triangle_mesh_collider_create(const triangle_t *triangles,
			      const size_t triangle_count) {
	triangle_mesh_collider_t *collider;
	vec3_t minimum;
	vec3_t maximum;
	vec3_t center;
	vec3_t half_extents;
	size_t triangle_index;
	size_t vertex_index;

	if (triangles == NULL || triangle_count == 0) { return NULL; }

	if (triangle_count > SIZE_MAX / sizeof(*collider->triangles)) {
		return NULL;
	}

	collider = calloc(1, sizeof(*collider));
	if (collider == NULL) { return NULL; }

	collider->triangles =
		malloc(triangle_count * sizeof(*collider->triangles));

	if (collider->triangles == NULL) {
		free(collider);
		return NULL;
	}

	memcpy(collider->triangles, triangles,
	       triangle_count * sizeof(*collider->triangles));

	collider->triangle_count = triangle_count;
	minimum = triangles[0].vertices[0];
	maximum = triangles[0].vertices[0];

	for (triangle_index = 0; triangle_index < triangle_count;
	     triangle_index++) {
		for (vertex_index = 0; vertex_index < 3; vertex_index++) {
			const vec3_t vertex = triangles[triangle_index]
						      .vertices[vertex_index];

			if (vertex.x < minimum.x) { minimum.x = vertex.x; }

			if (vertex.y < minimum.y) { minimum.y = vertex.y; }

			if (vertex.z < minimum.z) { minimum.z = vertex.z; }

			if (vertex.x > maximum.x) { maximum.x = vertex.x; }

			if (vertex.y > maximum.y) { maximum.y = vertex.y; }

			if (vertex.z > maximum.z) { maximum.z = vertex.z; }
		}
	}

	center = vec3_scale(vec3_add(minimum, maximum), 0.5f);
	half_extents = vec3_scale(vec3_subtract(maximum, minimum), 0.5f);
	collider->bounds = aabb_create(center, half_extents);

	return collider;
}

void triangle_mesh_collider_destroy(triangle_mesh_collider_t *collider) {
	if (collider == NULL) { return; }

	free(collider->triangles);
	free(collider);
}

size_t triangle_mesh_collider_get_triangle_count(
	const triangle_mesh_collider_t *collider) {
	if (collider == NULL) { return 0; }

	return collider->triangle_count;
}

bool triangle_mesh_collider_get_triangle(
	const triangle_mesh_collider_t *collider,
	const size_t index,
	triangle_t *triangle) {
	if (collider == NULL || triangle == NULL ||
	    index >= collider->triangle_count) {
		return false;
	}

	*triangle = collider->triangles[index];

	return true;
}

bool triangle_mesh_collider_get_bounds(const triangle_mesh_collider_t *collider,
				       aabb_t *bounds) {
	if (collider == NULL || bounds == NULL) { return false; }

	*bounds = collider->bounds;

	return true;
}
