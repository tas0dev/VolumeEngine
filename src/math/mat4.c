/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#include "math/mat4.h"
#include <math.h>
#include <stddef.h>

mat4_t mat4_identity(void) {
	mat4_t result = {0};

	result.elements[0] = 1.0f;
	result.elements[5] = 1.0f;
	result.elements[10] = 1.0f;
	result.elements[15] = 1.0f;

	return result;
}

mat4_t mat4_multiply(const mat4_t left, const mat4_t right) {
	mat4_t result = {0};

	for (size_t column = 0; column < 4; column++) {
		for (size_t row = 0; row < 4; row++) {
			for (size_t index = 0; index < 4; index++) {
				result.elements[column * 4 + row] +=
					left.elements[index * 4 + row] *
					right.elements[column * 4 + index];
			}
		}
	}

	return result;
}

mat4_t mat4_translation(const vec3_t translation) {
	mat4_t result = mat4_identity();
	result.elements[12] = translation.x;
	result.elements[13] = translation.y;
	result.elements[14] = translation.z;

	return result;
}

mat4_t mat4_rotation_x(const float radians) {
	const float cosine = cosf(radians);
	const float sine = sinf(radians);

	mat4_t result = mat4_identity();
	result.elements[5] = cosine;
	result.elements[6] = sine;
	result.elements[9] = -sine;
	result.elements[10] = cosine;

	return result;
}

mat4_t mat4_rotation_y(const float radians) {
	const float cosine = cosf(radians);
	const float sine = sinf(radians);

	mat4_t result = mat4_identity();
	result.elements[0] = cosine;
	result.elements[2] = -sine;
	result.elements[8] = sine;
	result.elements[10] = cosine;

	return result;
}

mat4_t mat4_rotation_z(const float radians) {
	const float cosine = cosf(radians);
	const float sine = sinf(radians);

	mat4_t result = mat4_identity();
	result.elements[0] = cosine;
	result.elements[1] = sine;
	result.elements[4] = -sine;
	result.elements[5] = cosine;

	return result;
}

mat4_t mat4_perspective(const float field_of_view,
			const float aspect_ratio,
			const float near_plane,
			const float far_plane) {
	mat4_t result = {0};

	const float tangent = tanf(field_of_view * 0.5f);

	result.elements[0] = 1.0f / (aspect_ratio * tangent);
	result.elements[5] = 1.0f / tangent;
	result.elements[10] =
		-(far_plane + near_plane) / (far_plane - near_plane);
	result.elements[11] = -1.0f;
	result.elements[14] =
		-(2.0f * far_plane * near_plane) / (far_plane - near_plane);

	return result;
}

const float *mat4_data(const mat4_t *matrix) {
	if (matrix == NULL) { return NULL; }

	return matrix->elements;
}

mat4_t
mat4_look_at(const vec3_t position, const vec3_t target, const vec3_t up) {
	const vec3_t forward = vec3_normalize(vec3_subtract(target, position));
	const vec3_t right = vec3_normalize(vec3_cross(forward, up));
	const vec3_t camera_up = vec3_cross(right, forward);
	mat4_t result = mat4_identity();

	result.elements[0] = right.x;
	result.elements[1] = camera_up.x;
	result.elements[2] = -forward.x;

	result.elements[4] = right.y;
	result.elements[5] = camera_up.y;
	result.elements[6] = -forward.y;

	result.elements[8] = right.z;
	result.elements[9] = camera_up.z;
	result.elements[10] = -forward.z;

	result.elements[12] = -vec3_dot(right, position);
	result.elements[13] = -vec3_dot(camera_up, position);
	result.elements[14] = vec3_dot(forward, position);

	return result;
}

mat4_t mat4_orthographic(const float left,
			 const float right,
			 const float bottom,
			 const float top,
			 const float near_plane,
			 const float far_plane) {
	mat4_t result = {0};

	if (left == right || bottom == top || near_plane == far_plane) {
		return mat4_identity();
	}

	result.elements[0] = 2.0f / (right - left);
	result.elements[5] = 2.0f / (top - bottom);
	result.elements[10] = -2.0f / (far_plane - near_plane);
	result.elements[12] = -(right + left) / (right - left);
	result.elements[13] = -(top + bottom) / (top - bottom);
	result.elements[14] =
		-(far_plane + near_plane) / (far_plane - near_plane);
	result.elements[15] = 1.0f;

	return result;
}
