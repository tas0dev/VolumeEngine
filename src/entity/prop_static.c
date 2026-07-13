/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#include "entity/prop_static.h"
#include <stdlib.h>

static void destroy_entity(entity_t *entity);

static const entity_class_t prop_static_class = {
	.classname = "prop_static",
	.update = NULL,
	.destroy = destroy_entity,
};

prop_static_t *prop_static_create(const entity_id_t id,
				  const mesh_t *mesh,
				  const material_t *material) {
	prop_static_t *prop;

	if (mesh == NULL || material == NULL) { return NULL; }

	prop = calloc(1, sizeof(*prop));
	if (prop == NULL) { return NULL; }

	prop->entity = entity_create(id, &prop_static_class);
	prop->mesh = mesh;
	prop->material = material;
	prop->casts_shadow = true;

	return prop;
}

void prop_static_destroy(prop_static_t *prop) {
	if (prop == NULL) { return; }

	entity_destroy(&prop->entity);
}

entity_t *prop_static_get_entity(prop_static_t *prop) {
	if (prop == NULL) { return NULL; }

	return &prop->entity;
}

const entity_t *prop_static_get_const_entity(const prop_static_t *prop) {
	if (prop == NULL) { return NULL; }

	return &prop->entity;
}

prop_static_t *prop_static_from_entity(entity_t *entity) {
	if (entity == NULL || entity->class != &prop_static_class) {
		return NULL;
	}

	return (prop_static_t *)entity;
}

const prop_static_t *prop_static_from_const_entity(const entity_t *entity) {
	if (entity == NULL || entity->class != &prop_static_class) {
		return NULL;
	}

	return (const prop_static_t *)entity;
}

static void destroy_entity(entity_t *entity) { free(entity); }