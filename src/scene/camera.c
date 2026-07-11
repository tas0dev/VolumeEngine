/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#include "scene/camera.h"
#include "math/math.h"

#include <stddef.h>

camera_t camera_create(const vec3_t position) {
	camera_t camera;

	camera.position = position;
	camera.forward = vec3_create(0.0f, 0.0f, -1.0f);
	camera.up = vec3_create(0.0f, 1.0f, 0.0f);
	camera.field_of_view = 60.0f;
	camera.near_plane = 0.1f;
	camera.far_plane = 100.0f;

	return camera;
}

mat4_t camera_get_view(const camera_t *camera) {
	if (camera == NULL) { return mat4_identity(); }

	const vec3_t target = vec3_add(camera->position, camera->forward);

	return mat4_look_at(camera->position, target, camera->up);
}

mat4_t camera_get_projection(const camera_t *camera, const float aspect_ratio) {
	if (camera == NULL || aspect_ratio <= 0.0f) { return mat4_identity(); }

	return mat4_perspective(camera->field_of_view * PI / 180.0f,
				aspect_ratio, camera->near_plane,
				camera->far_plane);
}