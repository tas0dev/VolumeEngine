/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "entity/trigger_hurt.h"
#include "entity/damage.h"
#include "entity/entity.h"
#include "entity/world.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRIGGER_HURT_MAXIMUM_TARGETS 64

typedef struct trigger_hurt {
	entity_t entity;
	float damage;
	float damage_interval;
	float cooldown;
} trigger_hurt_t;

static entity_t *create_entity(entity_id_t id,
			       const entity_spawn_context_t *context);
static void update_entity(entity_t *entity, float delta_time);
static bool accept_input(entity_t *entity,
			 const char *input_name,
			 const entity_input_context_t *context);
static void destroy_entity(entity_t *entity);
static void
set_error(const entity_spawn_context_t *context, const char *format, ...);

static const entity_class_t trigger_hurt_class = {
	.classname = "trigger_hurt",
	.create = create_entity,
	.update = update_entity,
	.accept_input = accept_input,
	.destroy = destroy_entity,
};

bool trigger_hurt_register(void) {
	return entity_register_class(&trigger_hurt_class);
}

static entity_t *create_entity(const entity_id_t id,
			       const entity_spawn_context_t *context) {
	const char *text;
	trigger_hurt_t *trigger;
	vec3_t size;
	bool starts_disabled;

	if (context == NULL || context->properties == NULL) { return NULL; }
	size = vec3_create(1.0f, 1.0f, 1.0f);
	starts_disabled = false;

	text = entity_property_get(context->source, "size");
	if (text != NULL && !entity_property_parse_vec3(text, &size)) {
		set_error(context, "invalid trigger_hurt size: \"%s\"", text);
		return NULL;
	}
	if (size.x <= 0.0f || size.y <= 0.0f || size.z <= 0.0f) {
		set_error(context, "trigger_hurt size must be positive");
		return NULL;
	}

	trigger = calloc(1, sizeof(*trigger));
	if (trigger == NULL) { return NULL; }
	entity_initialize(&trigger->entity, id, &trigger_hurt_class);
	trigger->entity.transform = context->properties->transform;
	trigger->damage = 10.0f;
	trigger->damage_interval = 0.5f;

	text = entity_property_get(context->source, "damage");
	if (text != NULL &&
	    (!entity_property_parse_float(text, &trigger->damage) ||
	     trigger->damage <= 0.0f)) {
		set_error(context,
			  "invalid positive trigger_hurt damage: \"%s\"", text);
		free(trigger);
		return NULL;
	}
	text = entity_property_get(context->source, "damage_interval");
	if (text != NULL &&
	    (!entity_property_parse_float(text, &trigger->damage_interval) ||
	     trigger->damage_interval <= 0.0f)) {
		set_error(
			context,
			"invalid positive trigger_hurt damage_interval: \"%s\"",
			text);
		free(trigger);
		return NULL;
	}
	text = entity_property_get(context->source, "starts_disabled");
	if (text != NULL &&
	    !entity_property_parse_bool(text, &starts_disabled)) {
		set_error(context,
			  "invalid trigger_hurt starts_disabled: \"%s\"", text);
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

static void update_entity(entity_t *entity, const float delta_time) {
	entity_id_t targets[TRIGGER_HURT_MAXIMUM_TARGETS];
	trigger_hurt_t *trigger;
	collision_filter_t filter;
	damage_info_t damage = {0};
	entity_t *target;
	aabb_t bounds;
	size_t count;
	size_t index;

	if (entity == NULL || entity->world == NULL || !entity->has_collider) {
		return;
	}
	trigger = (trigger_hurt_t *)entity;
	trigger->cooldown -= delta_time;
	if (trigger->cooldown > 0.0f) { return; }
	if (!collider_get_aabb(&entity->collider, entity->transform.position,
			       &bounds)) {
		return;
	}
	filter.layer = COLLISION_LAYER_TRIGGER;
	filter.mask = COLLISION_LAYER_PLAYER;
	filter.ignored_entity_id = entity->id;
	count = collision_world_query_aabb(
		world_get_const_collision_world(entity->world), bounds, filter,
		targets, TRIGGER_HURT_MAXIMUM_TARGETS);
	if (count > TRIGGER_HURT_MAXIMUM_TARGETS) {
		count = TRIGGER_HURT_MAXIMUM_TARGETS;
	}
	if (count == 0) { return; }

	trigger->cooldown = trigger->damage_interval;
	for (index = 0; index < count; index++) {
		target = world_find_entity(entity->world, targets[index]);
		damage.amount = trigger->damage;
		damage.type = DAMAGE_TYPE_GENERIC;
		damage.attacker = entity;
		damage.inflictor = entity;
		damage.position = target == NULL
					  ? entity_get_world_position(entity)
					  : entity_get_world_position(target);
		if (entity_take_damage(target, &damage)) {
			(void)world_fire_output(entity->world, entity, "OnHurt",
						target);
		}
	}
}

static bool accept_input(entity_t *entity,
			 const char *input_name,
			 const entity_input_context_t *context) {
	trigger_hurt_t *trigger;

	(void)context;
	if (entity == NULL || input_name == NULL) { return false; }
	trigger = (trigger_hurt_t *)entity;
	if (strcmp(input_name, "Enable") == 0) {
		trigger->cooldown = 0.0f;
		entity_set_active(entity, true);
		return true;
	}
	if (strcmp(input_name, "Disable") == 0) {
		trigger->cooldown = 0.0f;
		entity_set_active(entity, false);
		return true;
	}
	if (strcmp(input_name, "Toggle") == 0) {
		trigger->cooldown = 0.0f;
		entity_set_active(entity, !entity_is_active(entity));
		return true;
	}
	return false;
}

static void destroy_entity(entity_t *entity) { free(entity); }

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
