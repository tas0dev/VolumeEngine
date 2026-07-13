/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#include "entity/prop_static.h"
#include "renderer/renderer.h"
#include "scene/transform.h"
#include <stdlib.h>

static void draw_shadow_entity(entity_t *entity, renderer_t *renderer);
static void
draw_entity(entity_t *entity, renderer_t *renderer, const render_view_t *view);
static void destroy_entity(entity_t *entity);

static const entity_class_t prop_static_class = {
	.classname = "prop_static",
	.update = NULL,
	.draw_shadow = draw_shadow_entity,
	.draw = draw_entity,
	.destroy = destroy_entity,
};

prop_static_t *prop_static_create(const entity_id_t id,
				  const mesh_t *mesh,
				  const material_t *material) {
	prop_static_t *prop;

	if (mesh == NULL || material == NULL) { return NULL; }

	prop = calloc(1, sizeof(*prop));
	if (prop == NULL) { return NULL; }

	entity_initialize(&prop->entity, id, &prop_static_class);

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

static void draw_shadow_entity(entity_t *entity, renderer_t *renderer) {
	prop_static_t *prop;
	mat4_t model;

	prop = prop_static_from_entity(entity);
	if (prop == NULL || !prop->casts_shadow) { return; }

	model = transform_get_matrix(&entity->transform);
	renderer_draw_shadow_mesh(renderer, prop->mesh, &model);
}

static void
draw_entity(entity_t *entity, renderer_t *renderer, const render_view_t *view) {
	prop_static_t *prop;
	mat4_t model;

	prop = prop_static_from_entity(entity);
	if (prop == NULL) { return; }

	model = transform_get_matrix(&entity->transform);

	renderer_draw_mesh(renderer, prop->mesh, prop->material, &model,
			   &view->view, &view->projection,
			   &view->light_view_projection);
}

static void destroy_entity(entity_t *entity) { free(entity); }