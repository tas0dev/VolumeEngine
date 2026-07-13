/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#ifndef VOLUME_ENTITY_ENTITY_H
#define VOLUME_ENTITY_ENTITY_H

#include "scene/transform.h"
#include <stdbool.h>
#include <stdint.h>

typedef uint32_t entity_id_t;

typedef struct entity {
	entity_id_t id;
	const char *classname;
	transform_t transform;
	bool active;
} entity_t;

entity_t entity_create(entity_id_t id, const char *classname);
void entity_set_active(entity_t *entity, bool active);
bool entity_is_active(const entity_t *entity);

#endif