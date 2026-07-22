/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_ENGINE_H
#define VOLUME_ENGINE_H

// ReSharper disable CppUnusedIncludeDirective
#include "asset/manager.h"
#include "core/log.h"
#include "core/path.h"
#include "core/types.h"
#include "entity/info_player_start.h"
#include "entity/light_environment.h"
#include "entity/player.h"
#include "entity/world.h"
#include "game/game.h"
#include "input/input.h"
#include "map/spawn.h"
#include "math/mat4.h"
#include "math/math.h"
#include "math/vec3.h"
#include "physics/character_controller.h"
#include "renderer/renderer.h"
#include "scene/camera.h"
#include "scene/transform.h"
#include <stdbool.h>

typedef struct engine_config {
	const char *application_name;
	int window_width;
	int window_height;
	bool capture_mouse;
	float fixed_delta_time;
	const game_t *game;
} engine_config_t;

/// 設定からゲームエンジンを作成する。
///
/// ### Args
/// - `const engine_config_t *config`: ウィンドウ、固定tick、ゲームの設定。
///
/// ### Returns
/// - `engine_t *`: 作成したエンジン。失敗時は`NULL`。
engine_t *engine_create(const engine_config_t *config);
/// エンジンと所有する全サブシステムを破棄する。
///
/// ### Args
/// - `engine_t *engine`: 破棄するエンジン。
void engine_destroy(engine_t *engine);
/// 終了イベントまでゲームループを実行する。
///
/// ### Args
/// - `engine_t *engine`: 実行するエンジン。
///
/// ### Returns
/// - `true`: 正常に初期化・実行・終了した。
/// - `false`: 初期化または実行中に失敗した。
bool engine_run(engine_t *engine);
/// エンジンが所有するレンダラーを取得する。
///
/// ### Args
/// - `engine_t *engine`: 対象のエンジン。
///
/// ### Returns
/// - `renderer_t *`: レンダラー。引数が`NULL`の場合は`NULL`。
renderer_t *engine_get_renderer(engine_t *engine);
/// エンジンが所有する入力オブジェクトを取得する。
///
/// ### Args
/// - `engine_t *engine`: 対象のエンジン。
///
/// ### Returns
/// - `input_t *`: 入力オブジェクト。引数が`NULL`の場合は`NULL`。
input_t *engine_get_input(engine_t *engine);
/// エンジンウィンドウのマウスキャプチャ状態を設定する。
///
/// ### Args
/// - `engine_t *engine`: 対象のエンジン。
/// - `bool captured`: キャプチャする場合は`true`。
///
/// ### Returns
/// - `true`: 状態変更に成功した。
/// - `false`: 状態変更に失敗した。
bool engine_set_mouse_captured(engine_t *engine, bool captured);
/// エンジンウィンドウがマウスをキャプチャしているか調べる。
///
/// ### Args
/// - `const engine_t *engine`: 対象のエンジン。
///
/// ### Returns
/// - `true`: キャプチャしている。
/// - `false`: キャプチャしていない。
bool engine_is_mouse_captured(const engine_t *engine);

#endif
