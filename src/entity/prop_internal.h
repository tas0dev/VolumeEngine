/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_ENTITY_PROP_INTERNAL_H
#define VOLUME_ENTITY_PROP_INTERNAL_H

#include "entity/prop_static.h"

/// スポーン情報から内部用の静的propを作成する。
///
/// ### Args
/// - `entity_id_t id`: 割り当てる一意なID。
/// - `const entity_spawn_context_t *context`: スポーン情報。
///
/// ### Returns
/// - `entity_t *`: 作成した基底エンティティ。失敗時は`NULL`。
entity_t *prop_internal_create_static(entity_id_t id,
				      const entity_spawn_context_t *context);
/// propの共通フィールドをスポーン情報から初期化する。
///
/// ### Args
/// - `prop_t *prop`: 初期化するprop。
/// - `entity_id_t id`: 割り当てる一意なID。
/// - `const entity_class_t *class`: 使用するエンティティクラス。
/// - `const entity_spawn_context_t *context`: スポーン情報。
///
/// ### Returns
/// - `true`: 初期化に成功した。
/// - `false`: 必要なアセットまたはプロパティを取得できなかった。
bool prop_internal_initialize(prop_t *prop,
			      entity_id_t id,
			      const entity_class_t *class,
			      const entity_spawn_context_t *context);
/// propの影を描画する。
///
/// ### Args
/// - `entity_t *entity`: 描画するpropエンティティ。
/// - `renderer_t *renderer`: 使用するレンダラー。
void prop_internal_draw_shadow(entity_t *entity, renderer_t *renderer);
/// prop本体を描画する。
///
/// ### Args
/// - `entity_t *entity`: 描画するpropエンティティ。
/// - `renderer_t *renderer`: 使用するレンダラー。
/// - `const render_view_t *view`: 描画時のビュー情報。
void prop_internal_draw(entity_t *entity,
			renderer_t *renderer,
			const render_view_t *view);
/// propエンティティと所有リソースを破棄する。
///
/// ### Args
/// - `entity_t *entity`: 破棄するpropエンティティ。
void prop_internal_destroy(entity_t *entity);

#endif
