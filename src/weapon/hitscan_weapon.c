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

bool hitscan_weapon_initialize(hitscan_weapon_t *weapon,
			       const hitscan_weapon_config_t *config) {
	if (weapon == NULL || config == NULL || !isfinite(config->damage) ||
	    config->damage <= 0.0f || !isfinite(config->range) ||
	    config->range <= 0.0f || !isfinite(config->fire_interval) ||
	    config->fire_interval < 0.0f || config->magazine_size <= 0 ||
	    config->reserve_ammo < 0) {
		return false;
	}
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

void hitscan_weapon_update(hitscan_weapon_t *weapon, const float delta_time) {
	if (weapon == NULL || !isfinite(delta_time) || delta_time <= 0.0f) {
		return;
	}
	weapon->cooldown = fmaxf(0.0f, weapon->cooldown - delta_time);
	if (!weapon->reloading) { return; }
	weapon->reload_time_remaining =
		fmaxf(0.0f, weapon->reload_time_remaining - delta_time);
	if (weapon->reload_time_remaining <= 0.0f) {
		(void)transfer_reload_ammo(weapon);
		weapon->reloading = false;
	}
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

	if (weapon == NULL || world == NULL || weapon->ammo <= 0 ||
	    weapon->reloading || weapon->cooldown > 0.0f || vec3_length(direction) <= 0.000001f) {
		return false;
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
