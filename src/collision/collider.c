/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "collision/collider.h"

static aabb_t transform_aabb(const aabb_t bounds, const mat4_t transform) {
	vec3_t corners[8];
	vec3_t minimum;
	vec3_t maximum;
	vec3_t transformed;
	vec3_t center;
	vec3_t half_extents;
	size_t index;

	corners[0] = vec3_create(bounds.minimum.x, bounds.minimum.y,
				 bounds.minimum.z);
	corners[1] = vec3_create(bounds.maximum.x, bounds.minimum.y,
				 bounds.minimum.z);
	corners[2] = vec3_create(bounds.minimum.x, bounds.maximum.y,
				 bounds.minimum.z);
	corners[3] = vec3_create(bounds.maximum.x, bounds.maximum.y,
				 bounds.minimum.z);
	corners[4] = vec3_create(bounds.minimum.x, bounds.minimum.y,
				 bounds.maximum.z);
	corners[5] = vec3_create(bounds.maximum.x, bounds.minimum.y,
				 bounds.maximum.z);
	corners[6] = vec3_create(bounds.minimum.x, bounds.maximum.y,
				 bounds.maximum.z);
	corners[7] = vec3_create(bounds.maximum.x, bounds.maximum.y,
				 bounds.maximum.z);

	minimum = mat4_transform_point(transform, corners[0]);
	maximum = minimum;

	for (index = 1; index < 8; index++) {
		transformed = mat4_transform_point(transform, corners[index]);

		if (transformed.x < minimum.x) { minimum.x = transformed.x; }

		if (transformed.y < minimum.y) { minimum.y = transformed.y; }

		if (transformed.z < minimum.z) { minimum.z = transformed.z; }

		if (transformed.x > maximum.x) { maximum.x = transformed.x; }

		if (transformed.y > maximum.y) { maximum.y = transformed.y; }

		if (transformed.z > maximum.z) { maximum.z = transformed.z; }
	}

	center = vec3_scale(vec3_add(minimum, maximum), 0.5f);
	half_extents = vec3_scale(vec3_subtract(maximum, minimum), 0.5f);

	return aabb_create(center, half_extents);
}

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

	case COLLIDER_TYPE_TRIANGLE_MESH:
		*aabb = aabb_translate(collider->shape.triangle_mesh.bounds,
				       position);
		return true;

	case COLLIDER_TYPE_NONE:
	default: return false;
	}
}

collider_t collider_create_triangle_mesh(const triangle_mesh_collider_t *mesh,
					 const mat4_t transform) {
	collider_t collider;
	aabb_t bounds;

	collider = collider_create_none();

	if (mesh == NULL || !triangle_mesh_collider_get_bounds(mesh, &bounds)) {
		return collider;
	}

	collider.type = COLLIDER_TYPE_TRIANGLE_MESH;
	collider.shape.triangle_mesh.mesh = mesh;
	collider.shape.triangle_mesh.transform = transform;
	collider.shape.triangle_mesh.bounds = transform_aabb(bounds, transform);

	return collider;
}