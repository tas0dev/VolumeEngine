/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#ifndef VOLUME_RENDERER_VIEW_H
#define VOLUME_RENDERER_VIEW_H

#include "math/mat4.h"
#include "math/vec3.h"

typedef struct render_view {
	mat4_t view;
	mat4_t projection;
	mat4_t light_view_projection;
	vec3_t light_direction;
	vec3_t light_color;
	float light_intensity;
} render_view_t;

#endif
