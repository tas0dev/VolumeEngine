/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "entity/func_ladder.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct func_ladder {
	entity_t entity;
	vec3_t normal;
};

static entity_t *create_entity(entity_id_t id,
			       const entity_spawn_context_t *context);
static bool accept_input(entity_t *entity,
			 const char *input_name,
			 const entity_input_context_t *context);
static void destroy_entity(entity_t *entity);
static void
set_error(const entity_spawn_context_t *context, const char *format, ...);

static const entity_class_t func_ladder_class = {
	.classname = "func_ladder",
	.create = create_entity,
	.accept_input = accept_input,
	.destroy = destroy_entity,
};

bool func_ladder_register(void) {
	return entity_register_class(&func_ladder_class);
}

func_ladder_t *func_ladder_from_entity(entity_t *entity) {
	if (entity == NULL || entity->class != &func_ladder_class) {
		return NULL;
	}
	return (func_ladder_t *)entity;
}

const func_ladder_t *func_ladder_from_const_entity(const entity_t *entity) {
	if (entity == NULL || entity->class != &func_ladder_class) {
		return NULL;
	}
	return (const func_ladder_t *)entity;
}

entity_t *func_ladder_get_entity(func_ladder_t *ladder) {
	return ladder == NULL ? NULL : &ladder->entity;
}

vec3_t func_ladder_get_normal(const func_ladder_t *ladder) {
	if (ladder == NULL) { return vec3_create(0.0f, 0.0f, 1.0f); }
	return ladder->normal;
}

static entity_t *create_entity(const entity_id_t id,
			       const entity_spawn_context_t *context) {
	const char *text;
	func_ladder_t *ladder;
	vec3_t normal;
	vec3_t size;
	bool starts_disabled;

	if (context == NULL || context->properties == NULL) { return NULL; }

	size = vec3_create(1.0f, 2.0f, 0.25f);
	normal = vec3_create(0.0f, 0.0f, 1.0f);
	starts_disabled = false;

	text = entity_property_get(context->source, "size");
	if (text != NULL && !entity_property_parse_vec3(text, &size)) {
		set_error(context, "invalid func_ladder size: \"%s\"", text);
		return NULL;
	}
	if (size.x <= 0.0f || size.y <= 0.0f || size.z <= 0.0f) {
		set_error(context, "func_ladder size must be positive");
		return NULL;
	}

	text = entity_property_get(context->source, "normal");
	if (text != NULL && !entity_property_parse_vec3(text, &normal)) {
		set_error(context, "invalid func_ladder normal: \"%s\"", text);
		return NULL;
	}
	normal.y = 0.0f;
	if (vec3_length(normal) <= 0.000001f) {
		set_error(context, "func_ladder normal must be horizontal");
		return NULL;
	}
	normal = vec3_normalize(normal);

	text = entity_property_get(context->source, "starts_disabled");
	if (text != NULL &&
	    !entity_property_parse_bool(text, &starts_disabled)) {
		set_error(context,
			  "invalid func_ladder starts_disabled: \"%s\"", text);
		return NULL;
	}

	ladder = calloc(1, sizeof(*ladder));
	if (ladder == NULL) { return NULL; }
	entity_initialize(&ladder->entity, id, &func_ladder_class);
	ladder->entity.transform = context->properties->transform;
	ladder->normal = normal;
	entity_set_collider(&ladder->entity,
			    collider_create_box(vec3_create(0.0f, 0.0f, 0.0f),
						vec3_scale(size, 0.5f)));
	entity_set_collision_filter(&ladder->entity, COLLISION_LAYER_TRIGGER,
				    COLLISION_LAYER_PLAYER);
	ladder->entity.collider_follows_transform = true;
	ladder->entity.active = !starts_disabled;

	if (!entity_set_targetname(&ladder->entity,
				   context->properties->targetname)) {
		free(ladder);
		return NULL;
	}

	return &ladder->entity;
}

static bool accept_input(entity_t *entity,
			 const char *input_name,
			 const entity_input_context_t *context) {
	(void)context;
	if (func_ladder_from_entity(entity) == NULL || input_name == NULL) {
		return false;
	}
	if (strcmp(input_name, "Enable") == 0) {
		entity_set_active(entity, true);
		return true;
	}
	if (strcmp(input_name, "Disable") == 0) {
		entity_set_active(entity, false);
		return true;
	}
	return false;
}

static void destroy_entity(entity_t *entity) { free(entity); }

static void
set_error(const entity_spawn_context_t *context, const char *format, ...) {
	va_list arguments;

	if (context == NULL || context->error == NULL ||
	    context->error_size == 0) {
		return;
	}
	va_start(arguments, format);
	vsnprintf(context->error, context->error_size, format, arguments);
	va_end(arguments);
}
