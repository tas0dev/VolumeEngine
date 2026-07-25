/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_GAME_HITSCAN_WEAPON_H
#define VOLUME_GAME_HITSCAN_WEAPON_H

#include "collision/collision_world.h"
#include "entity/damage.h"

typedef struct hitscan_weapon_config {
	float damage;
	float range;
	float fire_interval;
	int magazine_size;
	int reserve_ammo;
} hitscan_weapon_config_t;

typedef struct hitscan_weapon {
	float damage;
	float range;
	float fire_interval;
	float cooldown;
	int magazine_size;
	int ammo;
	int reserve_ammo;
} hitscan_weapon_t;

/// 既定のピストル設定を作成する。
///
/// ### Returns
/// - `hitscan_weapon_config_t`: 既定の武器設定。
hitscan_weapon_config_t hitscan_weapon_config_create(void);

/// hitscan武器を初期化する。
///
/// ### Args
/// - `hitscan_weapon_t *weapon`: 初期化する武器。
/// - `const hitscan_weapon_config_t *config`: 武器設定。
///
/// ### Returns
/// - `true`: 初期化に成功した。
/// - `false`: 設定または引数が不正だった。
bool hitscan_weapon_initialize(hitscan_weapon_t *weapon,
			       const hitscan_weapon_config_t *config);

/// 武器の発射クールダウンを更新する。
///
/// ### Args
/// - `hitscan_weapon_t *weapon`: 更新する武器。
/// - `float delta_time`: 経過秒数。
void hitscan_weapon_update(hitscan_weapon_t *weapon, float delta_time);

/// ワールドへrayを飛ばして武器を発射する。
///
/// ### Args
/// - `hitscan_weapon_t *weapon`: 発射する武器。
/// - `world_t *world`: 射撃対象のワールド。
/// - `entity_t *owner`: 射撃者。存在しない場合は`NULL`。
/// - `vec3_t origin`: 射撃開始位置。
/// - `vec3_t direction`: 射撃方向。
/// - `collision_trace_t *trace`: 命中結果の格納先。不要な場合は`NULL`。
///
/// ### Returns
/// - `true`: 弾を消費して発射した。
/// - `false`: 弾切れ、クールダウン中、または引数が不正だった。
bool hitscan_weapon_fire(hitscan_weapon_t *weapon,
			 world_t *world,
			 entity_t *owner,
			 vec3_t origin,
			 vec3_t direction,
			 collision_trace_t *trace);

/// マガジンへ予備弾薬を装填する。
///
/// ### Args
/// - `hitscan_weapon_t *weapon`: リロードする武器。
///
/// ### Returns
/// - `true`: 1発以上装填した。
/// - `false`: マガジンが満杯または予備弾薬がない。
bool hitscan_weapon_reload(hitscan_weapon_t *weapon);

/// マガジン内の弾数を取得する。
///
/// ### Args
/// - `const hitscan_weapon_t *weapon`: 対象の武器。
///
/// ### Returns
/// - `int`: 現在の装弾数。
int hitscan_weapon_get_ammo(const hitscan_weapon_t *weapon);

/// 予備弾薬数を取得する。
///
/// ### Args
/// - `const hitscan_weapon_t *weapon`: 対象の武器。
///
/// ### Returns
/// - `int`: 予備弾薬数。
int hitscan_weapon_get_reserve_ammo(const hitscan_weapon_t *weapon);

#endif
