/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_COLLISION_COLLIDER_H
#define VOLUME_COLLISION_COLLIDER_H

#include "collision/aabb.h"
#include "collision/box_collider.h"
#include "collision/triangle_mesh_collider.h"
#include "math/mat4.h"
#include "math/vec3.h"
#include <stdbool.h>

typedef enum collider_type {
	COLLIDER_TYPE_NONE,
	COLLIDER_TYPE_BOX,
	COLLIDER_TYPE_TRIANGLE_MESH,
} collider_type_t;

typedef struct triangle_mesh_collider_instance {
	const triangle_mesh_collider_t *mesh;
	mat4_t transform;
	aabb_t bounds;
} triangle_mesh_collider_instance_t;

typedef union collider_shape {
	box_collider_t box;
	triangle_mesh_collider_instance_t triangle_mesh;
} collider_shape_t;

typedef struct collider {
	collider_type_t type;
	collider_shape_t shape;
} collider_t;

/// 形状を持たないコライダーを作成する。
///
/// ### Returns
/// - `collider_t`: `COLLIDER_TYPE_NONE`のコライダー。
collider_t collider_create_none(void);
/// 箱形状の汎用コライダーを作成する。
///
/// ### Args
/// - `vec3_t center`: ローカル空間の中心。
/// - `vec3_t half_extents`: 各軸方向の半サイズ。
///
/// ### Returns
/// - `collider_t`: 箱形状のコライダー。
collider_t collider_create_box(vec3_t center, vec3_t half_extents);
/// コライダーのワールド空間AABBを取得する。
///
/// ### Args
/// - `const collider_t *collider`: 対象のコライダー。
/// - `vec3_t position`: コライダー所有者のワールド座標。
/// - `aabb_t *aabb`: 取得結果の格納先。
///
/// ### Returns
/// - `true`: 境界ボックスを取得した。
/// - `false`: 形状がない、または引数が不正だった。
bool collider_get_aabb(const collider_t *collider,
		       vec3_t position,
		       aabb_t *aabb);
/// 変換行列付きの三角形メッシュコライダーを作成する。
///
/// ### Args
/// - `const triangle_mesh_collider_t *mesh`: 参照するメッシュ形状。
/// - `mat4_t transform`: メッシュへ適用する変換行列。
///
/// ### Returns
/// - `collider_t`: 三角形メッシュ形状のコライダー。失敗時は形状なし。
collider_t collider_create_triangle_mesh(const triangle_mesh_collider_t *mesh,
					 mat4_t transform);
/// 変換行列付きの箱コライダーを作成する。
///
/// ### Args
/// - `vec3_t center`: ローカル中心。
/// - `vec3_t half_extents`: ローカル半サイズ。
/// - `mat4_t transform`: 箱へ適用する変換。
///
/// ### Returns
/// - `collider_t`: 箱コライダー。
collider_t collider_create_box_transformed(vec3_t center,
					   vec3_t half_extents,
					   mat4_t transform);

#endif
