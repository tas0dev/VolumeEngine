/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
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

mat4_t mat4_scale(const vec3_t scale) {
	mat4_t result = mat4_identity();

	result.elements[0] = scale.x;
	result.elements[5] = scale.y;
	result.elements[10] = scale.z;

	return result;
}

vec3_t mat4_transform_point(const mat4_t matrix, const vec3_t point) {
	vec3_t result;

	result.x = matrix.elements[0] * point.x + matrix.elements[4] * point.y +
		   matrix.elements[8] * point.z + matrix.elements[12];
	result.y = matrix.elements[1] * point.x + matrix.elements[5] * point.y +
		   matrix.elements[9] * point.z + matrix.elements[13];
	result.z = matrix.elements[2] * point.x + matrix.elements[6] * point.y +
		   matrix.elements[10] * point.z + matrix.elements[14];

	return result;
}

bool mat4_inverse_affine(const mat4_t *matrix, mat4_t *inverse) {
	const float epsilon = 0.000001f;
	float a00;
	float a01;
	float a02;
	float a10;
	float a11;
	float a12;
	float a20;
	float a21;
	float a22;
	float determinant;
	float inverse_determinant;
	float translation_x;
	float translation_y;
	float translation_z;
	mat4_t result;

	if (matrix == NULL || inverse == NULL) { return false; }

	if (fabsf(matrix->elements[3]) > epsilon ||
	    fabsf(matrix->elements[7]) > epsilon ||
	    fabsf(matrix->elements[11]) > epsilon ||
	    fabsf(matrix->elements[15] - 1.0f) > epsilon) {
		return false;
	}

	a00 = matrix->elements[0];
	a01 = matrix->elements[4];
	a02 = matrix->elements[8];
	a10 = matrix->elements[1];
	a11 = matrix->elements[5];
	a12 = matrix->elements[9];
	a20 = matrix->elements[2];
	a21 = matrix->elements[6];
	a22 = matrix->elements[10];

	determinant = a00 * (a11 * a22 - a12 * a21) -
		      a01 * (a10 * a22 - a12 * a20) +
		      a02 * (a10 * a21 - a11 * a20);

	if (fabsf(determinant) <= epsilon) { return false; }

	inverse_determinant = 1.0f / determinant;
	result = mat4_identity();

	result.elements[0] = (a11 * a22 - a12 * a21) * inverse_determinant;
	result.elements[4] = (a02 * a21 - a01 * a22) * inverse_determinant;
	result.elements[8] = (a01 * a12 - a02 * a11) * inverse_determinant;

	result.elements[1] = (a12 * a20 - a10 * a22) * inverse_determinant;
	result.elements[5] = (a00 * a22 - a02 * a20) * inverse_determinant;
	result.elements[9] = (a02 * a10 - a00 * a12) * inverse_determinant;

	result.elements[2] = (a10 * a21 - a11 * a20) * inverse_determinant;
	result.elements[6] = (a01 * a20 - a00 * a21) * inverse_determinant;
	result.elements[10] = (a00 * a11 - a01 * a10) * inverse_determinant;

	translation_x = matrix->elements[12];
	translation_y = matrix->elements[13];
	translation_z = matrix->elements[14];

	result.elements[12] = -(result.elements[0] * translation_x +
				result.elements[4] * translation_y +
				result.elements[8] * translation_z);
	result.elements[13] = -(result.elements[1] * translation_x +
				result.elements[5] * translation_y +
				result.elements[9] * translation_z);
	result.elements[14] = -(result.elements[2] * translation_x +
				result.elements[6] * translation_y +
				result.elements[10] * translation_z);

	*inverse = result;

	return true;
}