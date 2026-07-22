/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "entity/prop_dynamic.h"
#include "entity/prop_internal.h"
#include "scene/transform.h"

static entity_t *create_entity(entity_id_t id,
			       const entity_spawn_context_t *context);
static void update_entity(entity_t *entity, float delta_time);

static const entity_class_t prop_dynamic_class = {
	.classname = "prop_dynamic",
	.create = create_entity,
	.update = update_entity,
	.draw_shadow = prop_internal_draw_shadow,
	.draw = prop_internal_draw,
	.destroy = prop_internal_destroy,
};

prop_dynamic_properties_t prop_dynamic_properties_create(void) {
	return prop_static_properties_create();
}

prop_dynamic_t *
prop_dynamic_create(const entity_id_t id,
		    const prop_dynamic_properties_t *properties) {
	prop_dynamic_t *prop;

	prop = (prop_dynamic_t *)prop_static_create(id, properties);
	if (prop == NULL) { return NULL; }

	prop->entity.class = &prop_dynamic_class;
	prop->entity.collider_follows_transform = true;

	return prop;
}

void prop_dynamic_destroy(prop_dynamic_t *prop) {
	if (prop == NULL) { return; }

	entity_destroy((entity_t *)prop);
}

entity_t *prop_dynamic_get_entity(prop_dynamic_t *prop) {
	return (entity_t *)prop;
}

const entity_t *prop_dynamic_get_const_entity(const prop_dynamic_t *prop) {
	return (const entity_t *)prop;
}

prop_dynamic_t *prop_dynamic_from_entity(entity_t *entity) {
	if (entity == NULL || entity->class != &prop_dynamic_class) {
		return NULL;
	}

	return (prop_dynamic_t *)entity;
}

const prop_dynamic_t *prop_dynamic_from_const_entity(const entity_t *entity) {
	if (entity == NULL || entity->class != &prop_dynamic_class) {
		return NULL;
	}

	return (const prop_dynamic_t *)entity;
}

bool prop_dynamic_register(void) {
	return entity_register_class(&prop_dynamic_class);
}

static entity_t *create_entity(const entity_id_t id,
			       const entity_spawn_context_t *context) {
	entity_t *entity;

	entity = prop_internal_create_static(id, context);
	if (entity == NULL) { return NULL; }

	entity->class = &prop_dynamic_class;
	entity->collider_follows_transform = true;

	return entity;
}

static void update_entity(entity_t *entity, const float delta_time) {
	const triangle_mesh_collider_t *mesh;
	transform_t local_transform;
	mat4_t transform;

	(void)delta_time;

	if (entity == NULL || !entity->has_collider ||
	    entity->collider.type != COLLIDER_TYPE_TRIANGLE_MESH) {
		return;
	}

	mesh = entity->collider.shape.triangle_mesh.mesh;
	local_transform = entity->transform;
	local_transform.position = vec3_create(0.0f, 0.0f, 0.0f);
	transform = transform_get_matrix(&local_transform);

	entity->collider = collider_create_triangle_mesh(mesh, transform);
}
