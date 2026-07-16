/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_RENDERER_MATERIAL_H
#define VOLUME_RENDERER_MATERIAL_H

#include "math/vec3.h"
#include "renderer/texture.h"

typedef struct material {
	vec3_t color;
	const texture_t *albedo_texture;
	const texture_t *normal_texture;
	float ambient_strength;
	float specular_strength;
	float shininess;
} material_t;

material_t material_create(vec3_t color);

#endif