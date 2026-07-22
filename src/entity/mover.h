/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_ENTITY_MOVER_H
#define VOLUME_ENTITY_MOVER_H

#include "entity/entity.h"

typedef enum mover_state {
	MOVER_AT_START,
	MOVER_MOVING_TO_END,
	MOVER_AT_END,
	MOVER_MOVING_TO_START,
} mover_state_t;

typedef enum mover_block_policy {
	MOVER_BLOCK_STOP,
	MOVER_BLOCK_REVERSE,
	MOVER_BLOCK_IGNORE,
} mover_block_policy_t;

typedef struct mover_outputs {
	const char *on_move_to_end;
	const char *on_reached_end;
	const char *on_move_to_start;
	const char *on_reached_start;
	const char *on_blocked;
	const char *on_unblocked;
} mover_outputs_t;

typedef struct mover_config {
	transform_t start_transform;
	transform_t end_transform;
	float speed;
	float acceleration;
	float deceleration;
	float wait;
	mover_block_policy_t block_policy;
	bool starts_at_end;
	bool sweep_collider;
	bool move_riders;
	mover_outputs_t outputs;
} mover_config_t;

typedef struct mover {
	entity_t *entity;
	transform_t start_transform;
	transform_t end_transform;
	float speed;
	float acceleration;
	float deceleration;
	float current_speed;
	float wait;
	float wait_remaining;
	entity_id_t activator_id;
	entity_id_t blocker_id;
	mover_state_t state;
	mover_block_policy_t block_policy;
	bool sweep_collider;
	bool move_riders;
	bool blocked;
	mover_outputs_t outputs;
} mover_t;

/// 移動エンティティの共通状態を初期化する。
///
/// ### Args
/// - `mover_t *mover`: 初期化するMover。
/// - `entity_t *entity`: Moverが制御するエンティティ。
/// - `const mover_config_t *config`: 移動範囲と挙動の設定。
///
/// ### Returns
/// - `true`: 初期化に成功した。
/// - `false`: 引数または設定が不正だった。
bool mover_initialize(mover_t *mover,
		      entity_t *entity,
		      const mover_config_t *config);

/// 終了Transformへの移動を開始する。
///
/// ### Args
/// - `mover_t *mover`: 対象のMover。
/// - `entity_t *activator`:
/// 移動を開始したエンティティ。存在しない場合は`NULL`。
///
/// ### Returns
/// - `true`: 新しい移動を開始した。
/// - `false`: 既に終了側または終了側へ移動中だった。
bool mover_move_to_end(mover_t *mover, entity_t *activator);

/// 開始Transformへの移動を開始する。
///
/// ### Args
/// - `mover_t *mover`: 対象のMover。
/// - `entity_t *activator`:
/// 移動を開始したエンティティ。存在しない場合は`NULL`。
///
/// ### Returns
/// - `true`: 新しい移動を開始した。
/// - `false`: 既に開始側または開始側へ移動中だった。
bool mover_move_to_start(mover_t *mover, entity_t *activator);

/// 加減速、Sweep、rider追従、通知を含む移動処理を更新する。
///
/// ### Args
/// - `mover_t *mover`: 更新するMover。
/// - `float delta_time`: 経過秒数。
void mover_update(mover_t *mover, float delta_time);

/// Moverを指定距離だけ目標位置へ近づける。
///
/// ### Args
/// - `vec3_t *position`: 更新する現在位置。
/// - `vec3_t target`: 目標位置。
/// - `float distance`: この更新で進める距離。
///
/// ### Returns
/// - `true`: 目標位置へ到達した。
/// - `false`: 目標位置まで距離が残っている。
bool mover_move_towards(vec3_t *position, vec3_t target, float distance);

/// Moverの現在状態を取得する。
///
/// ### Args
/// - `const mover_t *mover`: 対象のMover。
///
/// ### Returns
/// - `mover_state_t`: 現在の移動状態。
mover_state_t mover_get_state(const mover_t *mover);

/// Moverがブロックされているか調べる。
///
/// ### Args
/// - `const mover_t *mover`: 対象のMover。
///
/// ### Returns
/// - `true`: 接触によってブロックされている。
/// - `false`: ブロックされていない。
bool mover_is_blocked(const mover_t *mover);

/// 移動を開始したActivatorを取得する。
///
/// ### Args
/// - `const mover_t *mover`: 対象のMover。
///
/// ### Returns
/// - `entity_t *`: Activator。存在しない場合は`NULL`。
entity_t *mover_get_activator(const mover_t *mover);

/// 文字列からブロック時の方針を読み取る。
///
/// ### Args
/// - `const char *text`: `stop`、`reverse`、`ignore`のいずれか。
/// - `mover_block_policy_t *policy`: 読み取った方針の格納先。
///
/// ### Returns
/// - `true`: 読み取りに成功した。
/// - `false`: 文字列または格納先が不正だった。
bool mover_parse_block_policy(const char *text, mover_block_policy_t *policy);

#endif
