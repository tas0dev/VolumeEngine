/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "entity/logic_auto.h"
#include "entity/world.h"
#include <stdlib.h>

static entity_t *create_entity(entity_id_t id,
			       const entity_spawn_context_t *context);
static void activate_entity(entity_t *entity);
static void destroy_entity(entity_t *entity);

static const entity_class_t logic_auto_class = {
	.classname = "logic_auto",
	.create = create_entity,
	.activate = activate_entity,
	.destroy = destroy_entity,
};

static entity_t *create_entity(const entity_id_t id,
			       const entity_spawn_context_t *context) {
	entity_t *entity;

	if (context == NULL || context->properties == NULL) { return NULL; }
	entity = calloc(1, sizeof(*entity));
	if (entity == NULL) { return NULL; }

	entity_initialize(entity, id, &logic_auto_class);
	entity->transform = context->properties->transform;
	if (!entity_set_targetname(entity, context->properties->targetname)) {
		entity_destroy(entity);
		return NULL;
	}
	return entity;
}

static void activate_entity(entity_t *entity) {
	if (entity == NULL || entity->world == NULL) { return; }
	(void)world_fire_output(entity->world, entity, "OnMapSpawn", entity);
}

static void destroy_entity(entity_t *entity) { free(entity); }

bool logic_auto_register(void) {
	return entity_register_class(&logic_auto_class);
}
