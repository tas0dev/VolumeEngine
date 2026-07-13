/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#include "entity/world.h"
#include <stdlib.h>
#include <string.h>

struct world {
	entity_t **entities;
	size_t count;
	size_t capacity;
};

static bool world_reserve(world_t *world, size_t capacity) {
	entity_t **entities;

	if (capacity <= world->capacity) { return true; }

	entities =
		realloc(world->entities, capacity * sizeof(*world->entities));
	if (entities == NULL) { return false; }

	world->entities = entities;
	world->capacity = capacity;

	return true;
}

world_t *world_create(void) { return calloc(1, sizeof(world_t)); }

void world_destroy(world_t *world) {
	size_t index;

	if (world == NULL) { return; }

	for (index = 0; index < world->count; index++) {
		entity_destroy(world->entities[index]);
	}

	free(world->entities);
	free(world);
}

bool world_add_entity(world_t *world, entity_t *entity) {
	size_t capacity;

	if (world == NULL || entity == NULL) { return false; }

	if (world_find_entity(world, entity->id) != NULL) { return false; }

	if (world->count == world->capacity) {
		capacity = world->capacity == 0 ? 16 : world->capacity * 2;

		if (!world_reserve(world, capacity)) { return false; }
	}

	world->entities[world->count] = entity;
	world->count++;

	return true;
}

bool world_remove_entity(world_t *world, entity_id_t id) {
	size_t index;

	if (world == NULL) {
		return false; }

	for (index = 0; index < world->count; index++) {
		if (world->entities[index]->id != id) { continue; }

		entity_destroy(world->entities[index]);

		if (index + 1 < world->count) {
			memmove(
				&world->entities[index],
				&world->entities[index + 1],
				(world->count - index - 1) *
					sizeof(*world->entities)
			);
		}

		world->count--;
		world->entities[world->count] = NULL;

		return true;
	}

	return false;
}

entity_t *world_find_entity(world_t *world, entity_id_t id) {
	size_t index;

	if (world == NULL) { return NULL;
	}

	for (index = 0; index < world->count; index++) {
		if (world->entities[index]->id == id) {
			return world->entities[index];
		}
	}

	return NULL;
}

entity_t *world_find_by_classname(world_t *world, const char *classname) {
	const char *entity_classname;
	size_t index;

	if (world == NULL || classname == NULL) { return NULL; }

	for (index = 0; index < world->count; index++) {
		entity_classname =
			entity_get_classname(world->entities[index]);

		if (entity_classname == NULL) {
			continue; }

		if (strcmp(entity_classname, classname) == 0) {
			return world->entities[index];
		}
	}

	return NULL;
}

size_t world_get_entity_count(const world_t *world) {
	if (world == NULL) {
		return 0;
	}

	return world->count;
}

entity_t *world_get_entity(world_t *world, size_t index) {
	if (world == NULL || index >= world->count) {
		return NULL; }

	return world->entities[index];
}

void world_update(world_t *world, const float delta_time) {
	size_t index;

	if (world == NULL) {
		return;
	}

	for (index = 0; index < world->count; index++) {
		entity_update(world->entities[index], delta_time);
	}
}

void world_draw_shadows(world_t *world, renderer_t *renderer) {
	size_t index;

	if (world == NULL || renderer == NULL) {
		return; }

	for (index = 0; index < world->count; index++) {
		entity_draw_shadow(world->entities[index], renderer);
	}
}

void world_draw(world_t *world,
		renderer_t *renderer,
		const render_view_t *view) {
	size_t index;

	if (world == NULL || renderer == NULL || view == NULL) { return; }

	for (index = 0; index < world->count; index++) {
		entity_draw(world->entities[index], renderer, view);
	}
}