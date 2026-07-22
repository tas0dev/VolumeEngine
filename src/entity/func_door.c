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
static void begin_open(func_door_t *door, entity_t *activator);
static void begin_close(func_door_t *door, entity_t *activator);
static void
set_error(const entity_spawn_context_t *context, const char *format, ...);

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
	mover_config_t mover_config = {0};
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
	mover_config.speed = 1.0f;
	mover_config.wait = -1.0f;
	mover_config.block_policy = MOVER_BLOCK_STOP;
	mover_config.sweep_collider = true;
	mover_config.move_riders = true;
	mover_config.outputs.on_move_to_end = "OnOpen";
	mover_config.outputs.on_reached_end = "OnFullyOpen";
	mover_config.outputs.on_move_to_start = "OnClose";
	mover_config.outputs.on_reached_start = "OnFullyClosed";
	mover_config.outputs.on_blocked = "OnBlocked";
	mover_config.outputs.on_unblocked = "OnUnblocked";
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
	if (text != NULL &&
	    (!entity_property_parse_float(text, &mover_config.speed) ||
	     mover_config.speed <= 0.0f)) {
		set_error(context, "invalid positive func_door speed: \"%s\"",
			  text);
		entity_destroy(&door->prop.entity);
		return NULL;
	}

	text = entity_property_get(context->source, "acceleration");
	if (text != NULL &&
	    (!entity_property_parse_float(text, &mover_config.acceleration) ||
	     mover_config.acceleration < 0.0f)) {
		set_error(context,
			  "invalid non-negative func_door acceleration: \"%s\"",
			  text);
		entity_destroy(&door->prop.entity);
		return NULL;
	}

	text = entity_property_get(context->source, "deceleration");
	if (text != NULL &&
	    (!entity_property_parse_float(text, &mover_config.deceleration) ||
	     mover_config.deceleration < 0.0f)) {
		set_error(context,
			  "invalid non-negative func_door deceleration: \"%s\"",
			  text);
		entity_destroy(&door->prop.entity);
		return NULL;
	}

	text = entity_property_get(context->source, "wait");
	if (text != NULL &&
	    (!entity_property_parse_float(text, &mover_config.wait) ||
	     (mover_config.wait < 0.0f && mover_config.wait != -1.0f))) {
		set_error(context, "invalid func_door wait: \"%s\"", text);
		entity_destroy(&door->prop.entity);
		return NULL;
	}

	text = entity_property_get(context->source, "block_policy");
	if (text != NULL &&
	    !mover_parse_block_policy(text, &mover_config.block_policy)) {
		set_error(context, "invalid func_door block_policy: \"%s\"",
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

	mover_config.start_transform = door->prop.entity.transform;
	mover_config.end_transform = mover_config.start_transform;
	mover_config.end_transform.position =
		vec3_add(mover_config.start_transform.position, move_offset);
	mover_config.starts_at_end = starts_open;
	entity_set_collision_filter(&door->prop.entity, COLLISION_LAYER_DYNAMIC,
				    COLLISION_LAYER_WORLD_STATIC |
					    COLLISION_LAYER_DYNAMIC |
					    COLLISION_LAYER_PLAYER);
	if (!mover_initialize(&door->mover, &door->prop.entity,
			      &mover_config)) {
		set_error(context, "failed to initialize func_door mover");
		entity_destroy(&door->prop.entity);
		return NULL;
	}
	return &door->prop.entity;
}

static void update_entity(entity_t *entity, const float delta_time) {
	func_door_t *door;

	if (entity == NULL || delta_time < 0.0f) { return; }
	door = (func_door_t *)entity;
	mover_update(&door->mover, delta_time);
}

static bool accept_input(entity_t *entity,
			 const char *input_name,
			 const entity_input_context_t *context) {
	func_door_t *door;
	entity_t *activator;
	mover_state_t state;

	door = func_door_from_entity(entity);
	if (door == NULL || input_name == NULL) { return false; }
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
		(void)world_fire_output(entity->world, entity, "OnLockedUse",
					activator);
		return true;
	}

	state = mover_get_state(&door->mover);
	if (strcmp(input_name, "Open") == 0 || state == MOVER_AT_START ||
	    state == MOVER_MOVING_TO_START) {
		begin_open(door, activator);
	} else {
		begin_close(door, activator);
	}
	return true;
}

static void begin_open(func_door_t *door, entity_t *activator) {
	if (door == NULL) { return; }
	(void)mover_move_to_end(&door->mover, activator);
}

static void begin_close(func_door_t *door, entity_t *activator) {
	if (door == NULL) { return; }
	(void)mover_move_to_start(&door->mover, activator);
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
	return door == NULL ? FUNC_DOOR_CLOSED
			    : (func_door_state_t)mover_get_state(&door->mover);
}

bool func_door_is_blocked(const func_door_t *door) {
	return door != NULL && mover_is_blocked(&door->mover);
}

bool func_door_register(void) {
	return entity_register_class(&func_door_class);
}
