/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_INPUT_INPUT_H
#define VOLUME_INPUT_INPUT_H

#include "core/types.h"
#include <stdbool.h>

typedef enum input_key {
	INPUT_KEY_A,
	INPUT_KEY_B,
	INPUT_KEY_C,
	INPUT_KEY_D,
	INPUT_KEY_E,
	INPUT_KEY_F,
	INPUT_KEY_G,
	INPUT_KEY_H,
	INPUT_KEY_I,
	INPUT_KEY_J,
	INPUT_KEY_K,
	INPUT_KEY_L,
	INPUT_KEY_M,
	INPUT_KEY_N,
	INPUT_KEY_O,
	INPUT_KEY_P,
	INPUT_KEY_Q,
	INPUT_KEY_R,
	INPUT_KEY_S,
	INPUT_KEY_T,
	INPUT_KEY_U,
	INPUT_KEY_V,
	INPUT_KEY_W,
	INPUT_KEY_X,
	INPUT_KEY_Y,
	INPUT_KEY_Z,
	INPUT_KEY_SPACE,
	INPUT_KEY_CONTROL,
	INPUT_KEY_ESCAPE,
	INPUT_KEY_COUNT,
} input_key_t;

typedef enum input_mouse_button {
	INPUT_MOUSE_BUTTON_LEFT,
	INPUT_MOUSE_BUTTON_MIDDLE,
	INPUT_MOUSE_BUTTON_RIGHT,
	INPUT_MOUSE_BUTTON_COUNT,
} input_mouse_button_t;

/// 入力状態を管理するオブジェクトを作成する。
///
/// ### Returns
/// - `input_t *`: 作成した入力オブジェクト。失敗時は`NULL`。
input_t *input_create(void);
/// 入力オブジェクトを破棄する。
///
/// ### Args
/// - `input_t *input`: 破棄する入力オブジェクト。
void input_destroy(input_t *input);
/// フレーム固有の入力差分をリセットして新しいフレームを開始する。
///
/// ### Args
/// - `input_t *input`: 更新する入力オブジェクト。
void input_begin_frame(input_t *input);
/// キーが現在押されているか調べる。
///
/// ### Args
/// - `const input_t *input`: 対象の入力オブジェクト。
/// - `input_key_t key`: 調べるキー。
///
/// ### Returns
/// - `true`: 現在押されている。
/// - `false`: 押されていない、または引数が不正だった。
bool input_key_down(const input_t *input, input_key_t key);
/// キーがこのフレームに押されたか調べる。
///
/// ### Args
/// - `const input_t *input`: 対象の入力オブジェクト。
/// - `input_key_t key`: 調べるキー。
///
/// ### Returns
/// - `true`: このフレームに押された。
/// - `false`: 押された瞬間ではない。
bool input_key_pressed(const input_t *input, input_key_t key);
/// キーがこのフレームに離されたか調べる。
///
/// ### Args
/// - `const input_t *input`: 対象の入力オブジェクト。
/// - `input_key_t key`: 調べるキー。
///
/// ### Returns
/// - `true`: このフレームに離された。
/// - `false`: 離された瞬間ではない。
bool input_key_released(const input_t *input, input_key_t key);
/// マウスボタンが現在押されているか調べる。
///
/// ### Args
/// - `const input_t *input`: 対象の入力オブジェクト。
/// - `input_mouse_button_t button`: 調べるボタン。
///
/// ### Returns
/// - `true`: 現在押されている。
/// - `false`: 押されていない、または引数が不正だった。
bool input_mouse_button_down(const input_t *input, input_mouse_button_t button);
/// マウスボタンがこのフレームに押されたか調べる。
///
/// ### Args
/// - `const input_t *input`: 対象の入力オブジェクト。
/// - `input_mouse_button_t button`: 調べるボタン。
///
/// ### Returns
/// - `true`: このフレームに押された。
/// - `false`: 押された瞬間ではない。
bool input_mouse_button_pressed(const input_t *input,
				input_mouse_button_t button);
/// マウスボタンがこのフレームに離されたか調べる。
///
/// ### Args
/// - `const input_t *input`: 対象の入力オブジェクト。
/// - `input_mouse_button_t button`: 調べるボタン。
///
/// ### Returns
/// - `true`: このフレームに離された。
/// - `false`: 離された瞬間ではない。
bool input_mouse_button_released(const input_t *input,
				 input_mouse_button_t button);
/// 現在のマウス座標を取得する。
///
/// ### Args
/// - `const input_t *input`: 対象の入力オブジェクト。
/// - `float *x`: X座標の格納先。不要な場合は`NULL`。
/// - `float *y`: Y座標の格納先。不要な場合は`NULL`。
void input_get_mouse_position(const input_t *input, float *x, float *y);
/// このフレームのマウス移動量を取得する。
///
/// ### Args
/// - `const input_t *input`: 対象の入力オブジェクト。
/// - `float *x`: X方向移動量の格納先。不要な場合は`NULL`。
/// - `float *y`: Y方向移動量の格納先。不要な場合は`NULL`。
void input_get_mouse_delta(const input_t *input, float *x, float *y);
/// このフレームのマウスホイール移動量を取得する。
///
/// ### Args
/// - `const input_t *input`: 対象の入力オブジェクト。
/// - `float *x`: 水平方向移動量の格納先。不要な場合は`NULL`。
/// - `float *y`: 垂直方向移動量の格納先。不要な場合は`NULL`。
void input_get_mouse_wheel(const input_t *input, float *x, float *y);
/// キーの現在状態を設定する。
///
/// ### Args
/// - `input_t *input`: 更新する入力オブジェクト。
/// - `input_key_t key`: 更新するキー。
/// - `bool down`: 押下状態なら`true`。
void input_set_key(input_t *input, input_key_t key, bool down);
/// マウスボタンの現在状態を設定する。
///
/// ### Args
/// - `input_t *input`: 更新する入力オブジェクト。
/// - `input_mouse_button_t button`: 更新するボタン。
/// - `bool down`: 押下状態なら`true`。
void input_set_mouse_button(input_t *input,
			    input_mouse_button_t button,
			    bool down);
/// マウスの現在座標を設定する。
///
/// ### Args
/// - `input_t *input`: 更新する入力オブジェクト。
/// - `float x`: 新しいX座標。
/// - `float y`: 新しいY座標。
void input_set_mouse_position(input_t *input, float x, float y);
/// このフレームのマウス移動量を加算する。
///
/// ### Args
/// - `input_t *input`: 更新する入力オブジェクト。
/// - `float x`: X方向の追加移動量。
/// - `float y`: Y方向の追加移動量。
void input_add_mouse_delta(input_t *input, float x, float y);
/// このフレームのホイール移動量を加算する。
///
/// ### Args
/// - `input_t *input`: 更新する入力オブジェクト。
/// - `float x`: 水平方向の追加移動量。
/// - `float y`: 垂直方向の追加移動量。
void input_add_mouse_wheel(input_t *input, float x, float y);

#endif
