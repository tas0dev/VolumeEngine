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
static bool move_towards(vec3_t *position, vec3_t target, float distance);
static void begin_release(func_button_t *button);
static void finish_release(func_button_t *button);
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
	return button != NULL && (button->state == FUNC_BUTTON_PRESSING ||
				  button->state == FUNC_BUTTON_PRESSED);
}

bool func_button_is_enabled(const func_button_t *button) {
	return button != NULL && button->enabled;
}

func_button_state_t func_button_get_state(const func_button_t *button) {
	return button == NULL ? FUNC_BUTTON_IDLE : button->state;
}

static entity_t *create_entity(const entity_id_t id,
			       const entity_spawn_context_t *context) {
	const char *text;
	func_button_t *button;
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

	button->wait = 1.0f;
	button->speed = 1.0f;
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
	    (!entity_property_parse_float(text, &button->speed) ||
	     button->speed <= 0.0f)) {
		set_error(context, "invalid positive func_button speed: \"%s\"",
			  text);
		entity_destroy(&button->prop.entity);
		return NULL;
	}

	text = entity_property_get(context->source, "wait");
	if (text != NULL &&
	    (!entity_property_parse_float(text, &button->wait) ||
	     (button->wait < 0.0f && button->wait != -1.0f))) {
		set_error(context, "invalid func_button wait: \"%s\"", text);
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
	button->prop.entity.collider_follows_transform = true;
	button->released_position = button->prop.entity.transform.position;
	button->pressed_position =
		vec3_add(button->released_position, move_offset);
	button->state = FUNC_BUTTON_IDLE;
	button->enabled = !starts_disabled;
	return &button->prop.entity;
}

static void update_entity(entity_t *entity, const float delta_time) {
	func_button_t *button;
	bool reached;

	if (entity == NULL || delta_time < 0.0f) { return; }
	button = (func_button_t *)entity;

	if (button->state == FUNC_BUTTON_PRESSING) {
		reached = move_towards(&entity->transform.position,
				       button->pressed_position,
				       button->speed * delta_time);
		if (reached) {
			button->state = FUNC_BUTTON_PRESSED;
			button->wait_remaining = button->wait;
			world_fire_output(
				entity->world, entity, "OnPressed",
				world_find_entity(entity->world,
						  button->activator_id));
		}
		return;
	}

	if (button->state == FUNC_BUTTON_PRESSED) {
		if (button->wait < 0.0f) { return; }
		button->wait_remaining -= delta_time;
		if (button->wait_remaining <= 0.0f) { begin_release(button); }
		return;
	}

	if (button->state != FUNC_BUTTON_RELEASING) { return; }
	reached = move_towards(&entity->transform.position,
			       button->released_position,
			       button->speed * delta_time);
	if (reached) { finish_release(button); }
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
		begin_release(button);
		return true;
	}
	if (strcmp(input_name, "Toggle") == 0) {
		button->enabled = !button->enabled;
		if (!button->enabled) { begin_release(button); }
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
		begin_release(button);
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
	if (button->state == FUNC_BUTTON_PRESSING ||
	    button->state == FUNC_BUTTON_PRESSED) {
		return true;
	}

	button->state = FUNC_BUTTON_PRESSING;
	button->activator_id = activator == NULL ? 0 : activator->id;
	return true;
}

static bool
move_towards(vec3_t *position, const vec3_t target, const float distance) {
	vec3_t delta;
	float length;

	delta = vec3_subtract(target, *position);
	length = vec3_length(delta);
	if (length <= distance + 0.000001f || length <= 0.000001f) {
		*position = target;
		return true;
	}
	*position = vec3_add(*position, vec3_scale(delta, distance / length));
	return false;
}

static void begin_release(func_button_t *button) {
	if (button == NULL || button->state == FUNC_BUTTON_IDLE ||
	    button->state == FUNC_BUTTON_RELEASING) {
		return;
	}
	button->state = FUNC_BUTTON_RELEASING;
	button->wait_remaining = 0.0f;
}

static void finish_release(func_button_t *button) {
	entity_t *activator;

	if (button == NULL || button->state != FUNC_BUTTON_RELEASING) {
		return;
	}
	activator = button->prop.entity.world == NULL
			    ? NULL
			    : world_find_entity(button->prop.entity.world,
						button->activator_id);
	button->state = FUNC_BUTTON_IDLE;
	button->wait_remaining = 0.0f;
	button->activator_id = 0;
	if (button->prop.entity.world != NULL) {
		world_fire_output(button->prop.entity.world,
				  &button->prop.entity, "OnReleased",
				  activator);
	}
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
