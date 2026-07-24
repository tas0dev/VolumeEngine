/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "collision/box_collider.h"
#include <math.h>
#include <stddef.h>

box_collider_t box_collider_create(const vec3_t center, vec3_t half_extents) {
	return box_collider_create_transformed(center, half_extents,
					       mat4_identity());
}

aabb_t box_collider_get_aabb(const box_collider_t collider,
			     const vec3_t position) {
	vec3_t center;
	vec3_t axes[3];
	vec3_t half_extents;
	vec3_t world_half_extents;

	if (!box_collider_get_world_box(collider, position, &center, axes,
					&half_extents)) {
		return aabb_create(position, vec3_create(0.0f, 0.0f, 0.0f));
	}

	world_half_extents.x = fabsf(axes[0].x) * half_extents.x +
			       fabsf(axes[1].x) * half_extents.y +
			       fabsf(axes[2].x) * half_extents.z;
	world_half_extents.y = fabsf(axes[0].y) * half_extents.x +
			       fabsf(axes[1].y) * half_extents.y +
			       fabsf(axes[2].y) * half_extents.z;
	world_half_extents.z = fabsf(axes[0].z) * half_extents.x +
			       fabsf(axes[1].z) * half_extents.y +
			       fabsf(axes[2].z) * half_extents.z;

	return aabb_create(center, world_half_extents);
}

box_collider_t box_collider_create_transformed(const vec3_t center,
					       vec3_t half_extents,
					       const mat4_t transform) {
	box_collider_t collider;

	half_extents.x = fabsf(half_extents.x);
	half_extents.y = fabsf(half_extents.y);
	half_extents.z = fabsf(half_extents.z);

	collider.center = center;
	collider.half_extents = half_extents;
	collider.transform = transform;

	return collider;
}

bool box_collider_get_world_box(const box_collider_t collider,
				const vec3_t position,
				vec3_t *center,
				vec3_t axes[3],
				vec3_t *half_extents) {
	const float epsilon = 0.000001f;
	vec3_t axis_x;
	vec3_t axis_y;
	vec3_t axis_z;
	float length_x;
	float length_y;
	float length_z;

	if (center == NULL || axes == NULL || half_extents == NULL) {
		return false;
	}

	axis_x = vec3_create(collider.transform.elements[0],
			     collider.transform.elements[1],
			     collider.transform.elements[2]);
	axis_y = vec3_create(collider.transform.elements[4],
			     collider.transform.elements[5],
			     collider.transform.elements[6]);
	axis_z = vec3_create(collider.transform.elements[8],
			     collider.transform.elements[9],
			     collider.transform.elements[10]);

	length_x = vec3_length(axis_x);
	length_y = vec3_length(axis_y);
	length_z = vec3_length(axis_z);

	if (length_x <= epsilon || length_y <= epsilon ||
	    length_z <= epsilon) {
		return false;
	    }

	axes[0] = vec3_scale(axis_x, 1.0f / length_x);
	axes[1] = vec3_scale(axis_y, 1.0f / length_y);
	axes[2] = vec3_scale(axis_z, 1.0f / length_z);

	*center = vec3_add(
		mat4_transform_point(collider.transform, collider.center),
		position);

	*half_extents = vec3_create(
		collider.half_extents.x * length_x,
		collider.half_extents.y * length_y,
		collider.half_extents.z * length_z);

	return true;
}