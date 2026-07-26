/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_FPS_RECOIL_H
#define VOLUME_FPS_RECOIL_H

#include <stdbool.h>

typedef struct fps_recoil_config {
	float return_strength;
	float damping;
	float maximum_pitch;
	float maximum_yaw;
} fps_recoil_config_t;

typedef struct fps_recoil_offset {
	float pitch;
	float yaw;
} fps_recoil_offset_t;

typedef struct fps_recoil {
	fps_recoil_config_t config;
	fps_recoil_offset_t offset;
	fps_recoil_offset_t velocity;
} fps_recoil_t;

/// FPS用のばね式リコイル状態を初期化する。
///
/// ### Args
/// - `fps_recoil_t *recoil`: 初期化するリコイル状態。
/// - `const fps_recoil_config_t *config`: ゲームが指定する応答設定。
///
/// ### Returns
/// - `true`: 初期化に成功した。
/// - `false`: 引数または設定が不正だった。
bool fps_recoil_initialize(fps_recoil_t *recoil,
			   const fps_recoil_config_t *config);

/// 角速度としてリコイル衝撃を追加する。
///
/// ### Args
/// - `fps_recoil_t *recoil`: 対象のリコイル状態。
/// - `float pitch`: 上下方向へ加える角速度。
/// - `float yaw`: 左右方向へ加える角速度。
void fps_recoil_add_impulse(fps_recoil_t *recoil, float pitch, float yaw);

/// ばねと減衰を固定時間だけ進める。
///
/// ### Args
/// - `fps_recoil_t *recoil`: 更新するリコイル状態。
/// - `float delta_time`: 経過秒数。
void fps_recoil_update(fps_recoil_t *recoil, float delta_time);

/// 現在の視点角度オフセットを取得する。
///
/// ### Args
/// - `const fps_recoil_t *recoil`: 対象のリコイル状態。
///
/// ### Returns
/// - `fps_recoil_offset_t`: pitchとyawの角度オフセット。
fps_recoil_offset_t fps_recoil_get_offset(const fps_recoil_t *recoil);

/// リコイル角度と速度をゼロへ戻す。
///
/// ### Args
/// - `fps_recoil_t *recoil`: リセットするリコイル状態。
void fps_recoil_reset(fps_recoil_t *recoil);

#endif
