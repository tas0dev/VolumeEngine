/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_COLLISION_TRIANGLE_H
#define VOLUME_COLLISION_TRIANGLE_H
#include "collision/aabb.h"
#include "math/vec3.h"
#include <stdbool.h>

typedef struct triangle {
	vec3_t vertices[3];
	vec3_t normal;
} triangle_t;

/// 3頂点から法線付き三角形を作成する。
///
/// ### Args
/// - `vec3_t first`: 1つ目の頂点。
/// - `vec3_t second`: 2つ目の頂点。
/// - `vec3_t third`: 3つ目の頂点。
///
/// ### Returns
/// - `triangle_t`: 作成した三角形。
triangle_t triangle_create(vec3_t first, vec3_t second, vec3_t third);
/// 三角形を包含する境界ボックスを取得する。
///
/// ### Args
/// - `triangle_t triangle`: 対象の三角形。
///
/// ### Returns
/// - `aabb_t`: 三角形を包含するAABB。
aabb_t triangle_get_bounds(triangle_t triangle);
/// AABBと三角形の押し出し情報を計算する。
///
/// ### Args
/// - `aabb_t aabb`: 対象の境界ボックス。
/// - `triangle_t triangle`: 相手の三角形。
/// - `aabb_collision_t *collision`: 衝突法線と深度の格納先。
///
/// ### Returns
/// - `true`: 重なりがあり衝突情報を計算した。
/// - `false`: 重なっていない、または引数が不正だった。
bool aabb_get_triangle_collision(aabb_t aabb,
				 triangle_t triangle,
				 aabb_collision_t *collision);

#endif
