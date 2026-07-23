/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "entity/player.h"
#include "entity/func_ladder.h"
#include "entity/world.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

struct player {
	entity_t entity;
	character_controller_t controller;
	entity_id_t ladder_id;
	float ladder_detach_time;
};

static entity_t *create_entity(entity_id_t id,
			       const entity_spawn_context_t *context);
static void destroy_entity(entity_t *entity);
static void sync_player_transform_and_collider(player_t *player);
static func_ladder_t *find_touching_ladder(const player_t *player,
					   entity_id_t preferred_id);
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

vec3_t player_get_velocity(const player_t *player) {
	if (player == NULL) { return vec3_create(0.0f, 0.0f, 0.0f); }
	return player->controller.velocity;
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

bool player_is_surfing(const player_t *player) {
	return player != NULL && player->controller.surfing;
}

bool player_is_on_ladder(const player_t *player) {
	return player != NULL && player->controller.on_ladder;
}

entity_id_t player_get_ground_entity_id(const player_t *player) {
	if (player == NULL || !player->controller.grounded) { return 0; }
	return player->controller.ground_entity_id;
}

bool player_is_grounded_on(const player_t *player,
			   const entity_id_t entity_id) {
	return entity_id != 0 &&
	       player_get_ground_entity_id(player) == entity_id;
}

bool player_can_move_with_platform(const player_t *player,
				   const entity_id_t platform_id,
				   const vec3_t displacement) {
	collision_filter_t filter;
	collision_trace_t trace;
	vec3_t end;

	if (player == NULL || player->entity.world == NULL ||
	    platform_id == 0) {
		return false;
	}
	if (vec3_length(displacement) <= 0.000001f) { return true; }

	filter.layer = player->entity.collision_layer;
	filter.mask = player->entity.collision_mask;
	filter.ignored_entity_id = player->entity.id;
	end = vec3_add(player->controller.position, displacement);
	if (!collision_world_trace_aabb_filtered_ignoring(
		    world_get_const_collision_world(player->entity.world),
		    player->controller.bounds, player->controller.position, end,
		    filter, platform_id, &trace)) {
		return true;
	}

	return !trace.started_inside && trace.fraction >= 0.999999f;
}

bool player_move_with_platform(player_t *player,
			       const entity_id_t platform_id,
			       const vec3_t displacement) {
	if (!player_can_move_with_platform(player, platform_id, displacement)) {
		return false;
	}

	player->controller.position =
		vec3_add(player->controller.position, displacement);
	sync_player_transform_and_collider(player);
	return true;
}

void player_move(player_t *player,
		 const character_move_input_t *input,
		 const float delta_time) {
	const collision_world_t *collision_world;
	collision_filter_t filter;
	character_move_input_t adjusted_input;
	const character_move_input_t *movement_input;
	entity_t *ground_entity;
	entity_t *ladder_entity;
	func_ladder_t *ladder;
	vec3_t base_velocity;
	vec3_t ladder_normal;
	vec3_t wish_direction;
	bool entering_ladder;

	if (player == NULL) { return; }

	collision_world =
		player->entity.world == NULL
			? NULL
			: world_get_const_collision_world(player->entity.world);

	filter.layer = player->entity.collision_layer;
	filter.mask = player->entity.collision_mask;
	filter.ignored_entity_id = player->entity.id;
	adjusted_input = input == NULL ? (character_move_input_t){0} : *input;
	adjusted_input.ladder = false;
	adjusted_input.ladder_normal = vec3_create(0.0f, 0.0f, 0.0f);
	movement_input = &adjusted_input;
	player->ladder_detach_time -= delta_time;
	ladder = find_touching_ladder(player, player->ladder_id);
	if (ladder == NULL) { player->ladder_id = 0; }

	entering_ladder = false;
	if (ladder != NULL && input != NULL &&
	    player->ladder_detach_time <= 0.0f) {
		ladder_entity = func_ladder_get_entity(ladder);
		ladder_normal = func_ladder_get_normal(ladder);
		wish_direction = input->wish_direction;
		wish_direction.y = 0.0f;
		if (vec3_length(wish_direction) > 0.000001f) {
			wish_direction = vec3_normalize(wish_direction);
		}
		entering_ladder =
			player->ladder_id == ladder_entity->id ||
			vec3_dot(wish_direction, ladder_normal) < -0.1f;
		if (entering_ladder) { player->ladder_id = ladder_entity->id; }
	}

	if (entering_ladder && input->jump) {
		player->controller.on_ladder = false;
		player->controller.grounded = false;
		player->controller.ground_entity_id = 0;
		player->controller.velocity = vec3_add(
			vec3_scale(ladder_normal,
				   player->controller.jump_speed * 0.6f),
			vec3_create(0.0f, player->controller.jump_speed * 0.25f,
				    0.0f));
		player->ladder_id = 0;
		player->ladder_detach_time = 0.2f;
		adjusted_input.jump = false;
	} else if (entering_ladder) {
		adjusted_input.ladder = true;
		adjusted_input.ladder_normal = ladder_normal;
	}

	if (input != NULL && input->jump && player->controller.grounded) {
		ground_entity =
			player->entity.world == NULL
				? NULL
				: world_find_entity(
					  player->entity.world,
					  player->controller.ground_entity_id);
		base_velocity = ground_entity == NULL
					? vec3_create(0.0f, 0.0f, 0.0f)
					: ground_entity->linear_velocity;
		if (character_controller_jump(&player->controller)) {
			player->controller.velocity = vec3_add(
				player->controller.velocity, base_velocity);
			adjusted_input.jump = false;
		}
	}
	character_controller_move_filtered(&player->controller, collision_world,
					   filter, movement_input, delta_time);
	sync_player_transform_and_collider(player);
}

static void sync_player_transform_and_collider(player_t *player) {
	if (player == NULL) { return; }

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

static func_ladder_t *find_touching_ladder(const player_t *player,
					   const entity_id_t preferred_id) {
	const size_t maximum_candidates = 16;
	collision_filter_t filter;
	entity_id_t candidates[16];
	func_ladder_t *fallback;
	func_ladder_t *ladder;
	entity_t *entity;
	aabb_t bounds;
	size_t candidate_count;
	size_t index;

	if (player == NULL || player->entity.world == NULL) { return NULL; }

	bounds = aabb_translate(player->controller.bounds,
				player->controller.position);
	filter.layer = COLLISION_LAYER_PLAYER;
	filter.mask = COLLISION_LAYER_TRIGGER;
	filter.ignored_entity_id = player->entity.id;
	candidate_count = collision_world_query_aabb(
		world_get_const_collision_world(player->entity.world), bounds,
		filter, candidates, maximum_candidates);
	if (candidate_count > maximum_candidates) {
		candidate_count = maximum_candidates;
	}

	fallback = NULL;
	for (index = 0; index < candidate_count; index++) {
		entity = world_find_entity(player->entity.world,
					   candidates[index]);
		ladder = func_ladder_from_entity(entity);
		if (ladder == NULL || !entity_is_active(entity)) { continue; }
		if (entity->id == preferred_id) { return ladder; }
		if (fallback == NULL) { fallback = ladder; }
	}
	return fallback;
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

bool player_get_collision_debug_state(const player_t *player,
				      character_debug_state_t *state) {
	if (player == NULL) { return false; }

	return character_controller_get_debug_state(&player->controller, state);
}
