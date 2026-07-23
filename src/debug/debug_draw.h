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

/// 衝突ワールド内のコライダー形状を描画キューへ追加する。
///
/// 描画するには、呼び出し後に`renderer_flush_debug_lines()`を実行する。
///
/// ### Args
/// - `renderer_t *renderer`: 描画に使用するレンダラー。
/// - `const collision_world_t *collision_world`: 描画対象の衝突ワールド。
void debug_draw_colliders(renderer_t *renderer,
			  const collision_world_t *collision_world);

#endif