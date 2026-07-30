/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "weapon/hitscan_weapon.h"
#include "entity/entity.h"
#include "entity/world.h"
#include <math.h>
#include <stddef.h>

static bool hitscan_weapon_fire_internal(hitscan_weapon_t *weapon,
					 world_t *world,
					 entity_t *owner,
					 vec3_t origin,
					 vec3_t direction,
					 float action_duration,
					 collision_trace_t *trace);
static bool transfer_reload_ammo(hitscan_weapon_t *weapon);
static bool can_fire(const hitscan_weapon_t *weapon,
		     const world_t *world,
		     vec3_t direction);
static float calculate_inaccuracy(const hitscan_weapon_t *weapon,
				  const hitscan_accuracy_context_t *context);
static vec3_t
apply_spread(hitscan_weapon_t *weapon, vec3_t direction, float inaccuracy);
static float next_random(hitscan_weapon_t *weapon);

bool hitscan_weapon_initialize(hitscan_weapon_t *weapon,
			       const hitscan_weapon_config_t *config) {
	if (weapon == NULL || config == NULL || !isfinite(config->damage) ||
	    config->damage <= 0.0f || !isfinite(config->range) ||
	    config->range <= 0.0f || !isfinite(config->fire_interval) ||
	    config->fire_interval < 0.0f || config->magazine_size <= 0 ||
	    config->reserve_ammo < 0) {
		return false;
	}
	*weapon = (hitscan_weapon_t){0};
	weapon->damage = config->damage;
	weapon->range = config->range;
	weapon->fire_interval = config->fire_interval;
	weapon->cooldown = 0.0f;
	weapon->reload_time_remaining = 0.0f;
	weapon->reloading = false;
	weapon->magazine_size = config->magazine_size;
	weapon->ammo = config->magazine_size;
	weapon->reserve_ammo = config->reserve_ammo;
	return true;
}

bool hitscan_weapon_set_accuracy(hitscan_weapon_t *weapon,
				 const hitscan_accuracy_config_t *config) {
	if (weapon == NULL || config == NULL ||
	    !isfinite(config->standing_inaccuracy) ||
	    config->standing_inaccuracy < 0.0f ||
	    !isfinite(config->moving_inaccuracy) ||
	    config->moving_inaccuracy < 0.0f ||
	    !isfinite(config->airborne_inaccuracy) ||
	    config->airborne_inaccuracy < 0.0f ||
	    !isfinite(config->crouched_multiplier) ||
	    config->crouched_multiplier <= 0.0f ||
	    config->crouched_multiplier > 1.0f ||
	    !isfinite(config->firing_penalty_per_shot) ||
	    config->firing_penalty_per_shot < 0.0f ||
	    !isfinite(config->maximum_firing_penalty) ||
	    config->maximum_firing_penalty < 0.0f ||
	    !isfinite(config->penalty_recovery_delay) ||
	    config->penalty_recovery_delay < 0.0f ||
	    !isfinite(config->penalty_recovery_rate) ||
	    config->penalty_recovery_rate <= 0.0f ||
	    !isfinite(config->reference_move_speed) ||
	    config->reference_move_speed <= 0.0f) {
		return false;
	}
	weapon->accuracy = *config;
	weapon->firing_penalty = 0.0f;
	weapon->time_since_shot = config->penalty_recovery_delay;
	weapon->random_state = config->random_seed == 0 ? UINT32_C(0x6d2b79f5)
							: config->random_seed;
	weapon->uses_accuracy = true;
	return true;
}

void hitscan_weapon_update(hitscan_weapon_t *weapon, const float delta_time) {
	if (weapon == NULL || !isfinite(delta_time) || delta_time <= 0.0f) {
		return;
	}
	weapon->cooldown = fmaxf(0.0f, weapon->cooldown - delta_time);
	if (weapon->uses_accuracy) {
		weapon->time_since_shot += delta_time;
		if (weapon->time_since_shot >=
		    weapon->accuracy.penalty_recovery_delay) {
			weapon->firing_penalty = fmaxf(
				0.0f,
				weapon->firing_penalty -
					weapon->accuracy.penalty_recovery_rate *
						delta_time);
		}
	}
	if (!weapon->reloading) { return; }
	weapon->reload_time_remaining =
		fmaxf(0.0f, weapon->reload_time_remaining - delta_time);
	if (weapon->reload_time_remaining <= 0.0f) {
		(void)transfer_reload_ammo(weapon);
		weapon->reloading = false;
	}
}

bool hitscan_weapon_fire_accurate_timed(
	hitscan_weapon_t *weapon,
	world_t *world,
	entity_t *owner,
	const vec3_t origin,
	const vec3_t direction,
	const hitscan_accuracy_context_t *accuracy,
	const float action_duration,
	hitscan_shot_result_t *result) {
	hitscan_shot_result_t shot = {0};

	if (weapon == NULL || accuracy == NULL || !weapon->uses_accuracy ||
	    !isfinite(accuracy->movement_speed) ||
	    accuracy->movement_speed < 0.0f || !isfinite(action_duration) ||
	    action_duration < 0.0f || !can_fire(weapon, world, direction)) {
		return false;
	}
	shot.inaccuracy = calculate_inaccuracy(weapon, accuracy);
	shot.direction = apply_spread(weapon, direction, shot.inaccuracy);
	if (!hitscan_weapon_fire_internal(weapon, world, owner, origin,
					  shot.direction, action_duration,
					  &shot.trace)) {
		return false;
	}
	weapon->firing_penalty =
		fminf(weapon->accuracy.maximum_firing_penalty,
		      weapon->firing_penalty +
			      weapon->accuracy.firing_penalty_per_shot);
	weapon->time_since_shot = 0.0f;
	if (result != NULL) { *result = shot; }
	return true;
}

float hitscan_weapon_get_firing_penalty(const hitscan_weapon_t *weapon) {
	return weapon == NULL ? 0.0f : weapon->firing_penalty;
}

static float calculate_inaccuracy(const hitscan_weapon_t *weapon,
				  const hitscan_accuracy_context_t *context) {
	float inaccuracy;
	float movement_fraction;

	if (!context->grounded) {
		inaccuracy = weapon->accuracy.airborne_inaccuracy;
	} else {
		movement_fraction =
			fminf(context->movement_speed /
				      weapon->accuracy.reference_move_speed,
			      1.0f);
		inaccuracy = weapon->accuracy.standing_inaccuracy +
			     (weapon->accuracy.moving_inaccuracy -
			      weapon->accuracy.standing_inaccuracy) *
				     movement_fraction;
		if (context->crouched) {
			inaccuracy *= weapon->accuracy.crouched_multiplier;
		}
	}
	return inaccuracy + weapon->firing_penalty;
}

static vec3_t apply_spread(hitscan_weapon_t *weapon,
			   const vec3_t direction,
			   const float inaccuracy) {
	const float two_pi = 6.28318530717958647692f;
	vec3_t forward;
	vec3_t reference_up;
	vec3_t right;
	vec3_t up;
	float radius;
	float angle;
	float offset_x;
	float offset_y;

	if (vec3_length(direction) <= 0.000001f || inaccuracy <= 0.0f) {
		return vec3_normalize(direction);
	}
	forward = vec3_normalize(direction);
	reference_up = fabsf(forward.y) > 0.99f ? vec3_create(0.0f, 0.0f, 1.0f)
						: vec3_create(0.0f, 1.0f, 0.0f);
	right = vec3_normalize(vec3_cross(forward, reference_up));
	up = vec3_normalize(vec3_cross(right, forward));
	radius = sqrtf(next_random(weapon)) * inaccuracy;
	angle = next_random(weapon) * two_pi;
	offset_x = cosf(angle) * radius;
	offset_y = sinf(angle) * radius;
	return vec3_normalize(
		vec3_add(forward, vec3_add(vec3_scale(right, offset_x),
					   vec3_scale(up, offset_y))));
}

static float next_random(hitscan_weapon_t *weapon) {
	weapon->random_state =
		weapon->random_state * UINT32_C(1664525) + UINT32_C(1013904223);
	return (float)(weapon->random_state >> 8) * (1.0f / 16777216.0f);
}

bool hitscan_weapon_fire(hitscan_weapon_t *weapon,
			 world_t *world,
			 entity_t *owner,
			 const vec3_t origin,
			 const vec3_t direction,
			 collision_trace_t *trace) {
	return hitscan_weapon_fire_internal(
		weapon, world, owner, origin, direction,
		weapon == NULL ? 0.0f : weapon->fire_interval, trace);
}

bool hitscan_weapon_fire_timed(hitscan_weapon_t *weapon,
			       world_t *world,
			       entity_t *owner,
			       const vec3_t origin,
			       const vec3_t direction,
			       const float action_duration,
			       collision_trace_t *trace) {
	if (!isfinite(action_duration) || action_duration < 0.0f) {
		return false;
	}
	return hitscan_weapon_fire_internal(weapon, world, owner, origin,
					    direction, action_duration, trace);
}

static bool hitscan_weapon_fire_internal(hitscan_weapon_t *weapon,
					 world_t *world,
					 entity_t *owner,
					 const vec3_t origin,
					 const vec3_t direction,
					 const float action_duration,
					 collision_trace_t *trace) {
	collision_filter_t filter;
	collision_trace_t result = {0};
	damage_info_t damage = {0};
	entity_t *target;
	vec3_t normalized_direction;
	vec3_t end;

	if (!can_fire(weapon, world, direction)) { return false;
	}
	normalized_direction = vec3_normalize(direction);
	end = vec3_add(origin, vec3_scale(normalized_direction, weapon->range));
	filter.layer =
		owner == NULL ? COLLISION_LAYER_PLAYER : owner->collision_layer;
	filter.mask = COLLISION_LAYER_WORLD_STATIC | COLLISION_LAYER_DYNAMIC |
		      COLLISION_LAYER_PLAYER;
	filter.ignored_entity_id = owner == NULL ? 0 : owner->id;

	weapon->ammo--;
	weapon->cooldown = fmaxf(weapon->fire_interval, action_duration);
	if (collision_world_trace_ray_filtered(
		    world_get_const_collision_world(world), origin, end, filter,
		    &result)) {
		target = world_find_entity(world, result.entity_id);
		damage.amount = weapon->damage;
		damage.type = DAMAGE_TYPE_BULLET;
		damage.attacker = owner;
		damage.inflictor = owner;
		damage.position = result.position;
		damage.direction = normalized_direction;
		(void)entity_take_damage(target, &damage);
	}
	if (trace != NULL) { *trace = result; }
	return true;
}

static bool can_fire(const hitscan_weapon_t *weapon,
		     const world_t *world,
		     const vec3_t direction) {
	return weapon != NULL && world != NULL && weapon->ammo > 0 &&
	       !weapon->reloading && weapon->cooldown <= 0.0f &&
	       vec3_length(direction) > 0.000001f;
}

bool hitscan_weapon_reload(hitscan_weapon_t *weapon) {
	if (weapon == NULL) { return false; }
	weapon->reloading = false;
	weapon->reload_time_remaining = 0.0f;
	return transfer_reload_ammo(weapon);
}

bool hitscan_weapon_start_reload(hitscan_weapon_t *weapon,
				 const float duration) {
	if (weapon == NULL || !isfinite(duration) || duration <= 0.0f ||
	    hitscan_weapon_is_busy(weapon) ||
	    weapon->ammo >= weapon->magazine_size ||
	    weapon->reserve_ammo <= 0) {
		return false;
	}
	weapon->reloading = true;
	weapon->reload_time_remaining = duration;
	return true;
}

bool hitscan_weapon_is_busy(const hitscan_weapon_t *weapon) {
	return weapon != NULL && (weapon->reloading || weapon->cooldown > 0.0f);
}

bool hitscan_weapon_is_reloading(const hitscan_weapon_t *weapon) {
	return weapon != NULL && weapon->reloading;
}

float hitscan_weapon_get_action_time_remaining(const hitscan_weapon_t *weapon) {
	if (weapon == NULL) { return 0.0f; }
	return weapon->reloading ? weapon->reload_time_remaining
				 : weapon->cooldown;
}

static bool transfer_reload_ammo(hitscan_weapon_t *weapon) {
	int available;
	int needed;

	if (weapon->ammo >= weapon->magazine_size ||
	    weapon->reserve_ammo <= 0) {
		return false;
	}
	needed = weapon->magazine_size - weapon->ammo;
	available =
		needed < weapon->reserve_ammo ? needed : weapon->reserve_ammo;
	weapon->ammo += available;
	weapon->reserve_ammo -= available;
	return available > 0;
}

int hitscan_weapon_get_ammo(const hitscan_weapon_t *weapon) {
	return weapon == NULL ? 0 : weapon->ammo;
}

int hitscan_weapon_get_reserve_ammo(const hitscan_weapon_t *weapon) {
	return weapon == NULL ? 0 : weapon->reserve_ammo;
}
