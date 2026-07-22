/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_COLLISION_BOX_COLLIDER_H
#define VOLUME_COLLISION_BOX_COLLIDER_H

#include "collision/aabb.h"
#include "math/vec3.h"

typedef struct box_collider {
	vec3_t center;
	vec3_t half_extents;
} box_collider_t;

/// ローカル中心と半サイズから箱コライダーを作成する。
///
/// ### Args
/// - `vec3_t center`: ローカル空間の中心。
/// - `vec3_t half_extents`: 各軸方向の半サイズ。
///
/// ### Returns
/// - `box_collider_t`: 作成した箱コライダー。
box_collider_t box_collider_create(vec3_t center, vec3_t half_extents);
/// 箱コライダーのワールド空間AABBを取得する。
///
/// ### Args
/// - `box_collider_t collider`: 対象の箱コライダー。
/// - `vec3_t position`: コライダー所有者のワールド座標。
///
/// ### Returns
/// - `aabb_t`: ワールド空間の境界ボックス。
aabb_t box_collider_get_aabb(box_collider_t collider, vec3_t position);

#endif
