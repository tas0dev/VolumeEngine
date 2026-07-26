/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_EXAMPLE_SANDBOX_HUD_H
#define VOLUME_EXAMPLE_SANDBOX_HUD_H

#include "renderer/renderer.h"
#include <stdbool.h>

typedef struct sandbox_hud {
	float previous_health;
	float damage_flash_time;
} sandbox_hud_t;

typedef struct sandbox_hud_values {
	int health;
	int ammunition;
	int reserve_ammunition;
	bool alive;
} sandbox_hud_values_t;

/// sandboxゲーム用HUDを初期化する。
///
/// ### Args
/// - `sandbox_hud_t *hud`: 初期化するHUD。
/// - `float health`: 初期ヘルス。
void sandbox_hud_initialize(sandbox_hud_t *hud, float health);

/// ヘルス変化とダメージ表示時間を更新する。
///
/// ### Args
/// - `sandbox_hud_t *hud`: 更新するHUD。
/// - `float health`: 現在ヘルス。
/// - `float delta_time`: 経過秒数。
void sandbox_hud_update(sandbox_hud_t *hud, float health, float delta_time);

/// リスポーン後の表示状態へ戻す。
///
/// ### Args
/// - `sandbox_hud_t *hud`: リセットするHUD。
/// - `float health`: リスポーン後のヘルス。
void sandbox_hud_reset(sandbox_hud_t *hud, float health);

/// Source風のゲームHUDを描画する。
///
/// ### Args
/// - `const sandbox_hud_t *hud`: 描画状態を持つHUD。
/// - `renderer_t *renderer`: 描画に使用するレンダラー。
/// - `const renderer_font_t *font`: ゲームが選択したHUD用フォント。
/// - `int width`: 描画領域の幅。
/// - `int height`: 描画領域の高さ。
/// - `const sandbox_hud_values_t *values`: 表示するゲーム状態。
void sandbox_hud_draw(const sandbox_hud_t *hud,
		      renderer_t *renderer,
		      const renderer_font_t *font,
		      int width,
		      int height,
		      const sandbox_hud_values_t *values);

#endif
