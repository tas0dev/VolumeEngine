/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_ENTITY_DAMAGE_H
#define VOLUME_ENTITY_DAMAGE_H

#include "core/types.h"
#include "math/vec3.h"
#include <stdbool.h>

typedef enum damage_type {
	DAMAGE_TYPE_GENERIC = 0,
	DAMAGE_TYPE_CRUSH = 1 << 0,
	DAMAGE_TYPE_BULLET = 1 << 1,
	DAMAGE_TYPE_SLASH = 1 << 2,
	DAMAGE_TYPE_BURN = 1 << 3,
	DAMAGE_TYPE_FREEZE = 1 << 4,
	DAMAGE_TYPE_FALL = 1 << 5,
	DAMAGE_TYPE_BLAST = 1 << 6,
	DAMAGE_TYPE_CLUB = 1 << 7,
	DAMAGE_TYPE_SHOCK = 1 << 8,
	DAMAGE_TYPE_SONIC = 1 << 9,
	DAMAGE_TYPE_ENERGY_BEAM = 1 << 10,
	DAMAGE_TYPE_DROWN = 1 << 14,
	DAMAGE_TYPE_PARALYZE = 1 << 15,
	DAMAGE_TYPE_NERVE_GAS = 1 << 16,
	DAMAGE_TYPE_POISON = 1 << 17,
	DAMAGE_TYPE_RADIATION = 1 << 18,
	DAMAGE_TYPE_DROWN_RECOVER = 1 << 19,
	DAMAGE_TYPE_CHEMICAL = 1 << 20,
	DAMAGE_TYPE_SLOW_BURN = 1 << 21,
	DAMAGE_TYPE_SLOW_FREEZE = 1 << 22,
} damage_type_t;

typedef struct damage_info {
	float amount;
	damage_type_t type;
	entity_t *attacker;
	entity_t *inflictor;
	vec3_t position;
	vec3_t direction;
} damage_info_t;

/// エンティティへダメージを与える。
///
/// ### Args
/// - `entity_t *entity`: ダメージを受けるエンティティ。
/// - `const damage_info_t *damage`: ダメージ量と発生元の情報。
///
/// ### Returns
/// - `true`: エンティティがダメージを受理した。
/// - `false`: ダメージを受けられない、または情報が不正だった。
bool entity_take_damage(entity_t *entity, const damage_info_t *damage);

#endif
