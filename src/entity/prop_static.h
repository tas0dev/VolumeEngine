/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#ifndef VOLUME_ENTITY_PROP_STATIC_H
#define VOLUME_ENTITY_PROP_STATIC_H

#include "entity/entity.h"
#include "renderer/material.h"
#include "renderer/mesh.h"

typedef struct prop_static_properties {
	entity_properties_t entity;
	const mesh_t *mesh;
	const material_t *material;
	bool casts_shadow;
} prop_static_properties_t;

typedef struct prop_static {
	entity_t entity;
	const mesh_t *mesh;
	const material_t *material;
	bool casts_shadow;
} prop_static_t;

prop_static_properties_t prop_static_properties_create(void);
prop_static_t *prop_static_create(entity_id_t id,
				  const prop_static_properties_t *properties);
void prop_static_destroy(prop_static_t *prop);
entity_t *prop_static_get_entity(prop_static_t *prop);
const entity_t *prop_static_get_const_entity(const prop_static_t *prop);
prop_static_t *prop_static_from_entity(entity_t *entity);
const prop_static_t *prop_static_from_const_entity(const entity_t *entity);
bool prop_static_register(void);

#endif