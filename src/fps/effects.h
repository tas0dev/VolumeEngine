/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_FPS_EFFECTS_H
#define VOLUME_FPS_EFFECTS_H

#include "math/vec3.h"
#include "renderer/renderer.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct fps_effect_system fps_effect_system_t;

typedef struct fps_billboard_effect_config {
	vec3_t position;
	vec3_t normal;
	float start_size;
	float end_size;
	float lifetime;
	renderer_color_t start_color;
	renderer_color_t end_color;
	renderer_blend_mode_t blend_mode;
} fps_billboard_effect_config_t;

typedef struct fps_tracer_effect_config {
	vec3_t start;
	vec3_t end;
	float lifetime;
	renderer_color_t start_color;
	renderer_color_t end_color;
} fps_tracer_effect_config_t;

/// FPS向けの短命な描画エフェクトを管理するシステムを作成する。
///
/// ### Returns
/// - `fps_effect_system_t *`: 作成したシステム。確保に失敗した場合は`NULL`。
fps_effect_system_t *fps_effect_system_create(void);

/// エフェクトシステムと保持中の全エフェクトを破棄する。
///
/// ### Args
/// - `fps_effect_system_t *system`: 破棄するシステム。
void fps_effect_system_destroy(fps_effect_system_t *system);

/// 保持中の全エフェクトを消去する。
///
/// ### Args
/// - `fps_effect_system_t *system`: 対象のシステム。
void fps_effect_system_clear(fps_effect_system_t *system);

/// 指定した経過時間だけ全エフェクトを進め、寿命切れを除去する。
///
/// ### Args
/// - `fps_effect_system_t *system`: 更新するシステム。
/// - `float delta_time`: 経過秒数。
void fps_effect_system_update(fps_effect_system_t *system, float delta_time);

/// Billboardまたは面に沿うSpriteエフェクトを生成する。
/// `normal`がゼロの場合は常にカメラへ正対する。
///
/// ### Args
/// - `fps_effect_system_t *system`: 生成先のシステム。
/// - `const fps_billboard_effect_config_t *config`: エフェクト設定。
///
/// ### Returns
/// - `true`: 生成した。
/// - `false`: 引数が不正、または保持上限に達している。
bool fps_effect_system_spawn_billboard(
	fps_effect_system_t *system,
	const fps_billboard_effect_config_t *config);

/// 2点間を結ぶTracerエフェクトを生成する。
///
/// ### Args
/// - `fps_effect_system_t *system`: 生成先のシステム。
/// - `const fps_tracer_effect_config_t *config`: エフェクト設定。
///
/// ### Returns
/// - `true`: 生成した。
/// - `false`: 引数が不正、または保持上限に達している。
bool fps_effect_system_spawn_tracer(fps_effect_system_t *system,
				    const fps_tracer_effect_config_t *config);

/// 保持中のワールド空間エフェクトを描画する。
///
/// ### Args
/// - `const fps_effect_system_t *system`: 描画するシステム。
/// - `renderer_t *renderer`: 使用するレンダラー。
/// - `const render_view_t *view`: ワールドの描画ビュー。
void fps_effect_system_draw(const fps_effect_system_t *system,
			    renderer_t *renderer,
			    const render_view_t *view);

/// 現在保持しているエフェクトの総数を取得する。
///
/// ### Args
/// - `const fps_effect_system_t *system`: 対象のシステム。
///
/// ### Returns
/// - `size_t`: BillboardとTracerを合計した数。
size_t fps_effect_system_get_count(const fps_effect_system_t *system);

#endif
