/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#ifndef VOLUME_ENTITY_PROPERTIES_H
#define VOLUME_ENTITY_PROPERTIES_H

#include "math/vec3.h"
#include "scene/transform.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct entity_property_source {
	const void *context;
	const char *(*get)(const void *context, const char *key);
} entity_property_source_t;

typedef struct entity_properties {
	const char *targetname;
	transform_t transform;
} entity_properties_t;

entity_properties_t entity_properties_create(void);
const char *entity_property_get(const entity_property_source_t *source,
				const char *key);
bool entity_property_parse_float(const char *text, float *value);
bool entity_property_parse_vec3(const char *text, vec3_t *value);
bool entity_property_parse_bool(const char *text, bool *value);
bool entity_properties_load(entity_properties_t *properties,
			    const entity_property_source_t *source,
			    char *error,
			    size_t error_size);

#endif