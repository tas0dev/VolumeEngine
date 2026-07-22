/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_COLLISION_AABB_H
#define VOLUME_COLLISION_AABB_H

#include "math/vec3.h"
#include <stdbool.h>

typedef struct aabb {
	vec3_t minimum;
	vec3_t maximum;
} aabb_t;

typedef struct aabb_collision {
	vec3_t normal;
	float depth;
} aabb_collision_t;

/// 中心と半サイズから軸平行境界ボックスを作成する。
///
/// ### Args
/// - `vec3_t center`: ボックスの中心。
/// - `vec3_t half_extents`: 各軸方向の半サイズ。
///
/// ### Returns
/// - `aabb_t`: 作成した境界ボックス。
aabb_t aabb_create(vec3_t center, vec3_t half_extents);
/// 境界ボックスを指定量だけ平行移動する。
///
/// ### Args
/// - `aabb_t aabb`: 移動する境界ボックス。
/// - `vec3_t offset`: 加算する移動量。
///
/// ### Returns
/// - `aabb_t`: 移動後の境界ボックス。
aabb_t aabb_translate(aabb_t aabb, vec3_t offset);
/// 境界ボックスの中心を取得する。
///
/// ### Args
/// - `aabb_t aabb`: 対象の境界ボックス。
///
/// ### Returns
/// - `vec3_t`: ボックスの中心。
vec3_t aabb_get_center(aabb_t aabb);
/// 境界ボックスの各軸方向の半サイズを取得する。
///
/// ### Args
/// - `aabb_t aabb`: 対象の境界ボックス。
///
/// ### Returns
/// - `vec3_t`: 各軸方向の半サイズ。
vec3_t aabb_get_half_extents(aabb_t aabb);
/// 2つの境界ボックスが交差しているか調べる。
///
/// ### Args
/// - `aabb_t first`: 1つ目の境界ボックス。
/// - `aabb_t second`: 2つ目の境界ボックス。
///
/// ### Returns
/// - `true`: 交差または接触している。
/// - `false`: 離れている。
bool aabb_intersects(aabb_t first, aabb_t second);
/// 2つの境界ボックスの押し出し情報を計算する。
///
/// ### Args
/// - `aabb_t first`: 押し出される境界ボックス。
/// - `aabb_t second`: 相手の境界ボックス。
/// - `aabb_collision_t *collision`: 衝突法線と深度の格納先。
///
/// ### Returns
/// - `true`: 重なりがあり衝突情報を計算した。
/// - `false`: 重なっていない、または引数が不正だった。
bool aabb_get_collision(aabb_t first,
			aabb_t second,
			aabb_collision_t *collision);

#endif
