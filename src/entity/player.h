/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_ENTITY_PLAYER_H
#define VOLUME_ENTITY_PLAYER_H

#include "entity/damage.h"
#include "entity/health.h"
#include "physics/character_controller.h"

typedef struct player player_t;

/// プレイヤーエンティティクラスを登録する。
///
/// ### Returns
/// - `true`: 登録に成功した。
/// - `false`: 登録に失敗した。
bool player_register(void);
/// エンティティをプレイヤーとして取得する。
///
/// ### Args
/// - `entity_t *entity`: 変換するエンティティ。
///
/// ### Returns
/// - `player_t *`: 対応するプレイヤー。型が異なる場合は`NULL`。
player_t *player_from_entity(entity_t *entity);
/// 読み取り専用エンティティをプレイヤーとして取得する。
///
/// ### Args
/// - `const entity_t *entity`: 変換するエンティティ。
///
/// ### Returns
/// - `const player_t *`: 対応するプレイヤー。型が異なる場合は`NULL`。
const player_t *player_from_const_entity(const entity_t *entity);
/// プレイヤーが所有する基底エンティティを取得する。
///
/// ### Args
/// - `player_t *player`: 対象のプレイヤー。
///
/// ### Returns
/// - `entity_t *`: 基底エンティティ。引数が`NULL`の場合は`NULL`。
entity_t *player_get_entity(player_t *player);
/// プレイヤーが所有する読み取り専用の基底エンティティを取得する。
///
/// ### Args
/// - `const player_t *player`: 対象のプレイヤー。
///
/// ### Returns
/// - `const entity_t *`: 基底エンティティ。引数が`NULL`の場合は`NULL`。
const entity_t *player_get_const_entity(const player_t *player);
/// プレイヤーの足元のワールド座標を取得する。
///
/// ### Args
/// - `const player_t *player`: 対象のプレイヤー。
///
/// ### Returns
/// - `vec3_t`: プレイヤーの位置。
vec3_t player_get_position(const player_t *player);
/// プレイヤーの現在速度を取得する。
///
/// ### Args
/// - `const player_t *player`: 対象のプレイヤー。
///
/// ### Returns
/// - `vec3_t`: プレイヤーの速度。
vec3_t player_get_velocity(const player_t *player);
/// カメラに使用するプレイヤーの視点座標を取得する。
///
/// ### Args
/// - `const player_t *player`: 対象のプレイヤー。
///
/// ### Returns
/// - `vec3_t`: ワールド空間の視点座標。
vec3_t player_get_view_position(const player_t *player);
/// プレイヤーがしゃがんでいるか調べる。
///
/// ### Args
/// - `const player_t *player`: 対象のプレイヤー。
///
/// ### Returns
/// - `true`: しゃがんでいる。
/// - `false`: 立っている、または引数が`NULL`。
bool player_is_crouched(const player_t *player);
/// プレイヤーが急斜面をサーフしているか調べる。
///
/// ### Args
/// - `const player_t *player`: 対象のプレイヤー。
///
/// ### Returns
/// - `true`: サーフ中である。
/// - `false`: サーフ中ではない。
bool player_is_surfing(const player_t *player);
/// プレイヤーが梯子を使用しているか調べる。
///
/// ### Args
/// - `const player_t *player`: 対象のプレイヤー。
///
/// ### Returns
/// - `true`: 梯子移動中である。
/// - `false`: 梯子移動中ではない。
bool player_is_on_ladder(const player_t *player);
/// プレイヤーが接地しているエンティティIDを取得する。
///
/// ### Args
/// - `const player_t *player`: 対象のプレイヤー。
///
/// ### Returns
/// - `entity_id_t`: 接地先のID。非接地時は`0`。
entity_id_t player_get_ground_entity_id(const player_t *player);
/// 指定したエンティティ上にプレイヤーが接地しているか調べる。
///
/// ### Args
/// - `const player_t *player`: 対象のプレイヤー。
/// - `entity_id_t entity_id`: 接地先として確認するID。
///
/// ### Returns
/// - `true`: 指定エンティティ上に接地している。
/// - `false`: 接地していない。
bool player_is_grounded_on(const player_t *player, entity_id_t entity_id);
/// 動く足場による移動が衝突せず可能か調べる。
///
/// ### Args
/// - `const player_t *player`: 対象のプレイヤー。
/// - `entity_id_t platform_id`: 移動元となる足場のID。
/// - `vec3_t displacement`: 適用予定の移動量。
///
/// ### Returns
/// - `true`: 移動可能である。
/// - `false`: 壁や天井などに遮られる。
bool player_can_move_with_platform(const player_t *player,
				   entity_id_t platform_id,
				   vec3_t displacement);
/// 動く足場の移動量をプレイヤーへ適用する。
///
/// ### Args
/// - `player_t *player`: 対象のプレイヤー。
/// - `entity_id_t platform_id`: 移動元となる足場のID。
/// - `vec3_t displacement`: 適用する移動量。
///
/// ### Returns
/// - `true`: 移動を適用した。
/// - `false`: 衝突により移動できなかった。
bool player_move_with_platform(player_t *player,
			       entity_id_t platform_id,
			       vec3_t displacement);
/// 入力に従ってプレイヤーを1物理tick移動する。
///
/// ### Args
/// - `player_t *player`: 移動するプレイヤー。
/// - `const character_move_input_t *input`: このtickの移動入力。
/// - `float delta_time`: 物理tickの経過秒数。
void player_move(player_t *player,
		 const character_move_input_t *input,
		 float delta_time);
/// プレイヤーの直近の衝突デバッグ情報を取得する。
///
/// ### Args
/// - `const player_t *player`: 対象のプレイヤー。
/// - `character_debug_state_t *state`: デバッグ情報の格納先。
///
/// ### Returns
/// - `true`: 衝突デバッグ情報を取得した。
/// - `false`: 情報がない、または引数が不正だった。
bool player_get_collision_debug_state(const player_t *player,
				      character_debug_state_t *state);
/// プレイヤーの現在ヘルスを取得する。
///
/// ### Args
/// - `const player_t *player`: 対象のプレイヤー。
///
/// ### Returns
/// - `float`: 現在ヘルス。引数が`NULL`の場合は`0`。
float player_get_health(const player_t *player);
/// プレイヤーの最大ヘルスを取得する。
///
/// ### Args
/// - `const player_t *player`: 対象のプレイヤー。
///
/// ### Returns
/// - `float`: 最大ヘルス。引数が`NULL`の場合は`0`。
float player_get_max_health(const player_t *player);
/// プレイヤーが生存しているか調べる。
///
/// ### Args
/// - `const player_t *player`: 対象のプレイヤー。
///
/// ### Returns
/// - `true`: 生存している。
/// - `false`: 死亡している、または引数が`NULL`。
bool player_is_alive(const player_t *player);
/// プレイヤーを指定位置で最大ヘルスの状態へ戻す。
///
/// 速度、接地、しゃがみ、梯子状態を初期化し、死亡時に外したColliderを
/// 衝突ワールドへ再登録する。
///
/// ### Args
/// - `player_t *player`: 復帰させるプレイヤー。
/// - `vec3_t position`: 復帰先の足元座標。
///
/// ### Returns
/// - `true`: 復帰に成功した。
/// - `false`: 引数または座標が不正だった。
bool player_respawn(player_t *player, vec3_t position);

#endif
