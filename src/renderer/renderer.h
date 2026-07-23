/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_RENDERER_RENDERER_H
#define VOLUME_RENDERER_RENDERER_H

#include "core/types.h"
#include "math/mat4.h"
#include "renderer/material.h"
#include "renderer/mesh.h"
#include "renderer/view.h"
#include <stddef.h>

typedef struct renderer_color {
	float r;
	float g;
	float b;
	float a;
} renderer_color_t;

typedef struct renderer_frame_stats {
	size_t mesh_draw_calls;
	size_t shadow_draw_calls;
} renderer_frame_stats_t;

/// プラットフォームの描画コンテキストを使用してレンダラーを作成する。
///
/// ### Args
/// - `platform_t *platform`:
/// ウィンドウと描画コンテキストを所有するプラットフォーム。
///
/// ### Returns
/// - `renderer_t *`: 作成したレンダラー。失敗時は`NULL`。
renderer_t *renderer_create(platform_t *platform);
/// レンダラーと所有するGPUリソースを破棄する。
///
/// ### Args
/// - `renderer_t *renderer`: 破棄するレンダラー。
void renderer_destroy(renderer_t *renderer);
/// フレームの最終合成を行う。
///
/// ### Args
/// - `const renderer_t *renderer`: 対象のレンダラー。
void renderer_end_frame(const renderer_t *renderer);
/// 現在の描画領域サイズを取得する。
///
/// ### Args
/// - `const renderer_t *renderer`: 対象のレンダラー。
/// - `int *width`: 幅の格納先。不要な場合は`NULL`。
/// - `int *height`: 高さの格納先。不要な場合は`NULL`。
void renderer_get_size(const renderer_t *renderer, int *width, int *height);
/// シャドウマップ描画パスを開始する。
///
/// ### Args
/// - `renderer_t *renderer`: 対象のレンダラー。
/// - `const mat4_t *light_view_projection`: 光源視点のビュー投影行列。
void renderer_begin_shadow_pass(renderer_t *renderer,
				const mat4_t *light_view_projection);
/// シャドウパスへメッシュを描画する。
///
/// ### Args
/// - `renderer_t *renderer`: 対象のレンダラー。
/// - `const mesh_t *mesh`: 描画するメッシュ。
/// - `const mat4_t *model`: モデル変換行列。
void renderer_draw_shadow_mesh(renderer_t *renderer,
			       const mesh_t *mesh,
			       const mat4_t *model);
/// シャドウマップ描画パスを終了する。
///
/// ### Args
/// - `renderer_t *renderer`: 対象のレンダラー。
void renderer_end_shadow_pass(renderer_t *renderer);
/// メイン描画パスへマテリアル付きメッシュを描画する。
///
/// ### Args
/// - `renderer_t *renderer`: 対象のレンダラー。
/// - `const mesh_t *mesh`: 描画するメッシュ。
/// - `const material_t *material`: 使用するマテリアル。
/// - `const mat4_t *model`: モデル変換行列。
/// - `const render_view_t *view`: カメラと光源のビュー情報。
void renderer_draw_mesh(renderer_t *renderer,
			const mesh_t *mesh,
			const material_t *material,
			const mat4_t *model,
			const render_view_t *view);
/// 新しい描画フレームを開始して描画先を初期化する。
///
/// ### Args
/// - `renderer_t *renderer`: 対象のレンダラー。
void renderer_begin_frame(renderer_t *renderer);
/// 最終合成後に描画するスクリーン座標の矩形を登録する。
void renderer_draw_rectangle(renderer_t *renderer,
			     float x,
			     float y,
			     float width,
			     float height,
			     renderer_color_t color);
/// 最終合成後に描画するASCIIテキストを登録する。
void renderer_draw_text(const renderer_t *renderer,
			float x,
			float y,
			float scale,
			renderer_color_t color,
			const char *text);
/// 現在のフレームの描画統計を取得する。
renderer_frame_stats_t renderer_get_frame_stats(const renderer_t *renderer);
/// ワールド空間上へデバッグ用の線分を描画する。
///
/// ### Args
/// - `renderer_t *renderer`: 対象のレンダラー。
/// - `vec3_t start`: 線分の開始座標。
/// - `vec3_t end`: 線分の終了座標。
/// - `renderer_color_t color`: 線分の色。
/// - `const render_view_t *view`: 描画に使用するビュー情報。
void renderer_draw_debug_line(renderer_t *renderer,
			      vec3_t start,
			      vec3_t end,
			      renderer_color_t color,
			      const render_view_t *view);

#endif
