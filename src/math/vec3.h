/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_MATH_VEC3_H
#define VOLUME_MATH_VEC3_H

typedef struct vec3 {
	float x;
	float y;
	float z;
} vec3_t;

vec3_t vec3_create(float x, float y, float z);
vec3_t vec3_add(vec3_t left, vec3_t right);
vec3_t vec3_subtract(vec3_t left, vec3_t right);
vec3_t vec3_scale(vec3_t vector, float scalar);
float vec3_dot(vec3_t left, vec3_t right);
vec3_t vec3_cross(vec3_t left, vec3_t right);
float vec3_length(vec3_t vector);
vec3_t vec3_normalize(vec3_t vector);

#endif
