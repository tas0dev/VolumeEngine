/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_COLLISION_COLLISION_WORLD_H
#define VOLUME_COLLISION_COLLISION_WORLD_H

#include "collision/aabb.h"
#include "collision/collider.h"
#include "core/types.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define COLLISION_RESULT_MAX_CONTACTS 16

typedef uint32_t collision_layer_t;

#define COLLISION_LAYER_NONE ((collision_layer_t)0)
#define COLLISION_LAYER_WORLD_STATIC ((collision_layer_t)1u << 0)
#define COLLISION_LAYER_DYNAMIC ((collision_layer_t)1u << 1)
#define COLLISION_LAYER_PLAYER ((collision_layer_t)1u << 2)
#define COLLISION_LAYER_TRIGGER ((collision_layer_t)1u << 3)
#define COLLISION_LAYER_ALL UINT32_MAX

typedef struct collision_filter {
	collision_layer_t layer;
	collision_layer_t mask;
	entity_id_t ignored_entity_id;
} collision_filter_t;

typedef struct collision_contact {
	vec3_t normal;
	float depth;
	entity_id_t entity_id;
} collision_contact_t;

typedef enum collision_side {
	COLLISION_SIDE_NONE = 0,
	COLLISION_SIDE_NEGATIVE_X = 1u << 0,
	COLLISION_SIDE_POSITIVE_X = 1u << 1,
	COLLISION_SIDE_NEGATIVE_Y = 1u << 2,
	COLLISION_SIDE_POSITIVE_Y = 1u << 3,
	COLLISION_SIDE_NEGATIVE_Z = 1u << 4,
	COLLISION_SIDE_POSITIVE_Z = 1u << 5,
} collision_side_t;

typedef struct collision_result {
	vec3_t correction;
	unsigned int sides;
	size_t contact_count;
	collision_contact_t contacts[COLLISION_RESULT_MAX_CONTACTS];
} collision_result_t;

typedef struct collision_trace {
	bool hit;
	bool started_inside;
	float fraction;
	vec3_t position;
	vec3_t normal;
	entity_id_t entity_id;
} collision_trace_t;

typedef struct collision_world collision_world_t;

/// コライダーを管理する衝突ワールドを作成する。
///
/// ### Returns
/// - `collision_world_t *`: 作成した衝突ワールド。失敗時は`NULL`。
collision_world_t *collision_world_create(void);
/// 衝突ワールドを破棄する。
///
/// ### Args
/// - `collision_world_t *world`: 破棄する衝突ワールド。
void collision_world_destroy(collision_world_t *world);
/// 全レイヤーと衝突するコライダーを追加する。
///
/// ### Args
/// - `collision_world_t *world`: 追加先の衝突ワールド。
/// - `entity_id_t entity_id`: コライダー所有者のID。
/// - `collider_t collider`: 追加するコライダー。
/// - `vec3_t position`: 所有者のワールド座標。
///
/// ### Returns
/// - `true`: 追加に成功した。
/// - `false`: ID重複、形状なし、またはメモリ不足で失敗した。
bool collision_world_add_collider(collision_world_t *world,
				  entity_id_t entity_id,
				  collider_t collider,
				  vec3_t position);
/// レイヤーとマスクを指定してコライダーを追加する。
///
/// ### Args
/// - `collision_world_t *world`: 追加先の衝突ワールド。
/// - `entity_id_t entity_id`: コライダー所有者のID。
/// - `collider_t collider`: 追加するコライダー。
/// - `vec3_t position`: 所有者のワールド座標。
/// - `collision_layer_t layer`: 所有者の衝突レイヤー。
/// - `collision_layer_t mask`: 衝突対象レイヤーのマスク。
///
/// ### Returns
/// - `true`: 追加に成功した。
/// - `false`: ID重複、形状なし、またはメモリ不足で失敗した。
bool collision_world_add_collider_filtered(collision_world_t *world,
					   entity_id_t entity_id,
					   collider_t collider,
					   vec3_t position,
					   collision_layer_t layer,
					   collision_layer_t mask);
/// 登録済みコライダーの形状と位置を更新する。
///
/// ### Args
/// - `collision_world_t *world`: 対象の衝突ワールド。
/// - `entity_id_t entity_id`: 更新する所有者のID。
/// - `collider_t collider`: 新しいコライダー。
/// - `vec3_t position`: 新しいワールド座標。
///
/// ### Returns
/// - `true`: 更新または新規追加に成功した。
/// - `false`: 引数または形状が不正だった。
bool collision_world_update_collider(collision_world_t *world,
				     entity_id_t entity_id,
				     collider_t collider,
				     vec3_t position);
/// 登録済みコライダーの形状、位置、フィルターを更新する。
///
/// ### Args
/// - `collision_world_t *world`: 対象の衝突ワールド。
/// - `entity_id_t entity_id`: 更新する所有者のID。
/// - `collider_t collider`: 新しいコライダー。
/// - `vec3_t position`: 新しいワールド座標。
/// - `collision_layer_t layer`: 新しい衝突レイヤー。
/// - `collision_layer_t mask`: 新しい衝突マスク。
///
/// ### Returns
/// - `true`: 更新または新規追加に成功した。
/// - `false`: 引数または形状が不正だった。
bool collision_world_update_collider_filtered(collision_world_t *world,
					      entity_id_t entity_id,
					      collider_t collider,
					      vec3_t position,
					      collision_layer_t layer,
					      collision_layer_t mask);
/// 指定IDのコライダーを削除する。
///
/// ### Args
/// - `collision_world_t *world`: 対象の衝突ワールド。
/// - `entity_id_t entity_id`: 削除する所有者のID。
///
/// ### Returns
/// - `true`: コライダーを削除した。
/// - `false`: 指定IDが存在しなかった。
bool collision_world_remove(collision_world_t *world, entity_id_t entity_id);
/// 登録されているコライダー数を取得する。
///
/// ### Args
/// - `const collision_world_t *world`: 対象の衝突ワールド。
///
/// ### Returns
/// - `size_t`: 登録コライダー数。
size_t collision_world_get_count(const collision_world_t *world);
/// AABBの重なりを解消する最小移動を計算して位置へ適用する。
///
/// ### Args
/// - `const collision_world_t *world`: 衝突判定に使用するワールド。
/// - `aabb_t local_bounds`: 対象のローカル境界ボックス。
/// - `vec3_t *position`: 入出力となるワールド座標。
/// - `collision_result_t *result`: 接触情報の格納先。不要な場合は`NULL`。
///
/// ### Returns
/// - `true`: 1つ以上の重なりを解消した。
/// - `false`: 重なりがない、または引数が不正だった。
bool collision_world_resolve_aabb(const collision_world_t *world,
				  aabb_t local_bounds,
				  vec3_t *position,
				  collision_result_t *result);
/// 指定エンティティを無視してAABBの重なりを解消する。
///
/// ### Args
/// - `const collision_world_t *world`: 衝突判定に使用するワールド。
/// - `aabb_t local_bounds`: 対象のローカル境界ボックス。
/// - `vec3_t *position`: 入出力となるワールド座標。
/// - `entity_id_t ignored_entity_id`: 判定から除外する所有者ID。
/// - `collision_result_t *result`: 接触情報の格納先。不要な場合は`NULL`。
///
/// ### Returns
/// - `true`: 1つ以上の重なりを解消した。
/// - `false`: 重なりがない、または引数が不正だった。
bool collision_world_resolve_aabb_ignoring(const collision_world_t *world,
					   aabb_t local_bounds,
					   vec3_t *position,
					   entity_id_t ignored_entity_id,
					   collision_result_t *result);
/// 衝突フィルターを適用してAABBの重なりを解消する。
///
/// ### Args
/// - `const collision_world_t *world`: 衝突判定に使用するワールド。
/// - `aabb_t local_bounds`: 対象のローカル境界ボックス。
/// - `vec3_t *position`: 入出力となるワールド座標。
/// - `collision_filter_t filter`: 適用する衝突フィルター。
/// - `collision_result_t *result`: 接触情報の格納先。不要な場合は`NULL`。
///
/// ### Returns
/// - `true`: 1つ以上の重なりを解消した。
/// - `false`: 重なりがない、または引数が不正だった。
bool collision_world_resolve_aabb_filtered(const collision_world_t *world,
					   aabb_t local_bounds,
					   vec3_t *position,
					   collision_filter_t filter,
					   collision_result_t *result);
/// AABBを開始位置から終了位置まで掃引して最初の衝突を取得する。
///
/// ### Args
/// - `const collision_world_t *world`: 衝突判定に使用するワールド。
/// - `aabb_t local_bounds`: 掃引するローカル境界ボックス。
/// - `vec3_t start`: 掃引開始位置。
/// - `vec3_t end`: 掃引終了位置。
/// - `collision_trace_t *trace`: 掃引結果の格納先。
///
/// ### Returns
/// - `true`: 進路上で衝突した。
/// - `false`: 衝突しなかった、または引数が不正だった。
bool collision_world_trace_aabb(const collision_world_t *world,
				aabb_t local_bounds,
				vec3_t start,
				vec3_t end,
				collision_trace_t *trace);
/// 指定エンティティを無視してAABBを掃引する。
///
/// ### Args
/// - `const collision_world_t *world`: 衝突判定に使用するワールド。
/// - `aabb_t local_bounds`: 掃引するローカル境界ボックス。
/// - `vec3_t start`: 掃引開始位置。
/// - `vec3_t end`: 掃引終了位置。
/// - `entity_id_t ignored_entity_id`: 判定から除外する所有者ID。
/// - `collision_trace_t *trace`: 掃引結果の格納先。
///
/// ### Returns
/// - `true`: 進路上で衝突した。
/// - `false`: 衝突しなかった、または引数が不正だった。
bool collision_world_trace_aabb_ignoring(const collision_world_t *world,
					 aabb_t local_bounds,
					 vec3_t start,
					 vec3_t end,
					 entity_id_t ignored_entity_id,
					 collision_trace_t *trace);
/// 衝突フィルターを適用してAABBを掃引する。
///
/// ### Args
/// - `const collision_world_t *world`: 衝突判定に使用するワールド。
/// - `aabb_t local_bounds`: 掃引するローカル境界ボックス。
/// - `vec3_t start`: 掃引開始位置。
/// - `vec3_t end`: 掃引終了位置。
/// - `collision_filter_t filter`: 適用する衝突フィルター。
/// - `collision_trace_t *trace`: 掃引結果の格納先。
///
/// ### Returns
/// - `true`: 進路上で衝突した。
/// - `false`: 衝突しなかった、または引数が不正だった。
bool collision_world_trace_aabb_filtered(const collision_world_t *world,
					 aabb_t local_bounds,
					 vec3_t start,
					 vec3_t end,
					 collision_filter_t filter,
					 collision_trace_t *trace);
/// フィルターに加えてもう1つのエンティティを無視してAABBを掃引する。
///
/// ### Args
/// - `const collision_world_t *world`: 衝突判定に使用するワールド。
/// - `aabb_t local_bounds`: 掃引するローカル境界ボックス。
/// - `vec3_t start`: 掃引開始位置。
/// - `vec3_t end`: 掃引終了位置。
/// - `collision_filter_t filter`: 適用する衝突フィルター。
/// - `entity_id_t additional_ignored_entity_id`: 追加で除外する所有者ID。
/// - `collision_trace_t *trace`: 掃引結果の格納先。
///
/// ### Returns
/// - `true`: 進路上で衝突した。
/// - `false`: 衝突しなかった、または引数が不正だった。
bool collision_world_trace_aabb_filtered_ignoring(
	const collision_world_t *world,
	aabb_t local_bounds,
	vec3_t start,
	vec3_t end,
	collision_filter_t filter,
	entity_id_t additional_ignored_entity_id,
	collision_trace_t *trace);
/// 衝突フィルターを適用して線分レイを掃引する。
///
/// ### Args
/// - `const collision_world_t *world`: 衝突判定に使用するワールド。
/// - `vec3_t start`: レイの開始位置。
/// - `vec3_t end`: レイの終了位置。
/// - `collision_filter_t filter`: 適用する衝突フィルター。
/// - `collision_trace_t *trace`: 掃引結果の格納先。
///
/// ### Returns
/// - `true`: 線分上で衝突した。
/// - `false`: 衝突しなかった、または引数が不正だった。
bool collision_world_trace_ray_filtered(const collision_world_t *world,
					vec3_t start,
					vec3_t end,
					collision_filter_t filter,
					collision_trace_t *trace);
/// 指定範囲と交差するコライダーの所有者IDを列挙する。
///
/// ### Args
/// - `const collision_world_t *world`: 検索する衝突ワールド。
/// - `aabb_t bounds`: ワールド空間の検索範囲。
/// - `collision_filter_t filter`: 適用する衝突フィルター。
/// - `entity_id_t *entity_ids`: IDの格納先。件数確認だけなら`NULL`。
/// - `size_t capacity`: 格納先配列の要素数。
///
/// ### Returns
/// - `size_t`: 範囲と交差したコライダーの総数。
size_t collision_world_query_aabb(const collision_world_t *world,
				  aabb_t bounds,
				  collision_filter_t filter,
				  entity_id_t *entity_ids,
				  size_t capacity);
/// 指定位置に登録されたコライダー情報を取得する。
///
/// ### Args
/// - `const collision_world_t *world`: 対象の衝突ワールド。
/// - `size_t index`: 取得するコライダーの位置。
/// - `entity_id_t *entity_id`: 所有者IDの格納先。不要な場合は`NULL`。
/// - `collider_t *collider`: コライダーの格納先。不要な場合は`NULL`。
/// - `vec3_t *position`: 所有者のワールド座標の格納先。不要な場合は`NULL`。
/// - `collision_layer_t *layer`: 衝突レイヤーの格納先。不要な場合は`NULL`。
///
/// ### Returns
/// - `true`: コライダー情報を取得した。
/// - `false`: ワールドまたは位置が不正だった。
bool collision_world_get_collider(const collision_world_t *world,
				  size_t index,
				  entity_id_t *entity_id,
				  collider_t *collider,
				  vec3_t *position,
				  collision_layer_t *layer);

#endif
