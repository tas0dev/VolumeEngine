/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_PLATFORM_PLATFORM_H
#define VOLUME_PLATFORM_PLATFORM_H

#include "core/types.h"
#include <stdbool.h>

typedef struct platform_config {
	const char *title;
	int width;
	int height;
	bool capture_mouse;
} platform_config_t;

/// ウィンドウとプラットフォーム状態を作成する。
///
/// ### Args
/// - `const platform_config_t *config`: ウィンドウ作成設定。
///
/// ### Returns
/// - `platform_t *`: 作成したプラットフォーム。失敗時は`NULL`。
platform_t *platform_create(const platform_config_t *config);
/// プラットフォームとウィンドウを破棄する。
///
/// ### Args
/// - `platform_t *platform`: 破棄するプラットフォーム。
void platform_destroy(platform_t *platform);
/// OSイベントを取得して入力状態へ反映する。
///
/// ### Args
/// - `platform_t *platform`: 対象のプラットフォーム。
/// - `input_t *input`: 更新する入力オブジェクト。
///
/// ### Returns
/// - `true`: アプリケーションを継続できる。
/// - `false`: 終了イベントを受け取った、または処理に失敗した。
bool platform_poll_events(platform_t *platform, input_t *input);
/// プラットフォーム用のOpenGLコンテキストを作成する。
///
/// ### Args
/// - `const platform_t *platform`: 対象のプラットフォーム。
///
/// ### Returns
/// - `void *`: 作成したコンテキスト。失敗時は`NULL`。
void *platform_gl_create_context(const platform_t *platform);
/// OpenGLコンテキストを破棄する。
///
/// ### Args
/// - `void *context`: 破棄するコンテキスト。
void platform_gl_destroy_context(void *context);
/// OpenGLコンテキストを現在のスレッドへ設定する。
///
/// ### Args
/// - `const platform_t *platform`: 対象のプラットフォーム。
/// - `void *context`: 使用するOpenGLコンテキスト。
///
/// ### Returns
/// - `true`: 設定に成功した。
/// - `false`: 設定に失敗した。
bool platform_gl_make_current(const platform_t *platform, void *context);
/// 前面・背面バッファを交換して描画結果を表示する。
///
/// ### Args
/// - `const platform_t *platform`: 対象のプラットフォーム。
void platform_gl_swap_buffers(const platform_t *platform);
/// 実際の描画可能領域サイズを取得する。
///
/// ### Args
/// - `const platform_t *platform`: 対象のプラットフォーム。
/// - `int *width`: 幅の格納先。不要な場合は`NULL`。
/// - `int *height`: 高さの格納先。不要な場合は`NULL`。
void platform_get_drawable_size(const platform_t *platform,
				int *width,
				int *height);
/// 高精度タイマーの現在時刻を取得する。
///
/// ### Returns
/// - `double`: プラットフォーム基準の経過秒数。
double platform_get_time(void);
/// 指定秒数だけ現在のスレッドを休止する。
///
/// ### Args
/// - `double seconds`: 休止する秒数。
void platform_sleep(double seconds);
/// マウスカーソルのキャプチャ状態を設定する。
///
/// ### Args
/// - `platform_t *platform`: 対象のプラットフォーム。
/// - `bool captured`: キャプチャする場合は`true`。
///
/// ### Returns
/// - `true`: 状態変更に成功した。
/// - `false`: 状態変更に失敗した。
bool platform_set_mouse_captured(platform_t *platform, bool captured);
/// マウスカーソルがキャプチャされているか調べる。
///
/// ### Args
/// - `const platform_t *platform`: 対象のプラットフォーム。
///
/// ### Returns
/// - `true`: キャプチャされている。
/// - `false`: キャプチャされていない。
bool platform_is_mouse_captured(const platform_t *platform);

#endif
