/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#include "renderer/material.h"

material_t material_create(const vec3_t color) {
	material_t material;

	material.color = color;
	material.albedo_texture = NULL;
	material.normal_texture = NULL;
	material.ambient_strength = 0.2f;
	material.specular_strength = 0.5f;
	material.shininess = 32.0f;

	return material;
}