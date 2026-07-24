/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_MATH_MAT4_H
#define VOLUME_MATH_MAT4_H

#include "math/vec3.h"
#include <stdbool.h>

typedef struct mat4 {
	float elements[16];
} mat4_t;

/// 4x4単位行列を作成する。
///
/// ### Returns
/// - `mat4_t`: 単位行列。
mat4_t mat4_identity(void);
/// 2つの4x4行列を乗算する。
///
/// ### Args
/// - `mat4_t left`: 左辺の行列。
/// - `mat4_t right`: 右辺の行列。
///
/// ### Returns
/// - `mat4_t`: 行列積。
mat4_t mat4_multiply(mat4_t left, mat4_t right);
/// 平行移動行列を作成する。
///
/// ### Args
/// - `vec3_t translation`: 各軸方向の移動量。
///
/// ### Returns
/// - `mat4_t`: 平行移動行列。
mat4_t mat4_translation(vec3_t translation);
/// X軸回転行列を作成する。
///
/// ### Args
/// - `float radians`: 回転角度（ラジアン）。
///
/// ### Returns
/// - `mat4_t`: X軸回転行列。
mat4_t mat4_rotation_x(float radians);
/// Y軸回転行列を作成する。
///
/// ### Args
/// - `float radians`: 回転角度（ラジアン）。
///
/// ### Returns
/// - `mat4_t`: Y軸回転行列。
mat4_t mat4_rotation_y(float radians);
/// Z軸回転行列を作成する。
///
/// ### Args
/// - `float radians`: 回転角度（ラジアン）。
///
/// ### Returns
/// - `mat4_t`: Z軸回転行列。
mat4_t mat4_rotation_z(float radians);
/// 透視投影行列を作成する。
///
/// ### Args
/// - `float field_of_view`: 垂直視野角（ラジアン）。
/// - `float aspect_ratio`: 描画領域のアスペクト比。
/// - `float near_plane`: 近クリップ面までの距離。
/// - `float far_plane`: 遠クリップ面までの距離。
///
/// ### Returns
/// - `mat4_t`: 透視投影行列。
mat4_t mat4_perspective(float field_of_view,
			float aspect_ratio,
			float near_plane,
			float far_plane);
/// 行列要素配列への読み取り専用ポインタを取得する。
///
/// ### Args
/// - `const mat4_t *matrix`: 対象の行列。
///
/// ### Returns
/// - `const float *`: 16要素の配列先頭。引数が`NULL`の場合は`NULL`。
const float *mat4_data(const mat4_t *matrix);
/// 視点、注視点、上方向からビュー行列を作成する。
///
/// ### Args
/// - `vec3_t position`: 視点位置。
/// - `vec3_t target`: 注視点。
/// - `vec3_t up`: 上方向。
///
/// ### Returns
/// - `mat4_t`: ビュー行列。
mat4_t mat4_look_at(vec3_t position, vec3_t target, vec3_t up);
/// 平行投影行列を作成する。
///
/// ### Args
/// - `float left`: 左クリップ面。
/// - `float right`: 右クリップ面。
/// - `float bottom`: 下クリップ面。
/// - `float top`: 上クリップ面。
/// - `float near_plane`: 近クリップ面。
/// - `float far_plane`: 遠クリップ面。
///
/// ### Returns
/// - `mat4_t`: 平行投影行列。
mat4_t mat4_orthographic(float left,
			 float right,
			 float bottom,
			 float top,
			 float near_plane,
			 float far_plane);
/// スケール行列を作成する。
///
/// ### Args
/// - `vec3_t scale`: 各軸方向の倍率。
///
/// ### Returns
/// - `mat4_t`: スケール行列。
mat4_t mat4_scale(vec3_t scale);
/// 行列で3次元位置を変換する。
///
/// ### Args
/// - `mat4_t matrix`: 適用する変換行列。
/// - `vec3_t point`: 変換する位置。
///
/// ### Returns
/// - `vec3_t`: 変換後の位置。
vec3_t mat4_transform_point(mat4_t matrix, vec3_t point);
/// アフィン変換行列の逆行列を計算する。
///
/// ### Args
/// - `const mat4_t *matrix`: 逆変換する行列。
/// - `mat4_t *inverse`: 結果の格納先。
///
/// ### Returns
/// - `true`: 逆行列を計算できた。
/// - `false`: 行列が特異、または引数が不正だった。
bool mat4_inverse_affine(const mat4_t *matrix, mat4_t *inverse);

#endif
