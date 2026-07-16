/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_SCENE_CAMERA_H
#define VOLUME_SCENE_CAMERA_H

#include "math/mat4.h"
#include "math/vec3.h"

typedef struct camera {
	vec3_t position;
	vec3_t forward;
	vec3_t up;
	float field_of_view;
	float near_plane;
	float far_plane;
} camera_t;

camera_t camera_create(vec3_t position);
mat4_t camera_get_view(const camera_t *camera);
mat4_t camera_get_projection(const camera_t *camera, float aspect_ratio);

#endif