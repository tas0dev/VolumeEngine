/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_COLLISION_TRIANGLE_MESH_COLLIDER_H
#define VOLUME_COLLISION_TRIANGLE_MESH_COLLIDER_H
#include "collision/aabb.h"
#include "collision/triangle.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct triangle_mesh_collider triangle_mesh_collider_t;

/// 三角形配列を複製してメッシュコライダーを作成する。
///
/// ### Args
/// - `const triangle_t *triangles`: 元となる三角形配列。
/// - `size_t triangle_count`: 配列内の三角形数。
///
/// ### Returns
/// - `triangle_mesh_collider_t *`: 作成したコライダー。失敗時は`NULL`。
triangle_mesh_collider_t *
triangle_mesh_collider_create(const triangle_t *triangles,
			      size_t triangle_count);
/// 三角形メッシュコライダーを破棄する。
///
/// ### Args
/// - `triangle_mesh_collider_t *collider`: 破棄するコライダー。
void triangle_mesh_collider_destroy(triangle_mesh_collider_t *collider);
/// メッシュコライダーの三角形数を取得する。
///
/// ### Args
/// - `const triangle_mesh_collider_t *collider`: 対象のコライダー。
///
/// ### Returns
/// - `size_t`: 三角形数。
size_t triangle_mesh_collider_get_triangle_count(
	const triangle_mesh_collider_t *collider);
/// 指定添字の三角形を取得する。
///
/// ### Args
/// - `const triangle_mesh_collider_t *collider`: 対象のコライダー。
/// - `size_t index`: 取得する三角形の添字。
/// - `triangle_t *triangle`: 取得結果の格納先。
///
/// ### Returns
/// - `true`: 三角形を取得した。
/// - `false`: 添字または引数が不正だった。
bool triangle_mesh_collider_get_triangle(
	const triangle_mesh_collider_t *collider,
	size_t index,
	triangle_t *triangle);
/// メッシュ全体を包含する境界ボックスを取得する。
///
/// ### Args
/// - `const triangle_mesh_collider_t *collider`: 対象のコライダー。
/// - `aabb_t *bounds`: 取得結果の格納先。
///
/// ### Returns
/// - `true`: 境界ボックスを取得した。
/// - `false`: 引数が不正だった。
bool triangle_mesh_collider_get_bounds(const triangle_mesh_collider_t *collider,
				       aabb_t *bounds);

#endif
