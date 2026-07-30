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
#include "renderer/font.h"
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

typedef enum renderer_blend_mode {
	RENDERER_BLEND_ALPHA = 0,
	RENDERER_BLEND_ADDITIVE = 1,
} renderer_blend_mode_t;

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
/// FPS用ViewModel描画パスを開始する。
///
/// ワールドの深度を消去し、ViewModel同士だけで深度判定する。
/// ワールドと3Dデバッグ描画の後に呼び出す必要がある。
///
/// ### Args
/// - `renderer_t *renderer`: 対象のレンダラー。
/// - `float field_of_view`: ViewModelの垂直視野角。単位は度。
/// - `float near_plane`: 近クリップ面。
/// - `float far_plane`: 遠クリップ面。
///
/// ### Returns
/// - `true`: 描画パスを開始した。
/// - `false`: 設定または描画領域が不正だった。
bool renderer_begin_view_model_pass(renderer_t *renderer,
				    float field_of_view,
				    float near_plane,
				    float far_plane);
/// 現在のViewModelパスへカメラローカル座標のメッシュを描画する。
///
/// ### Args
/// - `renderer_t *renderer`: 対象のレンダラー。
/// - `const mesh_t *mesh`: 描画するメッシュ。
/// - `const material_t *material`: 使用するマテリアル。
/// - `const mat4_t *model`: カメラローカルのモデル変換。
/// - `const render_view_t *world_view`: 流用するワールド光源設定。
void renderer_draw_view_model_mesh(renderer_t *renderer,
				   const mesh_t *mesh,
				   const material_t *material,
				   const mat4_t *model,
				   const render_view_t *world_view);
/// 現在のViewModelパスへGPU Skinningされたメッシュを描画する。
///
/// ### Args
/// - `renderer_t *renderer`: 対象のレンダラー。
/// - `const mesh_t *mesh`: Animation Setを持つメッシュ。
/// - `const material_t *material`: 使用するマテリアル。
/// - `const mat4_t *model`: カメラローカルのモデル変換。
/// - `const render_view_t *world_view`: 流用するワールド光源設定。
/// - `const animator_t *animator`: Bone行列を提供するAnimator。
void renderer_draw_view_model_animated_mesh(renderer_t *renderer,
					    const mesh_t *mesh,
					    const material_t *material,
					    const mat4_t *model,
					    const render_view_t *world_view,
					    const animator_t *animator);
/// FPS用ViewModel描画パスを終了する。
///
/// ### Args
/// - `renderer_t *renderer`: 対象のレンダラー。
void renderer_end_view_model_pass(renderer_t *renderer);
/// ワールド空間にカメラ正対の円形Billboardを描画する。
///
/// ### Args
/// - `renderer_t *renderer`: 対象のレンダラー。
/// - `vec3_t position`: Billboard中心のワールド座標。
/// - `float size`: 一辺の半分の長さ。
/// - `renderer_color_t color`: 色と最大不透明度。
/// - `renderer_blend_mode_t blend_mode`: 合成方法。
/// - `const render_view_t *view`: 描画に使用するビュー。
void renderer_draw_world_billboard(renderer_t *renderer,
				   vec3_t position,
				   float size,
				   renderer_color_t color,
				   renderer_blend_mode_t blend_mode,
				   const render_view_t *view);
/// ワールド空間の指定平面に円形Spriteを描画する。
///
/// ### Args
/// - `renderer_t *renderer`: 対象のレンダラー。
/// - `vec3_t position`: Sprite中心のワールド座標。
/// - `vec3_t right`: 平面の右方向。
/// - `vec3_t up`: 平面の上方向。
/// - `float size`: 一辺の半分の長さ。
/// - `renderer_color_t color`: 色と最大不透明度。
/// - `renderer_blend_mode_t blend_mode`: 合成方法。
/// - `const render_view_t *view`: 描画に使用するビュー。
void renderer_draw_world_sprite(renderer_t *renderer,
				vec3_t position,
				vec3_t right,
				vec3_t up,
				float size,
				renderer_color_t color,
				renderer_blend_mode_t blend_mode,
				const render_view_t *view);
/// 2点間へカメラに正対する発光Beamを描画する。
///
/// ### Args
/// - `renderer_t *renderer`: 対象のレンダラー。
/// - `vec3_t start`: Beamの開始座標。
/// - `vec3_t end`: Beamの終了座標。
/// - `float width`: Beam全体の太さ。
/// - `renderer_color_t color`: 色と最大不透明度。
/// - `renderer_blend_mode_t blend_mode`: 合成方法。
/// - `const render_view_t *view`: 描画に使用するビュー。
void renderer_draw_world_beam(renderer_t *renderer,
			      vec3_t start,
			      vec3_t end,
			      float width,
			      renderer_color_t color,
			      renderer_blend_mode_t blend_mode,
			      const render_view_t *view);
/// 現在のViewModelパスにカメラローカルの円形Spriteを描画する。
///
/// ### Args
/// - `renderer_t *renderer`: 対象のレンダラー。
/// - `vec3_t position`: カメラローカルの中心座標。
/// - `float size`: 一辺の半分の長さ。
/// - `renderer_color_t color`: 色と最大不透明度。
/// - `renderer_blend_mode_t blend_mode`: 合成方法。
void renderer_draw_view_model_sprite(renderer_t *renderer,
				     vec3_t position,
				     float size,
				     renderer_color_t color,
				     renderer_blend_mode_t blend_mode);
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
/// ゲームが指定したTrueTypeフォントでASCIIテキストを登録する。
///
/// ### Args
/// - `const renderer_t *renderer`: 対象のレンダラー。
/// - `const renderer_font_t *font`: 使用するフォント。
/// - `float x`: 左上のX座標。
/// - `float y`: 左上のY座標。
/// - `float height`: 文字の高さ。
/// - `renderer_color_t color`: 文字色。
/// - `const char *text`: 描画するASCII文字列。
void renderer_draw_text_with_font(const renderer_t *renderer,
				  const renderer_font_t *font,
				  float x,
				  float y,
				  float height,
				  renderer_color_t color,
				  const char *text);
/// 現在のフレームの描画統計を取得する。
renderer_frame_stats_t renderer_get_frame_stats(const renderer_t *renderer);
/// デバッグ線分の収集を開始する。
///
/// 以前に追加された線分は破棄される。
///
/// ### Args
/// - `renderer_t *renderer`: 対象のレンダラー。
void renderer_begin_debug_lines(renderer_t *renderer);
/// ワールド空間上のデバッグ線分を描画キューへ追加する。
///
/// ### Args
/// - `renderer_t *renderer`: 対象のレンダラー。
/// - `vec3_t start`: 線分の開始座標。
/// - `vec3_t end`: 線分の終了座標。
/// - `renderer_color_t color`: 線分の色。
///
/// ### Returns
/// - `true`: 線分を追加した。
/// - `false`: 引数が不正、またはメモリ確保に失敗した。
bool renderer_add_debug_line(renderer_t *renderer,
			     vec3_t start,
			     vec3_t end,
			     renderer_color_t color);
/// 描画キューに追加されたデバッグ線分を一括描画する。
///
/// 描画後、キューは空になる。
///
/// ### Args
/// - `renderer_t *renderer`: 対象のレンダラー。
/// - `const render_view_t *view`: 描画に使用するビュー情報。
void renderer_flush_debug_lines(renderer_t *renderer,
				const render_view_t *view);

#endif
