/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_ASSET_MESH_LOADER_H
#define VOLUME_ASSET_MESH_LOADER_H

#include "renderer/mesh.h"
#include <stddef.h>

/// モデルファイルから描画メッシュを読み込む。
///
/// ### Args
/// - `const char *path`: 読み込むモデルファイルのパス。
/// - `char *error`: エラーメッセージの格納先。
/// - `size_t error_size`: エラー格納先のバイト数。
///
/// ### Returns
/// - `mesh_t *`: 読み込んだメッシュ。失敗時は`NULL`。
mesh_t *mesh_load(const char *path, char *error, size_t error_size);

#endif
