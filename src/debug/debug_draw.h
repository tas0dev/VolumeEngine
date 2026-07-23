/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_DEBUG_DEBUG_DRAW_H
#define VOLUME_DEBUG_DEBUG_DRAW_H

#include "collision/collision_world.h"
#include "renderer/renderer.h"
#include "renderer/view.h"

/// 衝突ワールド内のコライダー境界を描画する。
///
/// ### Args
/// - `renderer_t *renderer`: 描画に使用するレンダラー。
/// - `const collision_world_t *collision_world`: 描画対象の衝突ワールド。
/// - `const render_view_t *view`: 描画に使用するビュー情報。
void debug_draw_colliders(renderer_t *renderer,
			  const collision_world_t *collision_world,
			  const render_view_t *view);

#endif