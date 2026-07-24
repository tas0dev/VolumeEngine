/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "scene/transform.h"
#include <math.h>
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

bool transform_from_matrix(const mat4_t *matrix, transform_t *transform) {
	const float epsilon = 0.0001f;
	vec3_t axis_x;
	vec3_t axis_y;
	vec3_t axis_z;
	float scale_x;
	float scale_y;
	float scale_z;
	float dot_xy;
	float dot_xz;
	float dot_yz;
	float determinant;
	float cosine_y;
	float rotation_x;
	float rotation_y;
	float rotation_z;
	transform_t result;

	if (matrix == NULL || transform == NULL) { return false; }

	if (fabsf(matrix->elements[3]) > epsilon ||
	    fabsf(matrix->elements[7]) > epsilon ||
	    fabsf(matrix->elements[11]) > epsilon ||
	    fabsf(matrix->elements[15] - 1.0f) > epsilon) {
		return false;
	}

	axis_x = vec3_create(matrix->elements[0], matrix->elements[1],
			     matrix->elements[2]);
	axis_y = vec3_create(matrix->elements[4], matrix->elements[5],
			     matrix->elements[6]);
	axis_z = vec3_create(matrix->elements[8], matrix->elements[9],
			     matrix->elements[10]);

	scale_x = vec3_length(axis_x);
	scale_y = vec3_length(axis_y);
	scale_z = vec3_length(axis_z);

	if (scale_x <= epsilon || scale_y <= epsilon || scale_z <= epsilon) {
		return false;
	}

	axis_x = vec3_scale(axis_x, 1.0f / scale_x);
	axis_y = vec3_scale(axis_y, 1.0f / scale_y);
	axis_z = vec3_scale(axis_z, 1.0f / scale_z);

	dot_xy = vec3_dot(axis_x, axis_y);
	dot_xz = vec3_dot(axis_x, axis_z);
	dot_yz = vec3_dot(axis_y, axis_z);

	if (fabsf(dot_xy) > epsilon || fabsf(dot_xz) > epsilon ||
	    fabsf(dot_yz) > epsilon) {
		return false;
	}

	determinant = vec3_dot(axis_x, vec3_cross(axis_y, axis_z));

	if (determinant <= 0.0f || fabsf(determinant - 1.0f) > epsilon) {
		return false;
	}

	rotation_y = asinf(-axis_x.z);
	cosine_y = cosf(rotation_y);

	if (fabsf(cosine_y) > epsilon) {
		rotation_x = atan2f(axis_y.z, axis_z.z);
		rotation_z = atan2f(axis_x.y, axis_x.x);
	} else {
		rotation_x = 0.0f;
		rotation_z = atan2f(-axis_y.x, axis_y.y);
	}

	result = transform_create();
	result.position =
		vec3_create(matrix->elements[12], matrix->elements[13],
			    matrix->elements[14]);
	result.rotation = vec3_create(rotation_x, rotation_y, rotation_z);
	result.scale = vec3_create(scale_x, scale_y, scale_z);

	*transform = result;

	return true;
}