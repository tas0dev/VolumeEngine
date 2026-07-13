/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#ifndef VOLUME_ENTITY_PROPERTIES_H
#define VOLUME_ENTITY_PROPERTIES_H

#include "renderer/material.h"
#include "renderer/mesh.h"
#include "scene/transform.h"
#include <stdbool.h>

typedef struct entity_properties {
	const mesh_t *mesh;
	const material_t *material;
	transform_t transform;
	bool casts_shadow;
} entity_properties_t;

entity_properties_t entity_properties_create(void);

#endif
