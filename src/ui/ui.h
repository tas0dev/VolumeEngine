/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_UI_UI_H
#define VOLUME_UI_UI_H

#include "input/input.h"
#include "renderer/renderer.h"
#include <stdbool.h>

typedef struct ui_context ui_context_t;

typedef struct ui_rect {
	float x;
	float y;
	float width;
	float height;
} ui_rect_t;

typedef struct ui_style {
	renderer_color_t panel_color;
	renderer_color_t button_color;
	renderer_color_t button_hover_color;
	renderer_color_t button_pressed_color;
	renderer_color_t button_disabled_color;
	renderer_color_t text_color;
	renderer_color_t text_disabled_color;
	float text_height;
} ui_style_t;

/// TrueTypeフォントを使用するHUD/UIコンテキストを作成する。
///
/// ### Args
/// - `renderer_t *renderer`: UIを描画するレンダラー。
/// - `input_t *input`: UI操作に使用する入力。
/// - `const renderer_font_t *font`: UI用TrueTypeフォント。
///
/// ### Returns
/// - `ui_context_t *`: 作成したコンテキスト。失敗時は`NULL`。
ui_context_t *ui_context_create(renderer_t *renderer,
				input_t *input,
				const renderer_font_t *font);

/// HUD/UIコンテキストを破棄する。
///
/// ### Args
/// - `ui_context_t *ui`: 破棄するコンテキスト。
void ui_context_destroy(ui_context_t *ui);

/// UIの配色と文字サイズを変更する。
///
/// ### Args
/// - `ui_context_t *ui`: 対象のコンテキスト。
/// - `const ui_style_t *style`: 適用するスタイル。
void ui_context_set_style(ui_context_t *ui, const ui_style_t *style);

/// 単色のUIパネルを描画する。
///
/// ### Args
/// - `ui_context_t *ui`: 対象のコンテキスト。
/// - `ui_rect_t rectangle`: パネル領域。
void ui_panel(ui_context_t *ui, ui_rect_t rectangle);

/// UI用TrueTypeフォントでラベルを描画する。
///
/// ### Args
/// - `ui_context_t *ui`: 対象のコンテキスト。
/// - `float x`: 左上のX座標。
/// - `float y`: 左上のY座標。
/// - `float height`: 文字高。`0`以下ならスタイル値を使用する。
/// - `renderer_color_t color`: 文字色。
/// - `const char *text`: 描画する文字列。
void ui_label(ui_context_t *ui,
	      float x,
	      float y,
	      float height,
	      renderer_color_t color,
	      const char *text);

/// 状態に応じた背景と中央ラベルを持つボタンを描画する。
///
/// ### Args
/// - `ui_context_t *ui`: 対象のコンテキスト。
/// - `ui_rect_t rectangle`: ボタン領域。
/// - `const char *label`: ボタンのラベル。
/// - `bool enabled`: 操作可能なら`true`。
///
/// ### Returns
/// - `true`: このフレームに左クリックされた。
/// - `false`: クリックされていない、または無効。
bool ui_button(ui_context_t *ui,
	       ui_rect_t rectangle,
	       const char *label,
	       bool enabled);

#endif
