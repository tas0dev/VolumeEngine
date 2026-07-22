/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_GAME_GAME_H
#define VOLUME_GAME_GAME_H

#include "core/types.h"
#include <stdbool.h>

typedef struct game {
	/// ゲーム開始時に状態を初期化する。
	///
	/// ### Args
	/// - `engine_t *engine`: 実行中のエンジン。
	/// - `void *user_data`: ゲーム固有データ。
	///
	/// ### Returns
	/// - `true`: 初期化に成功した。
	/// - `false`: 初期化に失敗し、実行を中止する。
	bool (*initialize)(engine_t *engine, void *user_data);
	/// 可変間隔のゲーム更新を行う。
	///
	/// ### Args
	/// - `engine_t *engine`: 実行中のエンジン。
	/// - `float delta_time`: 前フレームからの経過秒数。
	/// - `void *user_data`: ゲーム固有データ。
	void (*update)(engine_t *engine, float delta_time, void *user_data);
	/// 固定間隔の物理・ゲーム更新を行う。
	///
	/// ### Args
	/// - `engine_t *engine`: 実行中のエンジン。
	/// - `float delta_time`: 固定tickの経過秒数。
	/// - `void *user_data`: ゲーム固有データ。
	void (*fixed_update)(engine_t *engine,
			     float delta_time,
			     void *user_data);
	/// 現在のゲーム状態を描画する。
	///
	/// ### Args
	/// - `engine_t *engine`: 実行中のエンジン。
	/// - `void *user_data`: ゲーム固有データ。
	void (*render)(engine_t *engine, void *user_data);
	/// ゲーム終了時に所有リソースを解放する。
	///
	/// ### Args
	/// - `engine_t *engine`: 終了するエンジン。
	/// - `void *user_data`: ゲーム固有データ。
	void (*shutdown)(engine_t *engine, void *user_data);
	void *user_data;
} game_t;

#endif
