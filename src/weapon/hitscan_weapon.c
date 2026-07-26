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
	weapon->magazine_size = config->magazine_size;
	weapon->ammo = config->magazine_size;
	weapon->reserve_ammo = config->reserve_ammo;
	return true;
}

void hitscan_weapon_update(hitscan_weapon_t *weapon, const float delta_time) {
	if (weapon == NULL || delta_time <= 0.0f) { return; }
	weapon->cooldown = fmaxf(0.0f, weapon->cooldown - delta_time);
}

bool hitscan_weapon_fire(hitscan_weapon_t *weapon,
			 world_t *world,
			 entity_t *owner,
			 const vec3_t origin,
			 const vec3_t direction,
			 collision_trace_t *trace) {
	collision_filter_t filter;
	collision_trace_t result = {0};
	damage_info_t damage = {0};
	entity_t *target;
	vec3_t normalized_direction;
	vec3_t end;

	if (weapon == NULL || world == NULL || weapon->ammo <= 0 ||
	    weapon->cooldown > 0.0f || vec3_length(direction) <= 0.000001f) {
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
	weapon->cooldown = weapon->fire_interval;
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
	int available;
	int needed;

	if (weapon == NULL || weapon->ammo >= weapon->magazine_size ||
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
