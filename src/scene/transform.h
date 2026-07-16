/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_SCENE_TRANSFORM_H
#define VOLUME_SCENE_TRANSFORM_H

#include "math/mat4.h"
#include "math/vec3.h"

typedef struct transform {
	vec3_t position;
	vec3_t rotation;
	vec3_t scale;
} transform_t;

transform_t transform_create(void);
mat4_t transform_get_matrix(const transform_t *transform);

#endif