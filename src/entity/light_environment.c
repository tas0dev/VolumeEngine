/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#include "entity/light_environment.h"
#include <math.h>
#include <stdlib.h>

static entity_t *create_entity(entity_id_t id,
			       const entity_properties_t *properties);
static void destroy_entity(entity_t *entity);

static const entity_class_t light_environment_class = {
	.classname = "light_environment",
	.create = create_entity,
	.update = NULL,
	.draw_shadow = NULL,
	.draw = NULL,
	.destroy = destroy_entity,
};

static entity_t *create_entity(const entity_id_t id,
			       const entity_properties_t *properties) {
	light_environment_t *light;

	if (properties == NULL || properties->light_intensity < 0.0f ||
	    properties->light_color.x < 0.0f ||
	    properties->light_color.y < 0.0f ||
	    properties->light_color.z < 0.0f) {
		return NULL;
	}

	light = calloc(1, sizeof(*light));
	if (light == NULL) { return NULL; }

	entity_initialize(&light->entity, id, &light_environment_class);

	if (!entity_set_targetname(&light->entity, properties->targetname)) {
		entity_destroy(&light->entity);
		return NULL;
	}

	light->entity.transform = properties->transform;
	light->color = properties->light_color;
	light->intensity = properties->light_intensity;

	return &light->entity;
}

static void destroy_entity(entity_t *entity) { free(entity); }

light_environment_t *light_environment_from_entity(entity_t *entity) {
	if (entity == NULL || entity->class != &light_environment_class) {
		return NULL;
	}

	return (light_environment_t *)entity;
}

const light_environment_t *
light_environment_from_const_entity(const entity_t *entity) {
	if (entity == NULL || entity->class != &light_environment_class) {
		return NULL;
	}

	return (const light_environment_t *)entity;
}

vec3_t light_environment_get_direction(const light_environment_t *light) {
	float pitch;
	float yaw;

	if (light == NULL) { return vec3_create(0.0f, -1.0f, 0.0f); }

	pitch = light->entity.transform.rotation.x;
	yaw = light->entity.transform.rotation.y;

	return vec3_normalize(vec3_create(cosf(pitch) * cosf(yaw), sinf(pitch),
					  cosf(pitch) * sinf(yaw)));
}

vec3_t light_environment_get_color(const light_environment_t *light) {
	if (light == NULL) { return vec3_create(1.0f, 1.0f, 1.0f); }

	return light->color;
}

float light_environment_get_intensity(const light_environment_t *light) {
	if (light == NULL) { return 1.0f; }

	return light->intensity;
}

bool light_environment_register(void) {
	return entity_register_class(&light_environment_class);
}