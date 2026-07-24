/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_SCENE_TRANSFORM_H
#define VOLUME_SCENE_TRANSFORM_H

#include "math/mat4.h"
#include "math/vec3.h"

typedef struct transform {
	vec3_t position;
	vec3_t rotation;
	vec3_t scale;
} transform_t;

/// 単位スケールの既定トランスフォームを作成する。
///
/// ### Returns
/// - `transform_t`: 初期化済みのトランスフォーム。
transform_t transform_create(void);
/// 位置、回転、スケールを合成した変換行列を取得する。
///
/// ### Args
/// - `const transform_t *transform`: 対象のトランスフォーム。
///
/// ### Returns
/// - `mat4_t`: 合成済みの変換行列。
mat4_t transform_get_matrix(const transform_t *transform);
/// 行列を位置、回転、スケールへ分解する。
///
/// シアーと反転を含む行列は分解できない。
///
/// ### Args
/// - `const mat4_t *matrix`: 分解する行列。
/// - `transform_t *transform`: 結果の格納先。
///
/// ### Returns
/// - `true`: 分解に成功した。
/// - `false`: 行列が不正、特異、シアーまたは反転を含む。
bool transform_from_matrix(const mat4_t *matrix, transform_t *transform);

#endif
