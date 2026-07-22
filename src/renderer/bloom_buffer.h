/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_RENDERER_BLOOM_BUFFER_H
#define VOLUME_RENDERER_BLOOM_BUFFER_H

#include <stdbool.h>

typedef struct bloom_buffer bloom_buffer_t;

/// Bloomぼかし処理用のピンポンバッファを作成する。
///
/// ### Args
/// - `int width`: バッファ幅。
/// - `int height`: バッファ高さ。
///
/// ### Returns
/// - `bloom_buffer_t *`: 作成したバッファ。失敗時は`NULL`。
bloom_buffer_t *bloom_buffer_create(int width, int height);
/// BloomバッファとGPUリソースを破棄する。
///
/// ### Args
/// - `bloom_buffer_t *buffer`: 破棄するバッファ。
void bloom_buffer_destroy(bloom_buffer_t *buffer);
/// Bloomバッファを指定サイズへ変更する。
///
/// ### Args
/// - `bloom_buffer_t *buffer`: 対象のバッファ。
/// - `int width`: 新しい幅。
/// - `int height`: 新しい高さ。
///
/// ### Returns
/// - `true`: サイズ変更に成功した。
/// - `false`: サイズが不正、またはGPUリソース作成に失敗した。
bool bloom_buffer_resize(bloom_buffer_t *buffer, int width, int height);
/// 指定側のBloomバッファを描画先としてバインドする。
///
/// ### Args
/// - `const bloom_buffer_t *buffer`: 対象のバッファ。
/// - `int index`: 使用するピンポンバッファの添字。
void bloom_buffer_bind(const bloom_buffer_t *buffer, int index);
/// 指定側のBloomテクスチャIDを取得する。
///
/// ### Args
/// - `const bloom_buffer_t *buffer`: 対象のバッファ。
/// - `int index`: 取得するピンポンバッファの添字。
///
/// ### Returns
/// - `unsigned int`: OpenGLテクスチャID。無効な場合は`0`。
unsigned int bloom_buffer_get_texture(const bloom_buffer_t *buffer, int index);

#endif
