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

	properties.targetname = NULL;
	properties.mesh = NULL;
	properties.material = NULL;
	properties.transform = transform_create();
	properties.light_color = vec3_create(1.0f, 1.0f, 1.0f);
	properties.light_intensity = 1.0f;
	properties.casts_shadow = true;

	return properties;
}
