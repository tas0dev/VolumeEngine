/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_RENDERER_HDR_BUFFER_H
#define VOLUME_RENDERER_HDR_BUFFER_H
#include <stdbool.h>

typedef struct hdr_buffer hdr_buffer_t;

/// HDR描画用フレームバッファを作成する。
///
/// ### Args
/// - `int width`: バッファ幅。
/// - `int height`: バッファ高さ。
///
/// ### Returns
/// - `hdr_buffer_t *`: 作成したバッファ。失敗時は`NULL`。
hdr_buffer_t *hdr_buffer_create(int width, int height);
/// HDRバッファとGPUリソースを破棄する。
///
/// ### Args
/// - `hdr_buffer_t *buffer`: 破棄するバッファ。
void hdr_buffer_destroy(hdr_buffer_t *buffer);
/// HDRバッファを指定サイズへ変更する。
///
/// ### Args
/// - `hdr_buffer_t *buffer`: 対象のバッファ。
/// - `int width`: 新しい幅。
/// - `int height`: 新しい高さ。
///
/// ### Returns
/// - `true`: サイズ変更に成功した。
/// - `false`: サイズが不正、またはGPUリソース作成に失敗した。
bool hdr_buffer_resize(hdr_buffer_t *buffer, int width, int height);
/// HDRバッファを描画先としてバインドする。
///
/// ### Args
/// - `const hdr_buffer_t *buffer`: 対象のバッファ。
void hdr_buffer_bind(const hdr_buffer_t *buffer);
/// HDRバッファのバインドを解除する。
void hdr_buffer_unbind(void);
/// HDRカラー出力のテクスチャIDを取得する。
///
/// ### Args
/// - `const hdr_buffer_t *buffer`: 対象のバッファ。
///
/// ### Returns
/// - `unsigned int`: OpenGLテクスチャID。無効な場合は`0`。
unsigned int hdr_buffer_get_texture(const hdr_buffer_t *buffer);
/// 輝度抽出出力のテクスチャIDを取得する。
///
/// ### Args
/// - `const hdr_buffer_t *buffer`: 対象のバッファ。
///
/// ### Returns
/// - `unsigned int`: OpenGLテクスチャID。無効な場合は`0`。
unsigned int hdr_buffer_get_brightness_texture(const hdr_buffer_t *buffer);

#endif
