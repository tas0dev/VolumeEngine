/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#include "entity/entity.h"
#include <stddef.h>

entity_t entity_create(const entity_id_t id, const char *classname) {
	entity_t entity;

	entity.id = id;
	entity.classname = classname;
	entity.transform = transform_create();
	entity.active = true;

	return entity;
}

void entity_set_active(entity_t *entity, const bool active) {
	if (entity == NULL) { return; }

	entity->active = active;
}

bool entity_is_active(const entity_t *entity) {
	if (entity == NULL) { return false; }

	return entity->active;
}