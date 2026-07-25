/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "entity/prop_breakable.h"
#include "entity/damage.h"
#include "entity/prop_internal.h"
#include "entity/world.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static entity_t *create_entity(entity_id_t id,
			       const entity_spawn_context_t *context);
static bool accept_input(entity_t *entity,
			 const char *input_name,
			 const entity_input_context_t *context);
static bool take_damage(entity_t *entity, const damage_info_t *damage);
static void
set_error(const entity_spawn_context_t *context, const char *format, ...);

static const entity_class_t prop_breakable_class = {
	.classname = "prop_breakable",
	.create = create_entity,
	.draw_shadow = prop_internal_draw_shadow,
	.draw = prop_internal_draw,
	.accept_input = accept_input,
	.take_damage = take_damage,
	.destroy = prop_internal_destroy,
};

static entity_t *create_entity(const entity_id_t id,
			       const entity_spawn_context_t *context) {
	prop_breakable_t *prop;
	const char *text;
	float maximum_health;

	if (context == NULL || context->properties == NULL ||
	    context->source == NULL) {
		return NULL;
	}
	prop = calloc(1, sizeof(*prop));
	if (prop == NULL) { return NULL; }
	if (!prop_internal_initialize(&prop->prop, id, &prop_breakable_class,
				      context)) {
		free(prop);
		return NULL;
	}

	maximum_health = 40.0f;
	text = entity_property_get(context->source, "health");
	if (text != NULL &&
	    (!entity_property_parse_float(text, &maximum_health) ||
	     maximum_health <= 0.0f)) {
		set_error(context,
			  "invalid positive prop_breakable health: \"%s\"",
			  text);
		entity_destroy(&prop->prop.entity);
		return NULL;
	}
	if (!health_initialize(&prop->health, maximum_health)) {
		entity_destroy(&prop->prop.entity);
		return NULL;
	}
	entity_set_collision_filter(&prop->prop.entity, COLLISION_LAYER_DYNAMIC,
				    COLLISION_LAYER_ALL);
	prop->prop.entity.collider_follows_transform = true;
	return &prop->prop.entity;
}

static bool accept_input(entity_t *entity,
			 const char *input_name,
			 const entity_input_context_t *context) {
	prop_breakable_t *prop;
	damage_info_t damage = {0};

	prop = prop_breakable_from_entity(entity);
	if (prop == NULL || input_name == NULL ||
	    strcmp(input_name, "Break") != 0) {
		return false;
	}
	damage.amount = health_get_current(&prop->health);
	damage.type = DAMAGE_TYPE_GENERIC;
	damage.attacker = context == NULL ? NULL : context->activator;
	damage.inflictor = context == NULL ? NULL : context->caller;
	damage.position = entity_get_world_position(entity);
	return entity_take_damage(entity, &damage);
}

static bool take_damage(entity_t *entity, const damage_info_t *damage) {
	prop_breakable_t *prop;
	float applied;

	prop = prop_breakable_from_entity(entity);
	if (prop == NULL || damage == NULL) { return false; }
	applied = health_apply_damage(&prop->health, damage->amount);
	if (applied <= 0.0f) { return false; }
	if (entity->world != NULL) {
		(void)world_fire_output(entity->world, entity, "OnDamaged",
					damage->attacker);
	}
	if (!health_is_alive(&prop->health)) {
		if (entity->world != NULL) {
			(void)world_fire_output(entity->world, entity,
						"OnBreak", damage->attacker);
		}
		entity->pending_destroy = true;
	}
	return true;
}

prop_breakable_t *prop_breakable_from_entity(entity_t *entity) {
	if (entity == NULL || entity->class != &prop_breakable_class) {
		return NULL;
	}
	return (prop_breakable_t *)entity;
}

float prop_breakable_get_health(const prop_breakable_t *prop) {
	return prop == NULL ? 0.0f : health_get_current(&prop->health);
}

bool prop_breakable_register(void) {
	return entity_register_class(&prop_breakable_class);
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
