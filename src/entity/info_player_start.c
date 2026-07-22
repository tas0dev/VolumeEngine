/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "entity/info_player_start.h"
#include <stdlib.h>

static entity_t *create_entity(entity_id_t id,
			       const entity_spawn_context_t *context);
static void destroy_entity(entity_t *entity);

static const entity_class_t info_player_start_class = {
	.classname = "info_player_start",
	.create = create_entity,
	.destroy = destroy_entity,
};

bool info_player_start_register(void) {
	return entity_register_class(&info_player_start_class);
}

info_player_start_t *info_player_start_from_entity(entity_t *entity) {
	if (entity == NULL || entity->class != &info_player_start_class) {
		return NULL;
	}
	return (info_player_start_t *)entity;
}

const info_player_start_t *
info_player_start_from_const_entity(const entity_t *entity) {
	if (entity == NULL || entity->class != &info_player_start_class) {
		return NULL;
	}
	return (const info_player_start_t *)entity;
}

static entity_t *create_entity(const entity_id_t id,
			       const entity_spawn_context_t *context) {
	info_player_start_t *start;

	if (context == NULL || context->properties == NULL) { return NULL; }
	start = calloc(1, sizeof(*start));
	if (start == NULL) { return NULL; }

	entity_initialize(&start->entity, id, &info_player_start_class);
	start->entity.transform = context->properties->transform;
	if (!entity_set_targetname(&start->entity,
				   context->properties->targetname)) {
		free(start);
		return NULL;
	}
	return &start->entity;
}

static void destroy_entity(entity_t *entity) { free(entity); }
