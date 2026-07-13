/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#include "entity/properties.h"
#include <stddef.h>

entity_properties_t entity_properties_create(void) {
	entity_properties_t properties;

	properties.mesh = NULL;
	properties.material = NULL;
	properties.transform = transform_create();
	properties.casts_shadow = true;

	return properties;
}
