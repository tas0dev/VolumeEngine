/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "entity/func_door.h"
#include "entity/prop_internal.h"
#include "entity/world.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static entity_t *create_entity(entity_id_t id,
			       const entity_spawn_context_t *context);
static void update_entity(entity_t *entity, float delta_time);
static bool accept_input(entity_t *entity,
			 const char *input_name,
			 const entity_input_context_t *context);
static void
set_error(const entity_spawn_context_t *context, const char *format, ...);
static void begin_open(func_door_t *door, entity_t *activator);
static void begin_close(func_door_t *door, entity_t *activator);
static bool move_towards(vec3_t *position, vec3_t target, float distance);
static entity_t *get_activator(func_door_t *door);

static const entity_class_t func_door_class = {
	.classname = "func_door",
	.create = create_entity,
	.update = update_entity,
	.draw_shadow = prop_internal_draw_shadow,
	.draw = prop_internal_draw,
	.accept_input = accept_input,
	.destroy = prop_internal_destroy,
};

static entity_t *create_entity(const entity_id_t id,
			       const entity_spawn_context_t *context) {
	func_door_t *door;
	const char *text;
	vec3_t move_offset;
	bool starts_open;

	if (context == NULL || context->properties == NULL ||
	    context->source == NULL) {
		return NULL;
	}

	door = calloc(1, sizeof(*door));
	if (door == NULL) { return NULL; }

	if (!prop_internal_initialize(&door->prop, id, &func_door_class,
				      context)) {
		free(door);
		return NULL;
	}

	move_offset = vec3_create(0.0f, 2.0f, 0.0f);
	door->speed = 1.0f;
	door->wait = -1.0f;
	door->locked = false;
	starts_open = false;

	text = entity_property_get(context->source, "move_offset");
	if (text != NULL && !entity_property_parse_vec3(text, &move_offset)) {
		set_error(context,
			  "invalid vec3 property \"move_offset\": \"%s\"",
			  text);
		entity_destroy(&door->prop.entity);
		return NULL;
	}

	if (vec3_length(move_offset) <= 0.000001f) {
		set_error(context, "func_door move_offset cannot be zero");
		entity_destroy(&door->prop.entity);
		return NULL;
	}

	text = entity_property_get(context->source, "speed");
	if (text != NULL && !entity_property_parse_float(text, &door->speed)) {
		set_error(context, "invalid float property \"speed\": \"%s\"",
			  text);
		entity_destroy(&door->prop.entity);
		return NULL;
	}

	if (door->speed <= 0.0f) {
		set_error(context, "func_door speed must be positive");
		entity_destroy(&door->prop.entity);
		return NULL;
	}

	text = entity_property_get(context->source, "wait");
	if (text != NULL && !entity_property_parse_float(text, &door->wait)) {
		set_error(context, "invalid float property \"wait\": \"%s\"",
			  text);
		entity_destroy(&door->prop.entity);
		return NULL;
	}

	text = entity_property_get(context->source, "locked");
	if (text != NULL && !entity_property_parse_bool(text, &door->locked)) {
		set_error(context,
			  "invalid boolean property \"locked\": \"%s\"", text);
		entity_destroy(&door->prop.entity);
		return NULL;
	}

	text = entity_property_get(context->source, "starts_open");
	if (text != NULL && !entity_property_parse_bool(text, &starts_open)) {
		set_error(context,
			  "invalid boolean property \"starts_open\": \"%s\"",
			  text);
		entity_destroy(&door->prop.entity);
		return NULL;
	}

	door->closed_position = door->prop.entity.transform.position;
	door->open_position = vec3_add(door->closed_position, move_offset);
	door->state = starts_open ? FUNC_DOOR_OPEN : FUNC_DOOR_CLOSED;
	door->wait_remaining = door->wait;
	door->prop.entity.collider_follows_transform = true;

	if (starts_open) {
		door->prop.entity.transform.position = door->open_position;
	}

	return &door->prop.entity;
}

static void update_entity(entity_t *entity, const float delta_time) {
	func_door_t *door;
	bool reached;

	if (entity == NULL || delta_time < 0.0f) { return; }

	door = (func_door_t *)entity;

	if (door->state == FUNC_DOOR_OPEN) {
		if (door->wait < 0.0f) { return; }

		door->wait_remaining -= delta_time;
		if (door->wait_remaining <= 0.0f) {
			begin_close(door, get_activator(door));
		}
		return;
	}

	if (door->state != FUNC_DOOR_OPENING &&
	    door->state != FUNC_DOOR_CLOSING) {
		return;
	}

	reached = move_towards(&entity->transform.position,
			       door->state == FUNC_DOOR_OPENING
				       ? door->open_position
				       : door->closed_position,
			       door->speed * delta_time);

	if (!reached) { return; }

	if (door->state == FUNC_DOOR_OPENING) {
		door->state = FUNC_DOOR_OPEN;
		door->wait_remaining = door->wait;
		world_fire_output(entity->world, entity, "OnFullyOpen",
				  get_activator(door));
	} else {
		door->state = FUNC_DOOR_CLOSED;
		world_fire_output(entity->world, entity, "OnFullyClosed",
				  get_activator(door));
	}
}

static bool accept_input(entity_t *entity,
			 const char *input_name,
			 const entity_input_context_t *context) {
	func_door_t *door;
	entity_t *activator;

	door = func_door_from_entity(entity);
	if (door == NULL) { return false; }

	activator = context == NULL ? NULL : context->activator;

	if (strcmp(input_name, "Lock") == 0) {
		door->locked = true;
		return true;
	}

	if (strcmp(input_name, "Unlock") == 0) {
		door->locked = false;
		return true;
	}

	if (strcmp(input_name, "Close") == 0) {
		begin_close(door, activator);
		return true;
	}

	if (strcmp(input_name, "Open") != 0 &&
	    strcmp(input_name, "Toggle") != 0) {
		return false;
	}

	if (door->locked) {
		world_fire_output(entity->world, entity, "OnLockedUse",
				  activator);
		return true;
	}

	if (strcmp(input_name, "Open") == 0 ||
	    door->state == FUNC_DOOR_CLOSED ||
	    door->state == FUNC_DOOR_CLOSING) {
		begin_open(door, activator);
	} else {
		begin_close(door, activator);
	}

	return true;
}

static void begin_open(func_door_t *door, entity_t *activator) {
	if (door == NULL || door->state == FUNC_DOOR_OPEN ||
	    door->state == FUNC_DOOR_OPENING) {
		return;
	}

	door->state = FUNC_DOOR_OPENING;
	door->activator_id = activator == NULL ? 0 : activator->id;
	world_fire_output(door->prop.entity.world, &door->prop.entity, "OnOpen",
			  activator);
}

static void begin_close(func_door_t *door, entity_t *activator) {
	if (door == NULL || door->state == FUNC_DOOR_CLOSED ||
	    door->state == FUNC_DOOR_CLOSING) {
		return;
	}

	door->state = FUNC_DOOR_CLOSING;
	door->activator_id = activator == NULL ? 0 : activator->id;
	world_fire_output(door->prop.entity.world, &door->prop.entity,
			  "OnClose", activator);
}

static bool
move_towards(vec3_t *position, const vec3_t target, const float distance) {
	vec3_t delta;
	float length;

	delta = vec3_subtract(target, *position);
	length = vec3_length(delta);

	if (length <= distance || length <= 0.000001f) {
		*position = target;
		return true;
	}

	*position = vec3_add(*position, vec3_scale(delta, distance / length));
	return false;
}

static entity_t *get_activator(func_door_t *door) {
	if (door == NULL || door->prop.entity.world == NULL ||
	    door->activator_id == 0) {
		return NULL;
	}

	return world_find_entity(door->prop.entity.world, door->activator_id);
}

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

func_door_t *func_door_from_entity(entity_t *entity) {
	if (entity == NULL || entity->class != &func_door_class) {
		return NULL;
	}
	return (func_door_t *)entity;
}

const func_door_t *func_door_from_const_entity(const entity_t *entity) {
	if (entity == NULL || entity->class != &func_door_class) {
		return NULL;
	}
	return (const func_door_t *)entity;
}

func_door_state_t func_door_get_state(const func_door_t *door) {
	return door == NULL ? FUNC_DOOR_CLOSED : door->state;
}

bool func_door_register(void) {
	return entity_register_class(&func_door_class);
}
