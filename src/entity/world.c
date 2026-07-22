/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "entity/world.h"
#include "entity/io.h"
#include <stdlib.h>
#include <string.h>

typedef struct entity_io_event {
	char *target_name;
	char *input_name;
	char *parameter;
	entity_id_t activator_id;
	entity_id_t caller_id;
	double fire_time;
} entity_io_event_t;

struct world {
	entity_t **entities;
	size_t count;
	size_t capacity;
	entity_id_t next_entity_id;
	collision_world_t *collision_world;
	entity_io_event_t *events;
	size_t event_count;
	size_t event_capacity;
	double time;
};

static bool world_reserve(world_t *world, size_t capacity);
static entity_id_t world_allocate_entity_id(world_t *world);
static char *duplicate_string(const char *string);
static bool world_reserve_events(world_t *world, size_t capacity);
static bool world_queue_input(world_t *world,
			      const entity_output_connection_t *connection,
			      entity_t *activator,
			      entity_t *caller);
static void world_process_events(world_t *world);
static void world_flush_destroyed_entities(world_t *world);

static char *duplicate_string(const char *string) {
	char *copy;
	size_t length;

	if (string == NULL) { string = ""; }
	length = strlen(string);
	copy = malloc(length + 1);
	if (copy == NULL) { return NULL; }
	memcpy(copy, string, length + 1);
	return copy;
}

static bool world_reserve(world_t *world, const size_t capacity) {
	entity_t **entities;

	if (capacity <= world->capacity) { return true; }

	entities =
		realloc(world->entities, capacity * sizeof(*world->entities));
	if (entities == NULL) { return false; }

	world->entities = entities;
	world->capacity = capacity;

	return true;
}

static bool world_reserve_events(world_t *world, const size_t capacity) {
	entity_io_event_t *events;

	if (capacity <= world->event_capacity) { return true; }
	if (capacity > SIZE_MAX / sizeof(*world->events)) { return false; }

	events = realloc(world->events, capacity * sizeof(*world->events));
	if (events == NULL) { return false; }

	world->events = events;
	world->event_capacity = capacity;
	return true;
}

static entity_id_t world_allocate_entity_id(world_t *world) {
	entity_id_t first_id;
	entity_id_t id;

	if (world->next_entity_id == 0) { world->next_entity_id = 1; }

	first_id = world->next_entity_id;

	do {
		id = world->next_entity_id;
		world->next_entity_id++;

		if (world->next_entity_id == 0) { world->next_entity_id = 1; }

		if (world_find_entity(world, id) == NULL) { return id; }
	} while (world->next_entity_id != first_id);

	return 0;
}

world_t *world_create(void) {
	world_t *world;

	world = calloc(1, sizeof(*world));
	if (world == NULL) { return NULL; }

	world->collision_world = collision_world_create();
	if (world->collision_world == NULL) {
		free(world);
		return NULL;
	}

	world->next_entity_id = 1;

	return world;
}

void world_destroy(world_t *world) {
	size_t index;

	if (world == NULL) { return; }

	for (index = 0; index < world->count; index++) {
		entity_destroy(world->entities[index]);
	}

	for (index = 0; index < world->event_count; index++) {
		free(world->events[index].parameter);
		free(world->events[index].input_name);
		free(world->events[index].target_name);
	}

	collision_world_destroy(world->collision_world);
	free(world->events);
	free(world->entities);
	free(world);
}
entity_t *world_spawn_entity(world_t *world,
			     const char *classname,
			     const entity_spawn_context_t *context) {
	entity_t *entity;
	entity_id_t id;

	if (world == NULL || classname == NULL || context == NULL ||
	    context->properties == NULL) {
		return NULL;
	}

	id = world_allocate_entity_id(world);
	if (id == 0) { return NULL; }

	entity = entity_create(classname, id, context);
	if (entity == NULL) { return NULL; }

	if (!world_add_entity(world, entity)) {
		entity_destroy(entity);
		return NULL;
	}

	return entity;
}

bool world_add_entity(world_t *world, entity_t *entity) {
	size_t capacity;

	if (world == NULL || entity == NULL || entity->id == 0) {
		return false;
	}

	if (world_find_entity(world, entity->id) != NULL) { return false; }

	if (world->count == world->capacity) {
		capacity = world->capacity == 0 ? 16 : world->capacity * 2;

		if (!world_reserve(world, capacity)) { return false; }
	}

	if (entity->has_collider &&
	    !collision_world_add_collider(world->collision_world, entity->id,
					  entity->collider,
					  entity->transform.position)) {
		return false;
	}

	world->entities[world->count] = entity;
	entity->world = world;
	world->count++;

	return true;
}

bool world_remove_entity(world_t *world, const entity_id_t id) {
	size_t index;

	if (world == NULL || id == 0) { return false; }

	for (index = 0; index < world->count; index++) {
		if (world->entities[index]->id != id) { continue; }

		collision_world_remove(world->collision_world, id);
		world->entities[index]->world = NULL;
		entity_destroy(world->entities[index]);

		if (index + 1 < world->count) {
			memmove(&world->entities[index],
				&world->entities[index + 1],
				(world->count - index - 1) *
					sizeof(*world->entities));
		}

		world->count--;
		world->entities[world->count] = NULL;

		return true;
	}

	return false;
}

entity_t *world_find_entity(world_t *world, const entity_id_t id) {
	size_t index;

	if (world == NULL || id == 0) { return NULL; }

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
		entity_classname = entity_get_classname(world->entities[index]);

		if (entity_classname == NULL) { continue; }

		if (strcmp(entity_classname, classname) == 0) {
			return world->entities[index];
		}
	}

	return NULL;
}

size_t world_get_entity_count(const world_t *world) {
	if (world == NULL) { return 0; }

	return world->count;
}

entity_t *world_get_entity(world_t *world, const size_t index) {
	if (world == NULL || index >= world->count) { return NULL; }

	return world->entities[index];
}

void world_update(world_t *world, const float delta_time) {
	size_t index;

	if (world == NULL) { return; }
	if (delta_time > 0.0f) { world->time += delta_time; }

	world_process_events(world);

	for (index = 0; index < world->count; index++) {
		entity_t *entity = world->entities[index];
		if (entity->pending_destroy) { continue; }

		entity_update(entity, delta_time);

		if (entity->collider_follows_transform) {
			collision_world_update_collider(
				world->collision_world, entity->id,
				entity->collider, entity->transform.position);
		}
	}

	world_flush_destroyed_entities(world);
}

void world_draw_shadows(world_t *world, renderer_t *renderer) {
	size_t index;

	if (world == NULL || renderer == NULL) { return; }

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

entity_t *world_find_by_targetname(world_t *world, const char *targetname) {
	const char *entity_targetname;
	size_t index;

	if (world == NULL || targetname == NULL) { return NULL; }

	for (index = 0; index < world->count; index++) {
		entity_targetname =
			entity_get_targetname(world->entities[index]);

		if (entity_targetname == NULL) { continue; }

		if (strcmp(entity_targetname, targetname) == 0) {
			return world->entities[index];
		}
	}

	return NULL;
}

collision_world_t *world_get_collision_world(world_t *world) {
	if (world == NULL) { return NULL; }

	return world->collision_world;
}

const collision_world_t *world_get_const_collision_world(const world_t *world) {
	if (world == NULL) { return NULL; }

	return world->collision_world;
}

size_t world_send_input(world_t *world,
			const char *target_name,
			const char *input_name,
			const char *parameter,
			entity_t *activator,
			entity_t *caller) {
	entity_input_context_t context;
	entity_t *target;
	const char *name;
	size_t accepted;
	size_t index;

	if (world == NULL || target_name == NULL || target_name[0] == '\0' ||
	    input_name == NULL || input_name[0] == '\0') {
		return 0;
	}

	context.world = world;
	context.activator = activator;
	context.caller = caller;
	context.parameter = parameter == NULL ? "" : parameter;
	accepted = 0;
	target = NULL;

	if (strcmp(target_name, "!self") == 0 ||
	    strcmp(target_name, "!caller") == 0) {
		target = caller;
	} else if (strcmp(target_name, "!activator") == 0) {
		target = activator;
	}

	if (target != NULL) {
		if (target->pending_destroy) { return 0; }
		return entity_accept_input(target, input_name, &context) ? 1
									 : 0;
	}

	if (target_name[0] == '!') { return 0; }

	for (index = 0; index < world->count; index++) {
		target = world->entities[index];
		if (target->pending_destroy) { continue; }

		name = entity_get_targetname(target);
		if (name == NULL || strcmp(name, target_name) != 0) {
			continue;
		}

		if (entity_accept_input(target, input_name, &context)) {
			accepted++;
		}
	}

	return accepted;
}

bool world_fire_output(world_t *world,
		       entity_t *caller,
		       const char *output_name,
		       entity_t *activator) {
	entity_output_connection_t *connection;
	bool queued;
	size_t index;

	if (world == NULL || caller == NULL || output_name == NULL ||
	    output_name[0] == '\0' || caller->pending_destroy) {
		return false;
	}

	queued = false;

	for (index = 0; index < caller->output_count; index++) {
		connection = &caller->outputs[index];

		if (strcmp(connection->output_name, output_name) != 0 ||
		    connection->remaining_fires == 0) {
			continue;
		}

		if (!world_queue_input(world, connection, activator, caller)) {
			continue;
		}

		if (connection->remaining_fires > 0) {
			connection->remaining_fires--;
		}

		queued = true;
	}

	return queued;
}

size_t world_get_pending_event_count(const world_t *world) {
	return world == NULL ? 0 : world->event_count;
}

static bool world_queue_input(world_t *world,
			      const entity_output_connection_t *connection,
			      entity_t *activator,
			      entity_t *caller) {
	entity_io_event_t event = {0};
	size_t capacity;

	event.target_name = duplicate_string(connection->target_name);
	event.input_name = duplicate_string(connection->input_name);
	event.parameter = duplicate_string(connection->parameter);
	event.activator_id = activator == NULL ? 0 : activator->id;
	event.caller_id = caller == NULL ? 0 : caller->id;
	event.fire_time = world->time + connection->delay;

	if (event.target_name == NULL || event.input_name == NULL ||
	    event.parameter == NULL) {
		free(event.parameter);
		free(event.input_name);
		free(event.target_name);
		return false;
	}

	if (world->event_count == world->event_capacity) {
		capacity = world->event_capacity == 0
				   ? 16
				   : world->event_capacity * 2;

		if (capacity < world->event_capacity ||
		    !world_reserve_events(world, capacity)) {
			free(event.parameter);
			free(event.input_name);
			free(event.target_name);
			return false;
		}
	}

	world->events[world->event_count++] = event;
	return true;
}

static void world_process_events(world_t *world) {
	const size_t maximum_events = 1024;
	entity_io_event_t event;
	entity_t *activator;
	entity_t *caller;
	size_t processed;
	size_t index;

	processed = 0;

	while (processed < maximum_events) {
		for (index = 0; index < world->event_count; index++) {
			if (world->events[index].fire_time <= world->time) {
				break;
			}
		}

		if (index == world->event_count) { break; }

		event = world->events[index];
		if (index + 1 < world->event_count) {
			memmove(&world->events[index],
				&world->events[index + 1],
				(world->event_count - index - 1) *
					sizeof(*world->events));
		}
		world->event_count--;

		activator = world_find_entity(world, event.activator_id);
		caller = world_find_entity(world, event.caller_id);
		world_send_input(world, event.target_name, event.input_name,
				 event.parameter, activator, caller);

		free(event.parameter);
		free(event.input_name);
		free(event.target_name);
		processed++;
	}
}

static void world_flush_destroyed_entities(world_t *world) {
	size_t index;

	index = 0;
	while (index < world->count) {
		if (!world->entities[index]->pending_destroy) {
			index++;
			continue;
		}

		if (!world_remove_entity(world, world->entities[index]->id)) {
			index++;
		}
	}
}
