/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_RENDERER_MESH_H
#define VOLUME_RENDERER_MESH_H
#include "collision/aabb.h"
#include "collision/triangle_mesh_collider.h"

#include <stdbool.h>
#include <stddef.h>

typedef struct mesh_vertex {
	float position[3];
	float normal[3];
	float color[3];
	float texture_coordinate[2];
	float tangent[3];
	float bitangent[3];
} mesh_vertex_t;

typedef struct mesh mesh_t;

/// 頂点とインデックス配列から描画メッシュを作成する。
///
/// ### Args
/// - `const mesh_vertex_t *vertices`: 頂点配列。
/// - `size_t vertex_count`: 頂点数。
/// - `const unsigned int *indices`: インデックス配列。
/// - `size_t index_count`: インデックス数。
///
/// ### Returns
/// - `mesh_t *`: 作成したメッシュ。失敗時は`NULL`。
mesh_t *mesh_create(const mesh_vertex_t *vertices,
		    size_t vertex_count,
		    const unsigned int *indices,
		    size_t index_count);
/// 描画メッシュとGPUリソースを破棄する。
///
/// ### Args
/// - `mesh_t *mesh`: 破棄するメッシュ。
void mesh_destroy(mesh_t *mesh);
/// メッシュを現在の描画状態で描画する。
///
/// ### Args
/// - `const mesh_t *mesh`: 描画するメッシュ。
void mesh_draw(const mesh_t *mesh);
/// メッシュ頂点を包含する境界ボックスを取得する。
///
/// ### Args
/// - `const mesh_t *mesh`: 対象のメッシュ。
/// - `aabb_t *bounds`: 取得結果の格納先。
///
/// ### Returns
/// - `true`: 境界ボックスを取得した。
/// - `false`: メッシュまたは格納先が不正だった。
bool mesh_get_bounds(const mesh_t *mesh, aabb_t *bounds);
/// メッシュから生成された衝突用三角形メッシュを取得する。
///
/// ### Args
/// - `const mesh_t *mesh`: 対象のメッシュ。
///
/// ### Returns
/// - `const triangle_mesh_collider_t *`: 衝突メッシュ。存在しない場合は`NULL`。
const triangle_mesh_collider_t *mesh_get_collision_mesh(const mesh_t *mesh);

#endif
