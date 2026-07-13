/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#include "entity/entity.h"
#include <stddef.h>

entity_t entity_create(const entity_id_t id, const entity_class_t *class) {
	entity_t entity;

	entity.id = id;
	entity.class = class;
	entity.transform = transform_create();
	entity.active = true;

	return entity;
}

const char *entity_get_classname(const entity_t *entity) {
	if (entity == NULL || entity->class == NULL) { return NULL; }

	return entity->class->classname;
}

void entity_update(entity_t *entity, const float delta_time) {
	if (entity == NULL || !entity->active || entity->class == NULL ||
	    entity->class->update == NULL) {
		return;
	}

	entity->class->update(entity, delta_time);
}

void entity_destroy(entity_t *entity) {
	if (entity == NULL || entity->class == NULL ||
	    entity->class->destroy == NULL) {
		return;
	}

	entity->class->destroy(entity);
}

void entity_set_active(entity_t *entity, const bool active) {
	if (entity == NULL) { return; }

	entity->active = active;
}

bool entity_is_active(const entity_t *entity) {
	if (entity == NULL) { return false; }

	return entity->active;
}