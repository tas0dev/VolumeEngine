/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_SCENE_CAMERA_H
#define VOLUME_SCENE_CAMERA_H

#include "math/mat4.h"
#include "math/vec3.h"

typedef struct camera {
	vec3_t position;
	vec3_t forward;
	vec3_t up;
	float field_of_view;
	float near_plane;
	float far_plane;
} camera_t;

/// 指定位置に既定設定のカメラを作成する。
///
/// ### Args
/// - `vec3_t position`: カメラの初期ワールド座標。
///
/// ### Returns
/// - `camera_t`: 初期化済みのカメラ。
camera_t camera_create(vec3_t position);
/// カメラのビュー行列を取得する。
///
/// ### Args
/// - `const camera_t *camera`: 対象のカメラ。
///
/// ### Returns
/// - `mat4_t`: ワールド空間をビュー空間へ変換する行列。
mat4_t camera_get_view(const camera_t *camera);
/// カメラの透視投影行列を取得する。
///
/// ### Args
/// - `const camera_t *camera`: 対象のカメラ。
/// - `float aspect_ratio`: 描画領域のアスペクト比。
///
/// ### Returns
/// - `mat4_t`: 透視投影行列。
mat4_t camera_get_projection(const camera_t *camera, float aspect_ratio);

#endif
