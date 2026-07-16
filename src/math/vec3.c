/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "math/vec3.h"
#include <math.h>

vec3_t vec3_create(const float x, const float y, const float z) {
	vec3_t result;

	result.x = x;
	result.y = y;
	result.z = z;

	return result;
}

vec3_t vec3_add(const vec3_t left, const vec3_t right) {
	return vec3_create(left.x + right.x, left.y + right.y,
			   left.z + right.z);
}

vec3_t vec3_subtract(const vec3_t left, const vec3_t right) {
	return vec3_create(left.x - right.x, left.y - right.y,
			   left.z - right.z);
}

vec3_t vec3_scale(const vec3_t vector, const float scalar) {
	return vec3_create(vector.x * scalar, vector.y * scalar,
			   vector.z * scalar);
}

float vec3_dot(const vec3_t left, const vec3_t right) {
	return left.x * right.x + left.y * right.y + left.z * right.z;
}

vec3_t vec3_cross(const vec3_t left, const vec3_t right) {
	return vec3_create(left.y * right.z - left.z * right.y,
			   left.z * right.x - left.x * right.z,
			   left.x * right.y - left.y * right.x);
}

float vec3_length(const vec3_t vector) {
	return sqrtf(vec3_dot(vector, vector));
}

vec3_t vec3_normalize(const vec3_t vector) {
	const float length = vec3_length(vector);
	if (length == 0.0f) { return vec3_create(0.0f, 0.0f, 0.0f); }

	return vec3_scale(vector, 1.0f / length);
}