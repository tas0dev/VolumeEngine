/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#ifndef VOLUME_MATH_MAT4_H
#define VOLUME_MATH_MAT4_H

#include "math/vec3.h"

typedef struct mat4 {
	float elements[16];
} mat4_t;

mat4_t mat4_identity(void);
mat4_t mat4_multiply(mat4_t left, mat4_t right);
mat4_t mat4_translation(vec3_t translation);
mat4_t mat4_rotation_x(float radians);
mat4_t mat4_rotation_y(float radians);
mat4_t mat4_rotation_z(float radians);
mat4_t mat4_perspective(float field_of_view,
			float aspect_ratio,
			float near_plane,
			float far_plane);
const float *mat4_data(const mat4_t *matrix);
mat4_t mat4_look_at(vec3_t position, vec3_t target, vec3_t up);

#endif