/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "entity/player.h"
#include "entity/world.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

struct player {
	entity_t entity;
	character_controller_t controller;
};

static entity_t *create_entity(entity_id_t id,
			       const entity_spawn_context_t *context);
static void destroy_entity(entity_t *entity);
static void
set_error(const entity_spawn_context_t *context, const char *format, ...);

static const entity_class_t player_class = {
	.classname = "player",
	.create = create_entity,
	.destroy = destroy_entity,
};

bool player_register(void) { return entity_register_class(&player_class); }

player_t *player_from_entity(entity_t *entity) {
	if (entity == NULL || entity->class != &player_class) { return NULL; }
	return (player_t *)entity;
}

const player_t *player_from_const_entity(const entity_t *entity) {
	if (entity == NULL || entity->class != &player_class) { return NULL; }
	return (const player_t *)entity;
}

entity_t *player_get_entity(player_t *player) {
	return player == NULL ? NULL : &player->entity;
}

const entity_t *player_get_const_entity(const player_t *player) {
	return player == NULL ? NULL : &player->entity;
}

vec3_t player_get_position(const player_t *player) {
	if (player == NULL) { return vec3_create(0.0f, 0.0f, 0.0f); }
	return player->controller.position;
}

vec3_t player_get_view_position(const player_t *player) {
	if (player == NULL) { return vec3_create(0.0f, 0.0f, 0.0f); }
	return vec3_add(
		player->controller.position,
		vec3_create(0.0f, player->controller.view_height, 0.0f));
}

bool player_is_crouched(const player_t *player) {
	return player != NULL && player->controller.crouched;
}

void player_move(player_t *player,
		 const character_move_input_t *input,
		 const float delta_time) {
	const collision_world_t *collision_world;
	collision_filter_t filter;

	if (player == NULL) { return; }

	collision_world =
		player->entity.world == NULL
			? NULL
			: world_get_const_collision_world(player->entity.world);

	filter.layer = player->entity.collision_layer;
	filter.mask = player->entity.collision_mask;
	filter.ignored_entity_id = player->entity.id;
	character_controller_move_filtered(&player->controller, collision_world,
					   filter, input, delta_time);
	player->entity.transform.position = player->controller.position;
	player->entity.collider = collider_create_box(
		aabb_get_center(player->controller.bounds),
		aabb_get_half_extents(player->controller.bounds));

	if (player->entity.world != NULL) {
		collision_world_update_collider_filtered(
			world_get_collision_world(player->entity.world),
			player->entity.id, player->entity.collider,
			player->entity.transform.position,
			player->entity.collision_layer,
			player->entity.collision_mask);
	}
}

static entity_t *create_entity(const entity_id_t id,
			       const entity_spawn_context_t *context) {
	const char *text;
	player_t *player;
	vec3_t center;
	vec3_t half_extents;
	float radius;
	float height;

	if (context == NULL || context->properties == NULL) { return NULL; }

	radius = 0.35f;
	height = 1.7f;

	text = entity_property_get(context->source, "radius");
	if (text != NULL &&
	    (!entity_property_parse_float(text, &radius) || radius <= 0.0f)) {
		set_error(context, "invalid positive player radius: \"%s\"",
			  text);
		return NULL;
	}

	text = entity_property_get(context->source, "height");
	if (text != NULL &&
	    (!entity_property_parse_float(text, &height) || height <= 0.0f)) {
		set_error(context, "invalid positive player height: \"%s\"",
			  text);
		return NULL;
	}

	player = calloc(1, sizeof(*player));
	if (player == NULL) { return NULL; }

	entity_initialize(&player->entity, id, &player_class);
	player->entity.transform = context->properties->transform;
	player->controller = character_controller_create(
		player->entity.transform.position, radius, height);

	center = aabb_get_center(player->controller.bounds);
	half_extents = aabb_get_half_extents(player->controller.bounds);
	entity_set_collider(&player->entity,
			    collider_create_box(center, half_extents));
	entity_set_collision_filter(&player->entity, COLLISION_LAYER_PLAYER,
				    COLLISION_LAYER_WORLD_STATIC |
					    COLLISION_LAYER_DYNAMIC |
					    COLLISION_LAYER_PLAYER);
	player->entity.collider_follows_transform = true;

	if (!entity_set_targetname(&player->entity,
				   context->properties->targetname)) {
		free(player);
		return NULL;
	}

	return &player->entity;
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
