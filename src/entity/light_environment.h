/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#ifndef VOLUME_ENTITY_LIGHT_ENVIRONMENT_H
#define VOLUME_ENTITY_LIGHT_ENVIRONMENT_H

#include "entity/entity.h"
#include "math/vec3.h"

typedef struct light_environment_properties {
	entity_properties_t entity;
	vec3_t color;
	float intensity;
} light_environment_properties_t;

typedef struct light_environment {
	entity_t entity;
	vec3_t color;
	float intensity;
} light_environment_t;

light_environment_properties_t light_environment_properties_create(void);
light_environment_t *
light_environment_create(entity_id_t id,
			 const light_environment_properties_t *properties);
light_environment_t *light_environment_from_entity(entity_t *entity);
const light_environment_t *
light_environment_from_const_entity(const entity_t *entity);
vec3_t light_environment_get_direction(const light_environment_t *light);
vec3_t light_environment_get_color(const light_environment_t *light);
float light_environment_get_intensity(const light_environment_t *light);
bool light_environment_register(void);

#endif