/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "entity/logic_relay.h"
#include "entity/world.h"
#include <stdlib.h>
#include <string.h>

static entity_t *create_entity(entity_id_t id,
			       const entity_spawn_context_t *context);
static bool accept_input(entity_t *entity,
			 const char *input_name,
			 const entity_input_context_t *context);
static void destroy_entity(entity_t *entity);

static const entity_class_t logic_relay_class = {
	.classname = "logic_relay",
	.create = create_entity,
	.accept_input = accept_input,
	.destroy = destroy_entity,
};

static entity_t *create_entity(const entity_id_t id,
			       const entity_spawn_context_t *context) {
	entity_t *entity;

	if (context == NULL || context->properties == NULL) { return NULL; }

	entity = calloc(1, sizeof(*entity));
	if (entity == NULL) { return NULL; }

	entity_initialize(entity, id, &logic_relay_class);
	entity->transform = context->properties->transform;

	if (!entity_set_targetname(entity, context->properties->targetname)) {
		free(entity);
		return NULL;
	}

	return entity;
}

static bool accept_input(entity_t *entity,
			 const char *input_name,
			 const entity_input_context_t *context) {
	if (strcmp(input_name, "Trigger") != 0 || !entity_is_active(entity)) {
		return false;
	}

	if (context == NULL || context->world == NULL) { return false; }

	world_fire_output(context->world, entity, "OnTrigger",
			  context->activator);
	return true;
}

static void destroy_entity(entity_t *entity) { free(entity); }

bool logic_relay_register(void) {
	return entity_register_class(&logic_relay_class);
}
