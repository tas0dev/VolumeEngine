/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_ENTITY_ENTITY_H
#define VOLUME_ENTITY_ENTITY_H

#include "collision/collision_world.h"
#include "core/types.h"
#include "entity/io.h"
#include "entity/properties.h"
#include "renderer/view.h"
#include "scene/transform.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct entity_class entity_class_t;
struct asset_manager;

typedef struct entity_spawn_context {
	const entity_properties_t *properties;
	const entity_property_source_t *source;
	struct asset_manager *assets;
	char *error;
	size_t error_size;
} entity_spawn_context_t;

struct entity_class {
	const char *classname;
	/// スポーン情報からクラス固有エンティティを作成する。
	///
	/// ### Args
	/// - `entity_id_t id`: 割り当てる一意なID。
	/// - `const entity_spawn_context_t *context`: スポーン情報。
	///
	/// ### Returns
	/// - `entity_t *`: 作成したエンティティ。失敗時は`NULL`。
	entity_t *(*create)(entity_id_t id,
			    const entity_spawn_context_t *context);
	/// 全マップエンティティの生成と参照読込後に有効化処理を行う。
	///
	/// ### Args
	/// - `entity_t *entity`: 有効化するエンティティ。
	void (*activate)(entity_t *entity);
	/// クラス固有状態を1tick更新する。
	///
	/// ### Args
	/// - `entity_t *entity`: 更新するエンティティ。
	/// - `float delta_time`: 経過秒数。
	void (*update)(entity_t *entity, float delta_time);
	/// クラス固有の影を描画する。
	///
	/// ### Args
	/// - `entity_t *entity`: 描画するエンティティ。
	/// - `renderer_t *renderer`: 使用するレンダラー。
	void (*draw_shadow)(entity_t *entity, renderer_t *renderer);
	/// クラス固有の本体描画を行う。
	///
	/// ### Args
	/// - `entity_t *entity`: 描画するエンティティ。
	/// - `renderer_t *renderer`: 使用するレンダラー。
	/// - `const render_view_t *view`: 描画時のビュー情報。
	void (*draw)(entity_t *entity,
		     renderer_t *renderer,
		     const render_view_t *view);
	/// クラス固有の名前付き入力を処理する。
	///
	/// ### Args
	/// - `entity_t *entity`: 入力を受け取るエンティティ。
	/// - `const char *input_name`: 入力名。
	/// - `const entity_input_context_t *context`: 呼び出しコンテキスト。
	///
	/// ### Returns
	/// - `true`: 入力を受理した。
	/// - `false`: 入力が未対応または不正だった。
	bool (*accept_input)(entity_t *entity,
			     const char *input_name,
			     const entity_input_context_t *context);
	/// クラス固有エンティティと所有リソースを破棄する。
	///
	/// ### Args
	/// - `entity_t *entity`: 破棄するエンティティ。
	void (*destroy)(entity_t *entity);
};

struct entity {
	entity_id_t id;
	const entity_class_t *class;
	world_t *world;
	char *targetname;
	transform_t transform;
	vec3_t linear_velocity;
	bool active;
	bool activated;
	bool has_collider;
	bool collider_follows_transform;
	bool pending_destroy;
	collider_t collider;
	collision_layer_t collision_layer;
	collision_layer_t collision_mask;
	entity_output_connection_t *outputs;
	size_t output_count;
	size_t output_capacity;
};

/// 既に確保されたエンティティの共通フィールドを初期化する。
///
/// ### Args
/// - `entity_t *entity`: 初期化するエンティティ。
/// - `entity_id_t id`: 割り当てる一意なID。
/// - `const entity_class_t *class`: エンティティクラス定義。
void entity_initialize(entity_t *entity,
		       entity_id_t id,
		       const entity_class_t *class);
/// 登録済みクラス名からエンティティを作成する。
///
/// ### Args
/// - `const char *classname`: 作成するクラス名。
/// - `entity_id_t id`: 割り当てる一意なID。
/// - `const entity_spawn_context_t *context`: スポーン情報。
///
/// ### Returns
/// - `entity_t *`: 作成したエンティティ。失敗時は`NULL`。
entity_t *entity_create(const char *classname,
			entity_id_t id,
			const entity_spawn_context_t *context);
/// エンティティのクラス固有Activate処理を一度だけ呼び出す。
///
/// ### Args
/// - `entity_t *entity`: 有効化するエンティティ。
void entity_activate(entity_t *entity);
/// エンティティがActivate済みか調べる。
///
/// ### Args
/// - `const entity_t *entity`: 対象のエンティティ。
///
/// ### Returns
/// - `true`: Activate処理が完了している。
/// - `false`: 未実行または引数が`NULL`。
bool entity_is_activated(const entity_t *entity);
/// エンティティのクラス名を取得する。
///
/// ### Args
/// - `const entity_t *entity`: 対象のエンティティ。
///
/// ### Returns
/// - `const char *`: クラス名。取得できない場合は`NULL`。
const char *entity_get_classname(const entity_t *entity);
/// エンティティを1tick更新する。
///
/// ### Args
/// - `entity_t *entity`: 更新するエンティティ。
/// - `float delta_time`: 経過秒数。
void entity_update(entity_t *entity, float delta_time);
/// エンティティの影を描画する。
///
/// ### Args
/// - `entity_t *entity`: 描画するエンティティ。
/// - `renderer_t *renderer`: 使用するレンダラー。
void entity_draw_shadow(entity_t *entity, renderer_t *renderer);
/// エンティティ本体を描画する。
///
/// ### Args
/// - `entity_t *entity`: 描画するエンティティ。
/// - `renderer_t *renderer`: 使用するレンダラー。
/// - `const render_view_t *view`: 描画時のビュー情報。
void entity_draw(entity_t *entity,
		 renderer_t *renderer,
		 const render_view_t *view);
/// エンティティと所有リソースを破棄する。
///
/// ### Args
/// - `entity_t *entity`: 破棄するエンティティ。
void entity_destroy(entity_t *entity);
/// エンティティの有効状態を設定する。
///
/// ### Args
/// - `entity_t *entity`: 対象のエンティティ。
/// - `bool active`: 有効にする場合は`true`。
void entity_set_active(entity_t *entity, bool active);
/// エンティティが有効か調べる。
///
/// ### Args
/// - `const entity_t *entity`: 対象のエンティティ。
///
/// ### Returns
/// - `true`: 有効である。
/// - `false`: 無効または引数が`NULL`。
bool entity_is_active(const entity_t *entity);
/// エンティティクラスをグローバルレジストリへ登録する。
///
/// ### Args
/// - `const entity_class_t *class`: 登録するクラス定義。
///
/// ### Returns
/// - `true`: 登録に成功した。
/// - `false`: 重複またはメモリ不足により失敗した。
bool entity_register_class(const entity_class_t *class);
/// エンティティクラスレジストリを破棄する。
void entity_registry_shutdown(void);
/// エンティティのターゲット名を設定する。
///
/// ### Args
/// - `entity_t *entity`: 対象のエンティティ。
/// - `const char *targetname`: 複製して保持するターゲット名。
///
/// ### Returns
/// - `true`: 設定に成功した。
/// - `false`: メモリ不足または無効な引数により失敗した。
bool entity_set_targetname(entity_t *entity, const char *targetname);
/// エンティティのターゲット名を取得する。
///
/// ### Args
/// - `const entity_t *entity`: 対象のエンティティ。
///
/// ### Returns
/// - `const char *`: ターゲット名。未設定の場合は`NULL`。
const char *entity_get_targetname(const entity_t *entity);
/// エンティティへコライダーを設定する。
///
/// ### Args
/// - `entity_t *entity`: 対象のエンティティ。
/// - `collider_t collider`: 設定するコライダー。
void entity_set_collider(entity_t *entity, collider_t collider);
/// エンティティのコライダーを解除する。
///
/// ### Args
/// - `entity_t *entity`: 対象のエンティティ。
void entity_clear_collider(entity_t *entity);
/// エンティティのコライダーを取得する。
///
/// ### Args
/// - `const entity_t *entity`: 対象のエンティティ。
/// - `collider_t *collider`: 取得結果の格納先。不要な場合は`NULL`。
///
/// ### Returns
/// - `true`: コライダーを保持している。
/// - `false`: コライダーが設定されていない。
bool entity_get_collider(const entity_t *entity, collider_t *collider);
/// エンティティの衝突レイヤーとマスクを設定する。
///
/// ### Args
/// - `entity_t *entity`: 対象のエンティティ。
/// - `collision_layer_t layer`: 所属する衝突レイヤー。
/// - `collision_layer_t mask`: 衝突対象とするレイヤーマスク。
void entity_set_collision_filter(entity_t *entity,
				 collision_layer_t layer,
				 collision_layer_t mask);

#endif
