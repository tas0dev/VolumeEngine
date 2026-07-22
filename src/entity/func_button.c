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
static void release_button(func_button_t *button);
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
	return button != NULL && button->pressed;
}

bool func_button_is_enabled(const func_button_t *button) {
	return button != NULL && button->enabled;
}

static entity_t *create_entity(const entity_id_t id,
			       const entity_spawn_context_t *context) {
	const char *text;
	func_button_t *button;
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
	starts_disabled = false;

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
				    COLLISION_LAYER_WORLD_STATIC,
				    COLLISION_LAYER_ALL);
	button->enabled = !starts_disabled;
	return &button->prop.entity;
}

static void update_entity(entity_t *entity, const float delta_time) {
	func_button_t *button;

	if (entity == NULL || delta_time < 0.0f) { return; }
	button = (func_button_t *)entity;
	if (!button->pressed || button->wait < 0.0f) { return; }

	button->wait_remaining -= delta_time;
	if (button->wait_remaining <= 0.0f) { release_button(button); }
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
		button->pressed = false;
		button->wait_remaining = 0.0f;
		button->activator_id = 0;
		return true;
	}
	if (strcmp(input_name, "Toggle") == 0) {
		button->enabled = !button->enabled;
		if (!button->enabled) {
			button->pressed = false;
			button->wait_remaining = 0.0f;
			button->activator_id = 0;
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
		if (button->pressed) { release_button(button); }
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
	if (button->pressed) { return true; }

	button->pressed = true;
	button->wait_remaining = button->wait;
	button->activator_id = activator == NULL ? 0 : activator->id;
	world_fire_output(entity->world, entity, "OnPressed", activator);
	return true;
}

static void release_button(func_button_t *button) {
	entity_t *activator;

	if (button == NULL || !button->pressed) { return; }
	activator = button->prop.entity.world == NULL
			    ? NULL
			    : world_find_entity(button->prop.entity.world,
						button->activator_id);
	button->pressed = false;
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
