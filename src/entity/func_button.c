/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "entity/func_button.h"
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

static const entity_class_t func_button_class = {
	.classname = "func_button",
	.create = create_entity,
	.update = update_entity,
	.draw_shadow = prop_internal_draw_shadow,
	.draw = prop_internal_draw,
	.accept_input = accept_input,
	.destroy = prop_internal_destroy,
};

bool func_button_register(void) {
	return entity_register_class(&func_button_class);
}

func_button_t *func_button_from_entity(entity_t *entity) {
	if (entity == NULL || entity->class != &func_button_class) {
		return NULL;
	}
	return (func_button_t *)entity;
}

const func_button_t *func_button_from_const_entity(const entity_t *entity) {
	if (entity == NULL || entity->class != &func_button_class) {
		return NULL;
	}
	return (const func_button_t *)entity;
}

bool func_button_is_pressed(const func_button_t *button) {
	return button != NULL &&
	       (mover_get_state(&button->mover) == MOVER_MOVING_TO_END ||
		mover_get_state(&button->mover) == MOVER_AT_END);
}

bool func_button_is_enabled(const func_button_t *button) {
	return button != NULL && button->enabled;
}

func_button_state_t func_button_get_state(const func_button_t *button) {
	return button == NULL
		       ? FUNC_BUTTON_IDLE
		       : (func_button_state_t)mover_get_state(&button->mover);
}

static entity_t *create_entity(const entity_id_t id,
			       const entity_spawn_context_t *context) {
	const char *text;
	func_button_t *button;
	mover_config_t mover_config = {0};
	vec3_t move_offset;
	bool starts_disabled;

	if (context == NULL || context->properties == NULL ||
	    context->source == NULL) {
		return NULL;
	}

	button = calloc(1, sizeof(*button));
	if (button == NULL) { return NULL; }
	if (!prop_internal_initialize(&button->prop, id, &func_button_class,
				      context)) {
		free(button);
		return NULL;
	}

	mover_config.speed = 1.0f;
	mover_config.wait = 1.0f;
	mover_config.block_policy = MOVER_BLOCK_STOP;
	mover_config.sweep_collider = false;
	mover_config.move_riders = true;
	mover_config.outputs.on_reached_end = "OnPressed";
	mover_config.outputs.on_reached_start = "OnReleased";
	mover_config.outputs.on_blocked = "OnBlocked";
	mover_config.outputs.on_unblocked = "OnUnblocked";
	move_offset = vec3_create(0.0f, 0.0f, -0.1f);
	starts_disabled = false;

	text = entity_property_get(context->source, "move_offset");
	if (text != NULL && !entity_property_parse_vec3(text, &move_offset)) {
		set_error(context, "invalid func_button move_offset: \"%s\"",
			  text);
		entity_destroy(&button->prop.entity);
		return NULL;
	}
	if (vec3_length(move_offset) <= 0.000001f) {
		set_error(context, "func_button move_offset cannot be zero");
		entity_destroy(&button->prop.entity);
		return NULL;
	}

	text = entity_property_get(context->source, "speed");
	if (text != NULL &&
	    (!entity_property_parse_float(text, &mover_config.speed) ||
	     mover_config.speed <= 0.0f)) {
		set_error(context, "invalid positive func_button speed: \"%s\"",
			  text);
		entity_destroy(&button->prop.entity);
		return NULL;
	}

	text = entity_property_get(context->source, "wait");
	if (text != NULL &&
	    (!entity_property_parse_float(text, &mover_config.wait) ||
	     (mover_config.wait < 0.0f && mover_config.wait != -1.0f))) {
		set_error(context, "invalid func_button wait: \"%s\"", text);
		entity_destroy(&button->prop.entity);
		return NULL;
	}

	text = entity_property_get(context->source, "acceleration");
	if (text != NULL &&
	    (!entity_property_parse_float(text, &mover_config.acceleration) ||
	     mover_config.acceleration < 0.0f)) {
		set_error(
			context,
			"invalid non-negative func_button acceleration: \"%s\"",
			text);
		entity_destroy(&button->prop.entity);
		return NULL;
	}

	text = entity_property_get(context->source, "deceleration");
	if (text != NULL &&
	    (!entity_property_parse_float(text, &mover_config.deceleration) ||
	     mover_config.deceleration < 0.0f)) {
		set_error(
			context,
			"invalid non-negative func_button deceleration: \"%s\"",
			text);
		entity_destroy(&button->prop.entity);
		return NULL;
	}

	text = entity_property_get(context->source, "block_policy");
	if (text != NULL &&
	    !mover_parse_block_policy(text, &mover_config.block_policy)) {
		set_error(context, "invalid func_button block_policy: \"%s\"",
			  text);
		entity_destroy(&button->prop.entity);
		return NULL;
	}

	text = entity_property_get(context->source, "locked");
	if (text != NULL &&
	    !entity_property_parse_bool(text, &button->locked)) {
		set_error(context, "invalid func_button locked: \"%s\"", text);
		entity_destroy(&button->prop.entity);
		return NULL;
	}

	text = entity_property_get(context->source, "starts_disabled");
	if (text != NULL &&
	    !entity_property_parse_bool(text, &starts_disabled)) {
		set_error(context,
			  "invalid func_button starts_disabled: \"%s\"", text);
		entity_destroy(&button->prop.entity);
		return NULL;
	}

	entity_set_collision_filter(&button->prop.entity,
				    COLLISION_LAYER_DYNAMIC,
				    COLLISION_LAYER_ALL);
	mover_config.start_transform = button->prop.entity.transform;
	mover_config.end_transform = mover_config.start_transform;
	mover_config.end_transform.position =
		vec3_add(mover_config.start_transform.position, move_offset);
	if (!mover_initialize(&button->mover, &button->prop.entity,
			      &mover_config)) {
		set_error(context, "failed to initialize func_button mover");
		entity_destroy(&button->prop.entity);
		return NULL;
	}
	button->enabled = !starts_disabled;
	return &button->prop.entity;
}

static void update_entity(entity_t *entity, const float delta_time) {
	func_button_t *button;

	if (entity == NULL || delta_time < 0.0f) { return; }
	button = (func_button_t *)entity;
	mover_update(&button->mover, delta_time);
}

static bool accept_input(entity_t *entity,
			 const char *input_name,
			 const entity_input_context_t *context) {
	func_button_t *button;
	entity_t *activator;

	if (entity == NULL || input_name == NULL) { return false; }

	button = (func_button_t *)entity;
	activator = context == NULL ? NULL : context->activator;

	if (strcmp(input_name, "Enable") == 0) {
		button->enabled = true;
		return true;
	}

	if (strcmp(input_name, "Disable") == 0) {
		button->enabled = false;
		(void)mover_move_to_start(&button->mover,
					  mover_get_activator(&button->mover));
		return true;
	}

	if (strcmp(input_name, "Toggle") == 0) {
		button->enabled = !button->enabled;

		if (!button->enabled) {
			(void)mover_move_to_start(
				&button->mover,
				mover_get_activator(&button->mover));
		}

		return true;
	}

	if (strcmp(input_name, "Lock") == 0) {
		button->locked = true;
		return true;
	}

	if (strcmp(input_name, "Unlock") == 0) {
		button->locked = false;
		return true;
	}

	if (strcmp(input_name, "Reset") == 0) {
		(void)mover_move_to_start(&button->mover,
					  mover_get_activator(&button->mover));
		return true;
	}

	if (strcmp(input_name, "Use") != 0 &&
	    strcmp(input_name, "Press") != 0) {
		return false;
	    }

	if (!button->enabled) { return false; }

	if (button->locked) {
		world_fire_output(entity->world, entity, "OnLockedUse",
				  activator);
		return true;
	}

	if (mover_get_state(&button->mover) == MOVER_MOVING_TO_END ||
	    mover_get_state(&button->mover) == MOVER_AT_END) {
		return true;
	    }

	if (!mover_move_to_end(&button->mover, activator)) { return false; }

	    world_fire_output(entity->world, entity, "OnPressed", activator);

	    return true;
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
