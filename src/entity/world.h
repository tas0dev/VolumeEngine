/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_ENTITY_WORLD_H
#define VOLUME_ENTITY_WORLD_H

#include "collision/collision_world.h"
#include "entity/entity.h"
#include "renderer/view.h"
#include <stdbool.h>
#include <stddef.h>

/// エンティティと衝突判定を管理するワールドを作成する。
///
/// ### Returns
/// - `world_t *`: 作成したワールド。失敗時は`NULL`。
world_t *world_create(void);
/// ワールドと所属する全エンティティを破棄する。
///
/// ### Args
/// - `world_t *world`: 破棄するワールド。
void world_destroy(world_t *world);
/// クラス名とスポーン情報からエンティティを生成してワールドへ追加する。
///
/// ### Args
/// - `world_t *world`: 追加先のワールド。
/// - `const char *classname`: 生成するエンティティのクラス名。
/// - `const entity_spawn_context_t *context`: スポーン情報。
///
/// ### Returns
/// - `entity_t *`: 生成したエンティティ。失敗時は`NULL`。
entity_t *world_spawn_entity(world_t *world,
			     const char *classname,
			     const entity_spawn_context_t *context);
/// 既存のエンティティをワールドへ追加する。
///
/// ### Args
/// - `world_t *world`: 追加先のワールド。
/// - `entity_t *entity`: 追加するエンティティ。
///
/// ### Returns
/// - `true`: 追加に成功した。
/// - `false`: ID重複、衝突登録失敗、または無効な引数で追加できなかった。
bool world_add_entity(world_t *world, entity_t *entity);
/// 指定IDのエンティティをワールドから削除して破棄する。
///
/// ### Args
/// - `world_t *world`: 対象のワールド。
/// - `entity_id_t id`: 削除するエンティティID。
///
/// ### Returns
/// - `true`: エンティティを削除した。
/// - `false`: 指定IDが存在しない、または引数が不正だった。
bool world_remove_entity(world_t *world, entity_id_t id);
/// IDからエンティティを検索する。
///
/// ### Args
/// - `world_t *world`: 検索するワールド。
/// - `entity_id_t id`: 検索するエンティティID。
///
/// ### Returns
/// - `entity_t *`: 見つかったエンティティ。存在しない場合は`NULL`。
entity_t *world_find_entity(world_t *world, entity_id_t id);
/// クラス名に一致する最初のエンティティを検索する。
///
/// ### Args
/// - `world_t *world`: 検索するワールド。
/// - `const char *classname`: 検索するクラス名。
///
/// ### Returns
/// - `entity_t *`: 見つかったエンティティ。存在しない場合は`NULL`。
entity_t *world_find_by_classname(world_t *world, const char *classname);
/// ワールドに所属するエンティティ数を取得する。
///
/// ### Args
/// - `const world_t *world`: 対象のワールド。
///
/// ### Returns
/// - `size_t`: エンティティ数。
size_t world_get_entity_count(const world_t *world);
/// 指定添字のエンティティを取得する。
///
/// ### Args
/// - `world_t *world`: 対象のワールド。
/// - `size_t index`: 取得するエンティティの添字。
///
/// ### Returns
/// - `entity_t *`: 対応するエンティティ。範囲外の場合は`NULL`。
entity_t *world_get_entity(world_t *world, size_t index);
/// ワールド内のイベントとエンティティを1tick更新する。
///
/// ### Args
/// - `world_t *world`: 更新するワールド。
/// - `float delta_time`: 経過秒数。
void world_update(world_t *world, float delta_time);
/// ワールド内の全エンティティの影を描画する。
///
/// ### Args
/// - `world_t *world`: 描画するワールド。
/// - `renderer_t *renderer`: 使用するレンダラー。
void world_draw_shadows(world_t *world, renderer_t *renderer);
/// ワールド内の全エンティティを描画する。
///
/// ### Args
/// - `world_t *world`: 描画するワールド。
/// - `renderer_t *renderer`: 使用するレンダラー。
/// - `const render_view_t *view`: 描画時のビュー情報。
void world_draw(world_t *world,
		renderer_t *renderer,
		const render_view_t *view);
/// ターゲット名に一致する最初のエンティティを検索する。
///
/// ### Args
/// - `world_t *world`: 検索するワールド。
/// - `const char *targetname`: 検索するターゲット名。
///
/// ### Returns
/// - `entity_t *`: 見つかったエンティティ。存在しない場合は`NULL`。
entity_t *world_find_by_targetname(world_t *world, const char *targetname);
/// ワールドが所有する衝突ワールドを取得する。
///
/// ### Args
/// - `world_t *world`: 対象のワールド。
///
/// ### Returns
/// - `collision_world_t *`: 衝突ワールド。引数が`NULL`の場合は`NULL`。
collision_world_t *world_get_collision_world(world_t *world);
/// ワールドが所有する読み取り専用の衝突ワールドを取得する。
///
/// ### Args
/// - `const world_t *world`: 対象のワールド。
///
/// ### Returns
/// - `const collision_world_t *`: 衝突ワールド。引数が`NULL`の場合は`NULL`。
const collision_world_t *world_get_const_collision_world(const world_t *world);
/// ターゲット名に一致するエンティティへ入力を送る。
///
/// ### Args
/// - `world_t *world`: 対象のワールド。
/// - `const char *target_name`: 入力先のターゲット名。
/// - `const char *input_name`: 送信する入力名。
/// - `const char *parameter`: 入力へ渡す文字列引数。
/// - `entity_t *activator`: 入力を発生させたエンティティ。
/// - `entity_t *caller`: 入力を送信したエンティティ。
///
/// ### Returns
/// - `size_t`: 入力を受理したエンティティ数。
size_t world_send_input(world_t *world,
			const char *target_name,
			const char *input_name,
			const char *parameter,
			entity_t *activator,
			entity_t *caller);
/// 特定のエンティティへ直接入力を送る。
///
/// ### Args
/// - `world_t *world`: 対象のワールド。
/// - `entity_t *target`: 入力先のエンティティ。
/// - `const char *input_name`: 送信する入力名。
/// - `const char *parameter`: 入力へ渡す文字列引数。
/// - `entity_t *activator`: 入力を発生させたエンティティ。
/// - `entity_t *caller`: 入力を送信したエンティティ。
///
/// ### Returns
/// - `true`: 入力が受理された。
/// - `false`: 入力が拒否された、または引数が不正だった。
bool world_send_input_to_entity(world_t *world,
				entity_t *target,
				const char *input_name,
				const char *parameter,
				entity_t *activator,
				entity_t *caller);
/// エンティティの名前付き出力を発火する。
///
/// ### Args
/// - `world_t *world`: 対象のワールド。
/// - `entity_t *caller`: 出力を所有するエンティティ。
/// - `const char *output_name`: 発火する出力名。
/// - `entity_t *activator`: 出力を発生させたエンティティ。
///
/// ### Returns
/// - `true`: 1件以上の入力イベントを登録した。
/// - `false`: 対応する接続がない、または登録に失敗した。
bool world_fire_output(world_t *world,
		       entity_t *caller,
		       const char *output_name,
		       entity_t *activator);
/// 遅延実行待ちのI/Oイベント数を取得する。
///
/// ### Args
/// - `const world_t *world`: 対象のワールド。
///
/// ### Returns
/// - `size_t`: 保留中のイベント数。
size_t world_get_pending_event_count(const world_t *world);

#endif
