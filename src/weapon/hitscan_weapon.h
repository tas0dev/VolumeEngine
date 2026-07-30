/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_WEAPON_HITSCAN_WEAPON_H
#define VOLUME_WEAPON_HITSCAN_WEAPON_H

#include "collision/collision_world.h"
#include "entity/damage.h"
#include <stdbool.h>

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
	float reload_time_remaining;
	bool reloading;
	int magazine_size;
	int ammo;
	int reserve_ammo;
} hitscan_weapon_t;

/// 指定設定でhitscan武器の状態を初期化する。
///
/// 武器固有の既定値は持たず、ゲームが全設定を指定する。
///
/// ### Args
/// - `hitscan_weapon_t *weapon`: 初期化する武器。
/// - `const hitscan_weapon_config_t *config`: ゲームが指定する武器設定。
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

/// 指定したアニメーション時間だけ次の操作をロックして発射する。
///
/// 弾丸とダメージは発射開始時に処理される。
///
/// ### Args
/// - `hitscan_weapon_t *weapon`: 発射する武器。
/// - `world_t *world`: 射撃対象のワールド。
/// - `entity_t *owner`: 射撃者。存在しない場合は`NULL`。
/// - `vec3_t origin`: 射撃開始位置。
/// - `vec3_t direction`: 射撃方向。
/// - `float action_duration`: 発射アニメーション時間。単位は秒。
/// - `collision_trace_t *trace`: 命中結果の格納先。不要な場合は`NULL`。
///
/// ### Returns
/// - `true`: 弾を消費して発射した。
/// - `false`: 操作中、弾切れ、または引数が不正だった。
bool hitscan_weapon_fire_timed(hitscan_weapon_t *weapon,
			       world_t *world,
			       entity_t *owner,
			       vec3_t origin,
			       vec3_t direction,
			       float action_duration,
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

/// 時間のかかるリロードを開始する。
///
/// 弾薬は指定時間が経過した時点でマガジンへ移動する。
///
/// ### Args
/// - `hitscan_weapon_t *weapon`: リロードする武器。
/// - `float duration`: リロードアニメーション時間。単位は秒。
///
/// ### Returns
/// - `true`: リロードを開始した。
/// - `false`: 操作中、装填不要、または引数が不正だった。
bool hitscan_weapon_start_reload(hitscan_weapon_t *weapon, float duration);

/// 武器が発射またはリロード操作中かを取得する。
///
/// ### Args
/// - `const hitscan_weapon_t *weapon`: 対象の武器。
///
/// ### Returns
/// - `true`: 次の発射・リロードを受け付けない状態。
/// - `false`: 操作可能、または引数が不正。
bool hitscan_weapon_is_busy(const hitscan_weapon_t *weapon);

/// 現在リロード中かを取得する。
///
/// ### Args
/// - `const hitscan_weapon_t *weapon`: 対象の武器。
///
/// ### Returns
/// - `true`: 時間制リロード中。
/// - `false`: リロード中ではない、または引数が不正。
bool hitscan_weapon_is_reloading(const hitscan_weapon_t *weapon);

/// 現在の操作が完了するまでの時間を取得する。
///
/// ### Args
/// - `const hitscan_weapon_t *weapon`: 対象の武器。
///
/// ### Returns
/// - `float`: 残り秒数。操作中でない場合は0。
float hitscan_weapon_get_action_time_remaining(const hitscan_weapon_t *weapon);

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
