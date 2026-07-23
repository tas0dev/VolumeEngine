/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_PHYSICS_CHARACTER_CONTROLLER_H
#define VOLUME_PHYSICS_CHARACTER_CONTROLLER_H
#include "collision/aabb.h"
#include "collision/collision_world.h"
#include "math/vec3.h"
#include <stdbool.h>

#define CHARACTER_DEBUG_MAXIMUM_CONTACTS 16

typedef struct character_debug_contact {
	vec3_t position;
	vec3_t normal;
	entity_id_t entity_id;
} character_debug_contact_t;

typedef struct character_debug_state {
	character_debug_contact_t contacts[CHARACTER_DEBUG_MAXIMUM_CONTACTS];
	size_t contact_count;
	vec3_t correction;
	bool valid;
} character_debug_state_t;

typedef struct character_move_input {
	vec3_t wish_direction;
	vec3_t look_direction;
	vec3_t ladder_normal;
	float wish_speed;
	bool jump;
	bool crouch;
	bool ladder;
} character_move_input_t;

typedef struct character_controller {
	aabb_t bounds;
	aabb_t standing_bounds;
	aabb_t crouched_bounds;
	vec3_t position;
	vec3_t velocity;
	vec3_t ground_normal;
	float maximum_speed;
	float ground_acceleration;
	float air_acceleration;
	float air_speed_cap;
	float friction;
	float stop_speed;
	float gravity;
	float jump_speed;
	float ladder_speed;
	float ground_stick_speed;
	float step_height;
	float standing_view_height;
	float crouched_view_height;
	float view_height;
	float crouch_transition_speed;
	float crouched_speed_multiplier;
	float minimum_ground_normal_y;
	character_debug_state_t debug_state;
	entity_id_t ground_entity_id;
	vec3_t surf_normal;
	bool grounded;
	bool surfing;
	bool on_ladder;
	bool crouched;
} character_controller_t;

/// 指定位置と寸法からキャラクターコントローラーを作成する。
///
/// ### Args
/// - `vec3_t position`: 足元を基準とした初期ワールド座標。
/// - `float radius`: 水平方向の半径。
/// - `float height`: 立ち状態の高さ。
///
/// ### Returns
/// - `character_controller_t`: 初期化済みのコントローラー。
character_controller_t
character_controller_create(vec3_t position, float radius, float height);
/// 接地中のコントローラーをジャンプさせる。
///
/// ### Args
/// - `character_controller_t *controller`: 対象のコントローラー。
///
/// ### Returns
/// - `true`: ジャンプを開始した。
/// - `false`: 非接地または無効な引数により開始できなかった。
bool character_controller_jump(character_controller_t *controller);
/// 全衝突レイヤーを対象にコントローラーを移動する。
///
/// ### Args
/// - `character_controller_t *controller`: 移動するコントローラー。
/// - `const collision_world_t *world`: 衝突判定に使用するワールド。
/// - `const character_move_input_t *input`: このtickの入力。
/// - `float delta_time`: 物理tickの経過秒数。
void character_controller_move(character_controller_t *controller,
			       const collision_world_t *world,
			       const character_move_input_t *input,
			       float delta_time);
/// 指定エンティティを無視してコントローラーを移動する。
///
/// ### Args
/// - `character_controller_t *controller`: 移動するコントローラー。
/// - `const collision_world_t *world`: 衝突判定に使用するワールド。
/// - `entity_id_t ignored_entity_id`: 衝突判定から除外するID。
/// - `const character_move_input_t *input`: このtickの入力。
/// - `float delta_time`: 物理tickの経過秒数。
void character_controller_move_ignoring(character_controller_t *controller,
					const collision_world_t *world,
					entity_id_t ignored_entity_id,
					const character_move_input_t *input,
					float delta_time);
/// 衝突フィルターを指定してコントローラーを移動する。
///
/// ### Args
/// - `character_controller_t *controller`: 移動するコントローラー。
/// - `const collision_world_t *world`: 衝突判定に使用するワールド。
/// - `collision_filter_t filter`: 適用する衝突フィルター。
/// - `const character_move_input_t *input`: このtickの入力。
/// - `float delta_time`: 物理tickの経過秒数。
void character_controller_move_filtered(character_controller_t *controller,
					const collision_world_t *world,
					collision_filter_t filter,
					const character_move_input_t *input,
					float delta_time);
/// キャラクターコントローラーの直近の衝突デバッグ情報を取得する。
///
/// ### Args
/// - `const character_controller_t *controller`: 対象のコントローラー。
/// - `character_debug_state_t *state`: デバッグ情報の格納先。
///
/// ### Returns
/// - `true`: 衝突デバッグ情報を取得した。
/// - `false`: 情報がない、または引数が不正だった。
bool character_controller_get_debug_state(
	const character_controller_t *controller,
	character_debug_state_t *state);

#endif
