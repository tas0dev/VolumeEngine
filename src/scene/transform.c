/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#include "scene/transform.h"
#include <stddef.h>

transform_t transform_create(void) {
	transform_t transform;

	transform.position = vec3_create(0.0f, 0.0f, 0.0f);
	transform.rotation = vec3_create(0.0f, 0.0f, 0.0f);
	transform.scale = vec3_create(1.0f, 1.0f, 1.0f);

	return transform;
}

mat4_t transform_get_matrix(const transform_t *transform) {
	mat4_t translation;
	mat4_t rotation_x;
	mat4_t rotation_y;
	mat4_t rotation_z;
	mat4_t rotation;
	mat4_t scale;
	mat4_t model;

	if (transform == NULL) { return mat4_identity(); }

	translation = mat4_translation(transform->position);
	rotation_x = mat4_rotation_x(transform->rotation.x);
	rotation_y = mat4_rotation_y(transform->rotation.y);
	rotation_z = mat4_rotation_z(transform->rotation.z);
	scale = mat4_scale(transform->scale);

	rotation = mat4_multiply(rotation_z, rotation_y);
	rotation = mat4_multiply(rotation, rotation_x);

	model = mat4_multiply(translation, rotation);
	model = mat4_multiply(model, scale);

	return model;
}