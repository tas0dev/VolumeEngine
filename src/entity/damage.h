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
	DAMAGE_TYPE_GENERIC,
	DAMAGE_TYPE_BULLET,
	DAMAGE_TYPE_CRUSH,
	DAMAGE_TYPE_FALL,
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
