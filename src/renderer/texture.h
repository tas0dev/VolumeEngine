/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_RENDERER_TEXTURE_H
#define VOLUME_RENDERER_TEXTURE_H

#include <stddef.h>

typedef struct texture texture_t;

/// 画像ファイルからGPUテクスチャを読み込む。
///
/// ### Args
/// - `const char *path`: 読み込む画像ファイルのパス。
/// - `char *error`: エラーメッセージの格納先。
/// - `size_t error_size`: エラー格納先のバイト数。
///
/// ### Returns
/// - `texture_t *`: 作成したテクスチャ。失敗時は`NULL`。
texture_t *texture_load(const char *path, char *error, size_t error_size);
/// GPUテクスチャを破棄する。
///
/// ### Args
/// - `texture_t *texture`: 破棄するテクスチャ。
void texture_destroy(texture_t *texture);
/// テクスチャを指定ユニットへバインドする。
///
/// ### Args
/// - `const texture_t *texture`: バインドするテクスチャ。
/// - `unsigned int unit`: 使用するテクスチャユニット。
void texture_bind(const texture_t *texture, unsigned int unit);
/// 指定テクスチャユニットのバインドを解除する。
///
/// ### Args
/// - `unsigned int unit`: 解除するテクスチャユニット。
void texture_unbind(unsigned int unit);

#endif
