/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "entity/light_environment.h"
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

static void
set_error(const entity_spawn_context_t *context, const char *format, ...);
static entity_t *create_entity(entity_id_t id,
			       const entity_spawn_context_t *context);
static void destroy_entity(entity_t *entity);

static const entity_class_t light_environment_class = {
	.classname = "light_environment",
	.create = create_entity,
	.update = NULL,
	.draw_shadow = NULL,
	.draw = NULL,
	.destroy = destroy_entity,
};

static void
set_error(const entity_spawn_context_t *context, const char *format, ...) {
	va_list arguments;

	if (context->error == NULL || context->error_size == 0) { return; }

	va_start(arguments, format);
	vsnprintf(context->error, context->error_size, format, arguments);
	va_end(arguments);
}

light_environment_properties_t light_environment_properties_create(void) {
	light_environment_properties_t properties;

	properties.entity = entity_properties_create();
	properties.color = vec3_create(1.0f, 1.0f, 1.0f);
	properties.intensity = 1.0f;

	return properties;
}

light_environment_t *
light_environment_create(const entity_id_t id,
			 const light_environment_properties_t *properties) {
	light_environment_t *light;

	if (properties == NULL || properties->intensity < 0.0f ||
	    properties->color.x < 0.0f || properties->color.y < 0.0f ||
	    properties->color.z < 0.0f) {
		return NULL;
	}

	light = calloc(1, sizeof(*light));
	if (light == NULL) { return NULL; }

	entity_initialize((entity_t *)light, id, &light_environment_class);

	if (!entity_set_targetname((entity_t *)light,
				   properties->entity.targetname)) {
		free(light);
		return NULL;
	}

	light->entity.transform = properties->entity.transform;
	light->color = properties->color;
	light->intensity = properties->intensity;

	return light;
}

static entity_t *create_entity(const entity_id_t id,
			       const entity_spawn_context_t *context) {
	light_environment_properties_t properties;
	light_environment_t *light;
	const char *color;
	const char *intensity;

	if (context == NULL || context->properties == NULL ||
	    context->source == NULL) {
		return NULL;
	}

	properties = light_environment_properties_create();
	properties.entity = *context->properties;

	color = entity_property_get(context->source, "color");
	if (color != NULL &&
	    !entity_property_parse_vec3(color, &properties.color)) {
		set_error(context, "invalid vec3 property \"color\": \"%s\"",
			  color);
		return NULL;
	}

	if (properties.color.x < 0.0f || properties.color.y < 0.0f ||
	    properties.color.z < 0.0f) {
		set_error(context, "light color cannot be negative");
		return NULL;
	}

	intensity = entity_property_get(context->source, "intensity");
	if (intensity != NULL &&
	    !entity_property_parse_float(intensity, &properties.intensity)) {
		set_error(context,
			  "invalid float property \"intensity\": \"%s\"",
			  intensity);
		return NULL;
	}

	if (properties.intensity < 0.0f) {
		set_error(context, "light intensity cannot be negative");
		return NULL;
	}

	light = light_environment_create(id, &properties);
	if (light == NULL) {
		set_error(context, "failed to create light_environment");
		return NULL;
	}

	return (entity_t *)light;
}

static void destroy_entity(entity_t *entity) {
	free((light_environment_t *)entity);
}

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
	if (light == NULL) {
		return vec3_create(1.0f, 1.0f, 1.0f); }

	return light->color;
}

float light_environment_get_intensity(const light_environment_t *light) {
	if (light == NULL) { return 1.0f; }

	return light->intensity;
}

bool light_environment_register(void) {
	return entity_register_class(&light_environment_class);
}