/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_RENDERER_SHADOW_MAP_H
#define VOLUME_RENDERER_SHADOW_MAP_H

typedef struct shadow_map shadow_map_t;

/// 指定解像度のシャドウマップを作成する。
///
/// ### Args
/// - `int width`: テクスチャ幅。
/// - `int height`: テクスチャ高さ。
///
/// ### Returns
/// - `shadow_map_t *`: 作成したシャドウマップ。失敗時は`NULL`。
shadow_map_t *shadow_map_create(int width, int height);
/// シャドウマップとGPUリソースを破棄する。
///
/// ### Args
/// - `shadow_map_t *shadow_map`: 破棄するシャドウマップ。
void shadow_map_destroy(shadow_map_t *shadow_map);
/// シャドウマップを描画先としてバインドする。
///
/// ### Args
/// - `const shadow_map_t *shadow_map`: 対象のシャドウマップ。
void shadow_map_begin(const shadow_map_t *shadow_map);
/// シャドウマップへの描画を終了する。
void shadow_map_end(void);
/// シャドウマップの深度テクスチャIDを取得する。
///
/// ### Args
/// - `const shadow_map_t *shadow_map`: 対象のシャドウマップ。
///
/// ### Returns
/// - `unsigned int`: OpenGLテクスチャID。無効な場合は`0`。
unsigned int shadow_map_get_texture(const shadow_map_t *shadow_map);
/// シャドウマップの幅を取得する。
///
/// ### Args
/// - `const shadow_map_t *shadow_map`: 対象のシャドウマップ。
///
/// ### Returns
/// - `int`: テクスチャ幅。
int shadow_map_get_width(const shadow_map_t *shadow_map);
/// シャドウマップの高さを取得する。
///
/// ### Args
/// - `const shadow_map_t *shadow_map`: 対象のシャドウマップ。
///
/// ### Returns
/// - `int`: テクスチャ高さ。
int shadow_map_get_height(const shadow_map_t *shadow_map);

#endif
