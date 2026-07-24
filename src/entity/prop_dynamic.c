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

static const entity_class_t prop_dynamic_class = {
	.classname = "prop_dynamic",
	.create = create_entity,
	.update = NULL,
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
	entity_set_collision_filter(&prop->entity, COLLISION_LAYER_DYNAMIC,
				    COLLISION_LAYER_ALL);
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
	entity_set_collision_filter(entity, COLLISION_LAYER_DYNAMIC,
				    COLLISION_LAYER_ALL);
	entity->collider_follows_transform = true;

	return entity;
}
