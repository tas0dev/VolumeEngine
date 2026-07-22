/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "entity/mover.h"
#include "entity/player.h"
#include "entity/world.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

static float get_step_distance(mover_t *mover, vec3_t target, float delta_time);
static transform_t interpolate_transform(const transform_t *start,
					 const transform_t *end,
					 float fraction);
static transform_t transform_at_position(const mover_t *mover, vec3_t position);
static bool try_move(mover_t *mover, const transform_t *next_transform);
static bool
trace_movement(const mover_t *mover, vec3_t end, collision_trace_t *trace);
static bool trace_hit_rider(const mover_t *mover,
			    const collision_trace_t *trace);
static bool move_contacting_riders(mover_t *mover,
				   vec3_t displacement,
				   entity_id_t *blocker_id);
static void handle_blocked(mover_t *mover, entity_id_t blocker_id);
static void handle_unblocked(mover_t *mover);
static void reverse_after_block(mover_t *mover);
static void
fire_output(mover_t *mover, const char *output_name, entity_t *activator);

bool mover_initialize(mover_t *mover,
		      entity_t *entity,
		      const mover_config_t *config) {
	if (mover == NULL || entity == NULL || config == NULL ||
	    config->speed <= 0.0f || config->acceleration < 0.0f ||
	    config->deceleration < 0.0f ||
	    (config->wait < 0.0f && config->wait != -1.0f)) {
		return false;
	}

	memset(mover, 0, sizeof(*mover));
	mover->entity = entity;
	mover->start_transform = config->start_transform;
	mover->end_transform = config->end_transform;
	mover->speed = config->speed;
	mover->acceleration = config->acceleration;
	mover->deceleration = config->deceleration;
	mover->wait = config->wait;
	mover->wait_remaining = config->wait;
	mover->block_policy = config->block_policy;
	mover->sweep_collider = config->sweep_collider;
	mover->move_riders = config->move_riders;
	mover->outputs = config->outputs;
	mover->state = config->starts_at_end ? MOVER_AT_END : MOVER_AT_START;
	entity->transform = config->starts_at_end ? config->end_transform
						  : config->start_transform;
	entity->collider_follows_transform = true;
	return true;
}

bool mover_move_to_end(mover_t *mover, entity_t *activator) {
	if (mover == NULL || mover->entity == NULL ||
	    mover->state == MOVER_AT_END ||
	    mover->state == MOVER_MOVING_TO_END) {
		return false;
	}

	mover->state = MOVER_MOVING_TO_END;
	mover->current_speed = 0.0f;
	mover->wait_remaining = 0.0f;
	mover->activator_id = activator == NULL ? 0 : activator->id;
	fire_output(mover, mover->outputs.on_move_to_end, activator);
	return true;
}

bool mover_move_to_start(mover_t *mover, entity_t *activator) {
	if (mover == NULL || mover->entity == NULL ||
	    mover->state == MOVER_AT_START ||
	    mover->state == MOVER_MOVING_TO_START) {
		return false;
	}

	mover->state = MOVER_MOVING_TO_START;
	mover->current_speed = 0.0f;
	mover->wait_remaining = 0.0f;
	mover->activator_id = activator == NULL ? 0 : activator->id;
	fire_output(mover, mover->outputs.on_move_to_start, activator);
	return true;
}

void mover_update(mover_t *mover, const float delta_time) {
	const transform_t *destination;
	transform_t next_transform;
	vec3_t next_position;
	bool reached;

	if (mover == NULL || mover->entity == NULL || delta_time < 0.0f) {
		return;
	}

	if (mover->state == MOVER_AT_END) {
		if (mover->wait < 0.0f) { return; }
		mover->wait_remaining -= delta_time;
		if (mover->wait_remaining <= 0.0f) {
			(void)mover_move_to_start(mover,
						  mover_get_activator(mover));
		}
		return;
	}

	if ((mover->state != MOVER_MOVING_TO_END &&
	     mover->state != MOVER_MOVING_TO_START) ||
	    delta_time <= 0.0f) {
		return;
	}

	destination = mover->state == MOVER_MOVING_TO_END
			      ? &mover->end_transform
			      : &mover->start_transform;
	next_position = mover->entity->transform.position;
	reached = mover_move_towards(
		&next_position, destination->position,
		get_step_distance(mover, destination->position, delta_time));

	if (vec3_length(vec3_subtract(mover->end_transform.position,
				      mover->start_transform.position)) <=
	    0.000001f) {
		reached = true;
	}
	next_transform = transform_at_position(mover, next_position);

	if (!try_move(mover, &next_transform)) { return; }
	if (!reached) { return; }

	mover->current_speed = 0.0f;
	if (mover->state == MOVER_MOVING_TO_END) {
		mover->state = MOVER_AT_END;
		mover->wait_remaining = mover->wait;
		fire_output(mover, mover->outputs.on_reached_end,
			    mover_get_activator(mover));
	} else {
		mover->state = MOVER_AT_START;
		mover->wait_remaining = 0.0f;
		fire_output(mover, mover->outputs.on_reached_start,
			    mover_get_activator(mover));
	}
}

bool mover_move_towards(vec3_t *position,
			const vec3_t target,
			const float distance) {
	vec3_t delta;
	float length;

	if (position == NULL || distance < 0.0f) { return false; }
	delta = vec3_subtract(target, *position);
	length = vec3_length(delta);
	if (length <= distance + 0.000001f || length <= 0.000001f) {
		*position = target;
		return true;
	}
	*position = vec3_add(*position, vec3_scale(delta, distance / length));
	return false;
}

mover_state_t mover_get_state(const mover_t *mover) {
	return mover == NULL ? MOVER_AT_START : mover->state;
}

bool mover_is_blocked(const mover_t *mover) {
	return mover != NULL && mover->blocked;
}

entity_t *mover_get_activator(const mover_t *mover) {
	if (mover == NULL || mover->entity == NULL ||
	    mover->entity->world == NULL || mover->activator_id == 0) {
		return NULL;
	}
	return world_find_entity(mover->entity->world, mover->activator_id);
}

bool mover_parse_block_policy(const char *text, mover_block_policy_t *policy) {
	if (text == NULL || policy == NULL) { return false; }
	if (strcmp(text, "stop") == 0) {
		*policy = MOVER_BLOCK_STOP;
		return true;
	}
	if (strcmp(text, "reverse") == 0) {
		*policy = MOVER_BLOCK_REVERSE;
		return true;
	}
	if (strcmp(text, "ignore") == 0) {
		*policy = MOVER_BLOCK_IGNORE;
		return true;
	}
	return false;
}

static float
get_step_distance(mover_t *mover, const vec3_t target, const float delta_time) {
	float desired_speed;
	float remaining;
	float change;

	remaining = vec3_length(
		vec3_subtract(target, mover->entity->transform.position));
	desired_speed = mover->speed;
	if (mover->deceleration > 0.0f) {
		desired_speed =
			fminf(desired_speed,
			      sqrtf(2.0f * mover->deceleration * remaining));
	}

	if (mover->current_speed < desired_speed) {
		if (mover->acceleration <= 0.0f) {
			mover->current_speed = desired_speed;
		} else {
			change = mover->acceleration * delta_time;
			mover->current_speed = fminf(
				desired_speed, mover->current_speed + change);
		}
	} else if (mover->current_speed > desired_speed) {
		if (mover->deceleration <= 0.0f) {
			mover->current_speed = desired_speed;
		} else {
			change = mover->deceleration * delta_time;
			mover->current_speed = fmaxf(
				desired_speed, mover->current_speed - change);
		}
	}

	return fminf(remaining, mover->current_speed * delta_time);
}

static transform_t interpolate_transform(const transform_t *start,
					 const transform_t *end,
					 const float fraction) {
	transform_t result;

	result.position = vec3_add(
		start->position,
		vec3_scale(vec3_subtract(end->position, start->position),
			   fraction));
	result.rotation = vec3_add(
		start->rotation,
		vec3_scale(vec3_subtract(end->rotation, start->rotation),
			   fraction));
	result.scale = vec3_add(
		start->scale,
		vec3_scale(vec3_subtract(end->scale, start->scale), fraction));
	return result;
}

static transform_t transform_at_position(const mover_t *mover,
					 const vec3_t position) {
	transform_t result;
	vec3_t segment;
	float distance;
	float fraction;

	segment = vec3_subtract(mover->end_transform.position,
				mover->start_transform.position);
	distance = vec3_length(segment);
	if (distance <= 0.000001f) {
		fraction = mover->state == MOVER_MOVING_TO_END ? 1.0f : 0.0f;
	} else {
		fraction = vec3_length(vec3_subtract(
				   position, mover->start_transform.position)) /
			   distance;
		if (fraction < 0.0f) { fraction = 0.0f; }
		if (fraction > 1.0f) { fraction = 1.0f; }
	}
	result = interpolate_transform(&mover->start_transform,
				       &mover->end_transform, fraction);
	result.position = position;
	return result;
}

static bool try_move(mover_t *mover, const transform_t *next_transform) {
	collision_trace_t rider_trace = {0};
	collision_trace_t trace;
	vec3_t displacement;
	entity_id_t rider_blocker_id;
	bool initially_hit;

	displacement = vec3_subtract(next_transform->position,
				     mover->entity->transform.position);
	if (!mover->sweep_collider) {
		if (mover->move_riders &&
		    !move_contacting_riders(mover, displacement,
					    &rider_blocker_id)) {
			handle_blocked(mover, rider_blocker_id);
			return false;
		}
		handle_unblocked(mover);
		mover->entity->transform = *next_transform;
		return true;
	}
	if (mover->block_policy == MOVER_BLOCK_IGNORE) {
		if (mover->move_riders) {
			(void)move_contacting_riders(mover, displacement, NULL);
		}
		handle_unblocked(mover);
		mover->entity->transform = *next_transform;
		return true;
	}

	initially_hit = trace_movement(mover, next_transform->position, &trace);
	if (initially_hit && !trace_hit_rider(mover, &trace)) {
		mover->entity->transform =
			transform_at_position(mover, trace.position);
		handle_blocked(mover, trace.entity_id);
		if (mover->block_policy == MOVER_BLOCK_REVERSE) {
			reverse_after_block(mover);
		}
		return false;
	}

	if (mover->move_riders &&
	    !move_contacting_riders(mover, displacement, &rider_blocker_id)) {
		rider_trace.entity_id = rider_blocker_id;
		handle_blocked(mover, rider_trace.entity_id);
		if (mover->block_policy == MOVER_BLOCK_REVERSE) {
			reverse_after_block(mover);
		}
		return false;
	}

	if (trace_movement(mover, next_transform->position, &trace) &&
	    !(trace_hit_rider(mover, &trace) && trace.fraction >= 0.999999f)) {
		if (mover->move_riders) {
			(void)move_contacting_riders(
				mover, vec3_scale(displacement, -1.0f), NULL);
		}
		mover->entity->transform =
			transform_at_position(mover, trace.position);
		handle_blocked(mover, trace.entity_id);
		if (mover->block_policy == MOVER_BLOCK_REVERSE) {
			reverse_after_block(mover);
		}
		return false;
	}

	handle_unblocked(mover);
	mover->entity->transform = *next_transform;
	return true;
}

static bool trace_movement(const mover_t *mover,
			   const vec3_t end,
			   collision_trace_t *trace) {
	const float skin = 0.001f;
	aabb_t local_bounds;
	aabb_t world_bounds;
	collision_filter_t filter;
	vec3_t center;
	vec3_t half_extents;

	if (mover == NULL || mover->entity == NULL || trace == NULL ||
	    mover->entity->world == NULL || !mover->entity->has_collider ||
	    vec3_length(vec3_subtract(
		    end, mover->entity->transform.position)) <= 0.000001f ||
	    !collider_get_aabb(&mover->entity->collider,
			       mover->entity->transform.position,
			       &world_bounds)) {
		return false;
	}

	center = vec3_subtract(aabb_get_center(world_bounds),
			       mover->entity->transform.position);
	half_extents = aabb_get_half_extents(world_bounds);
	half_extents.x = fmaxf(0.0f, half_extents.x - skin);
	half_extents.y = fmaxf(0.0f, half_extents.y - skin);
	half_extents.z = fmaxf(0.0f, half_extents.z - skin);
	local_bounds = aabb_create(center, half_extents);
	filter.layer = mover->entity->collision_layer;
	filter.mask = mover->entity->collision_mask;
	filter.ignored_entity_id = mover->entity->id;
	return collision_world_trace_aabb_filtered(
		world_get_const_collision_world(mover->entity->world),
		local_bounds, mover->entity->transform.position, end, filter,
		trace);
}

static bool trace_hit_rider(const mover_t *mover,
			    const collision_trace_t *trace) {
	entity_t *entity;
	player_t *player;

	if (mover == NULL || mover->entity == NULL || trace == NULL ||
	    trace->entity_id == 0 || mover->entity->world == NULL) {
		return false;
	}
	entity = world_find_entity(mover->entity->world, trace->entity_id);
	player = player_from_entity(entity);
	return player_is_grounded_on(player, mover->entity->id);
}

static bool move_contacting_riders(mover_t *mover,
				   const vec3_t displacement,
				   entity_id_t *blocker_id) {
	entity_t *entity;
	player_t **riders;
	player_t *player;
	size_t entity_count;
	size_t index;
	size_t moved;
	size_t rider_count;

	if (blocker_id != NULL) { *blocker_id = 0; }
	if (mover == NULL || mover->entity == NULL ||
	    mover->entity->world == NULL ||
	    vec3_length(displacement) <= 0.000001f) {
		return true;
	}

	entity_count = world_get_entity_count(mover->entity->world);
	riders = malloc(entity_count * sizeof(*riders));
	if (riders == NULL && entity_count > 0) { return false; }

	rider_count = 0;
	for (index = 0; index < entity_count; index++) {
		entity = world_get_entity(mover->entity->world, index);
		player = player_from_entity(entity);
		if (!player_is_grounded_on(player, mover->entity->id)) {
			continue;
		}
		riders[rider_count++] = player;
	}

	for (index = 0; index < rider_count; index++) {
		if (player_can_move_with_platform(
			    riders[index], mover->entity->id, displacement)) {
			continue;
		}
		if (blocker_id != NULL) {
			*blocker_id = player_get_entity(riders[index])->id;
		}
		free(riders);
		return false;
	}

	for (moved = 0; moved < rider_count; moved++) {
		if (player_move_with_platform(riders[moved], mover->entity->id,
					      displacement)) {
			continue;
		}
		while (moved > 0) {
			moved--;
			(void)player_move_with_platform(
				riders[moved], mover->entity->id,
				vec3_scale(displacement, -1.0f));
		}
		free(riders);
		return false;
	}

	free(riders);
	return true;
}

static void handle_blocked(mover_t *mover, const entity_id_t blocker_id) {
	entity_t *blocker;

	if (mover == NULL || mover->entity == NULL ||
	    (mover->blocked && mover->blocker_id == blocker_id)) {
		return;
	}
	blocker = mover->entity->world == NULL
			  ? NULL
			  : world_find_entity(mover->entity->world, blocker_id);
	mover->blocked = true;
	mover->blocker_id = blocker_id;
	mover->current_speed = 0.0f;
	fire_output(mover, mover->outputs.on_blocked, blocker);
}

static void handle_unblocked(mover_t *mover) {
	entity_t *blocker;

	if (mover == NULL || mover->entity == NULL || !mover->blocked) {
		return;
	}
	blocker = mover->entity->world == NULL
			  ? NULL
			  : world_find_entity(mover->entity->world,
					      mover->blocker_id);
	mover->blocked = false;
	mover->blocker_id = 0;
	fire_output(mover, mover->outputs.on_unblocked, blocker);
}

static void reverse_after_block(mover_t *mover) {
	entity_t *activator;
	mover_state_t blocked_state;

	blocked_state = mover->state;
	activator = mover_get_activator(mover);
	if (blocked_state == MOVER_MOVING_TO_END) {
		(void)mover_move_to_start(mover, activator);
	} else if (blocked_state == MOVER_MOVING_TO_START) {
		(void)mover_move_to_end(mover, activator);
	}
}

static void
fire_output(mover_t *mover, const char *output_name, entity_t *activator) {
	if (mover == NULL || mover->entity == NULL ||
	    mover->entity->world == NULL || output_name == NULL ||
	    output_name[0] == '\0') {
		return;
	}
	(void)world_fire_output(mover->entity->world, mover->entity,
				output_name, activator);
}
