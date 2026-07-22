/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "entity/trigger.h"
#include "entity/world.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRIGGER_MAX_TOUCHING 64

typedef struct trigger {
	entity_t entity;
	entity_id_t touching[TRIGGER_MAX_TOUCHING];
	size_t touching_count;
	float wait;
	float cooldown;
	bool once;
} trigger_t;

static entity_t *create_once(entity_id_t id,
			     const entity_spawn_context_t *context);
static entity_t *create_multiple(entity_id_t id,
				 const entity_spawn_context_t *context);
static entity_t *create_trigger(entity_id_t id,
				const entity_spawn_context_t *context,
				bool once);
static void update_trigger(entity_t *entity, float delta_time);
static bool accept_input(entity_t *entity,
			 const char *input_name,
			 const entity_input_context_t *context);
static void destroy_trigger(entity_t *entity);
static bool contains_id(const entity_id_t *ids, size_t count, entity_id_t id);
static void reset_trigger(trigger_t *trigger);
static void
set_error(const entity_spawn_context_t *context, const char *format, ...);

static const entity_class_t trigger_once_class = {
	.classname = "trigger_once",
	.create = create_once,
	.update = update_trigger,
	.accept_input = accept_input,
	.destroy = destroy_trigger,
};

static const entity_class_t trigger_multiple_class = {
	.classname = "trigger_multiple",
	.create = create_multiple,
	.update = update_trigger,
	.accept_input = accept_input,
	.destroy = destroy_trigger,
};

bool trigger_once_register(void) {
	return entity_register_class(&trigger_once_class);
}

bool trigger_multiple_register(void) {
	return entity_register_class(&trigger_multiple_class);
}

static entity_t *create_once(const entity_id_t id,
			     const entity_spawn_context_t *context) {
	return create_trigger(id, context, true);
}

static entity_t *create_multiple(const entity_id_t id,
				 const entity_spawn_context_t *context) {
	return create_trigger(id, context, false);
}

static entity_t *create_trigger(const entity_id_t id,
				const entity_spawn_context_t *context,
				const bool once) {
	const char *text;
	trigger_t *trigger;
	vec3_t size;
	bool starts_disabled;

	if (context == NULL || context->properties == NULL) { return NULL; }

	size = vec3_create(1.0f, 1.0f, 1.0f);
	starts_disabled = false;

	text = entity_property_get(context->source, "size");
	if (text != NULL && !entity_property_parse_vec3(text, &size)) {
		set_error(context, "invalid trigger size: \"%s\"", text);
		return NULL;
	}
	if (size.x <= 0.0f || size.y <= 0.0f || size.z <= 0.0f) {
		set_error(context, "trigger size must be positive");
		return NULL;
	}

	trigger = calloc(1, sizeof(*trigger));
	if (trigger == NULL) { return NULL; }

	entity_initialize(&trigger->entity, id,
			  once ? &trigger_once_class : &trigger_multiple_class);
	trigger->entity.transform = context->properties->transform;
	trigger->once = once;
	trigger->wait = 0.5f;

	text = entity_property_get(context->source, "wait");
	if (!once && text != NULL &&
	    (!entity_property_parse_float(text, &trigger->wait) ||
	     trigger->wait < 0.0f)) {
		set_error(context, "invalid non-negative trigger wait: \"%s\"",
			  text);
		free(trigger);
		return NULL;
	}

	text = entity_property_get(context->source, "starts_disabled");
	if (text != NULL &&
	    !entity_property_parse_bool(text, &starts_disabled)) {
		set_error(context, "invalid trigger starts_disabled: \"%s\"",
			  text);
		free(trigger);
		return NULL;
	}

	entity_set_collider(&trigger->entity,
			    collider_create_box(vec3_create(0.0f, 0.0f, 0.0f),
						vec3_scale(size, 0.5f)));
	entity_set_collision_filter(&trigger->entity, COLLISION_LAYER_TRIGGER,
				    COLLISION_LAYER_PLAYER);
	trigger->entity.collider_follows_transform = true;
	trigger->entity.active = !starts_disabled;

	if (!entity_set_targetname(&trigger->entity,
				   context->properties->targetname)) {
		free(trigger);
		return NULL;
	}

	return &trigger->entity;
}

static void update_trigger(entity_t *entity, const float delta_time) {
	collision_filter_t filter;
	entity_id_t current[TRIGGER_MAX_TOUCHING];
	trigger_t *trigger;
	entity_t *activator;
	aabb_t bounds;
	size_t current_count;
	size_t index;
	bool had_touching;

	if (entity == NULL || entity->world == NULL || !entity->has_collider) {
		return;
	}

	trigger = (trigger_t *)entity;
	if (!collider_get_aabb(&entity->collider, entity->transform.position,
			       &bounds)) {
		return;
	}

	filter.layer = COLLISION_LAYER_TRIGGER;
	filter.mask = COLLISION_LAYER_PLAYER;
	filter.ignored_entity_id = entity->id;
	current_count = collision_world_query_aabb(
		world_get_const_collision_world(entity->world), bounds, filter,
		current, TRIGGER_MAX_TOUCHING);
	if (current_count > TRIGGER_MAX_TOUCHING) {
		current_count = TRIGGER_MAX_TOUCHING;
	}

	had_touching = trigger->touching_count > 0;
	for (index = 0; index < current_count; index++) {
		if (contains_id(trigger->touching, trigger->touching_count,
				current[index])) {
			continue;
		}
		activator = world_find_entity(entity->world, current[index]);
		world_fire_output(entity->world, entity, "OnStartTouch",
				  activator);
	}

	for (index = 0; index < trigger->touching_count; index++) {
		if (contains_id(current, current_count,
				trigger->touching[index])) {
			continue;
		}
		activator = world_find_entity(entity->world,
					      trigger->touching[index]);
		world_fire_output(entity->world, entity, "OnEndTouch",
				  activator);
	}

	if (!had_touching && current_count > 0) {
		activator = world_find_entity(entity->world, current[0]);
		world_fire_output(entity->world, entity, "OnStartTouchAll",
				  activator);
	}
	if (had_touching && current_count == 0) {
		activator =
			world_find_entity(entity->world, trigger->touching[0]);
		world_fire_output(entity->world, entity, "OnEndTouchAll",
				  activator);
	}

	memcpy(trigger->touching, current,
	       current_count * sizeof(*trigger->touching));
	trigger->touching_count = current_count;
	trigger->cooldown -= delta_time;

	if (current_count == 0 || trigger->cooldown > 0.0f) { return; }

	activator = world_find_entity(entity->world, current[0]);
	world_fire_output(entity->world, entity, "OnTrigger", activator);

	if (trigger->once) {
		entity_set_active(entity, false);
		trigger->touching_count = 0;
	} else {
		trigger->cooldown = trigger->wait;
	}
}

static bool accept_input(entity_t *entity,
			 const char *input_name,
			 const entity_input_context_t *context) {
	trigger_t *trigger;

	(void)context;
	if (entity == NULL || input_name == NULL) { return false; }
	trigger = (trigger_t *)entity;

	if (strcmp(input_name, "Enable") == 0) {
		reset_trigger(trigger);
		entity_set_active(entity, true);
		return true;
	}
	if (strcmp(input_name, "Disable") == 0) {
		reset_trigger(trigger);
		entity_set_active(entity, false);
		return true;
	}
	if (strcmp(input_name, "Toggle") == 0) {
		reset_trigger(trigger);
		entity_set_active(entity, !entity_is_active(entity));
		return true;
	}
	if (strcmp(input_name, "Reset") == 0) {
		reset_trigger(trigger);
		entity_set_active(entity, true);
		return true;
	}

	return false;
}

static void destroy_trigger(entity_t *entity) { free(entity); }

static bool
contains_id(const entity_id_t *ids, const size_t count, const entity_id_t id) {
	size_t index;

	for (index = 0; index < count; index++) {
		if (ids[index] == id) { return true; }
	}
	return false;
}

static void reset_trigger(trigger_t *trigger) {
	if (trigger == NULL) { return; }
	trigger->touching_count = 0;
	trigger->cooldown = 0.0f;
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
