/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_ENTITY_PROP_H
#define VOLUME_ENTITY_PROP_H

#include "entity/entity.h"
#include "renderer/material.h"
#include "renderer/mesh.h"

typedef struct prop_properties {
	entity_properties_t entity;
	const mesh_t *mesh;
	const material_t *material;
	bool casts_shadow;
	bool has_collider;
	collider_t collider;
} prop_properties_t;

typedef struct prop {
	entity_t entity;
	const mesh_t *mesh;
	const material_t *material;
	bool casts_shadow;
} prop_t;

#endif
