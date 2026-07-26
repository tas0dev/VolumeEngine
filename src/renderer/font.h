/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_RENDERER_FONT_H
#define VOLUME_RENDERER_FONT_H

#include "core/types.h"

typedef struct renderer_font renderer_font_t;

/// TrueTypeフォントを読み込み、ASCII描画用アトラスを作成する。
///
/// 呼び出し時にはレンダラーのOpenGLコンテキストが有効である必要がある。
///
/// ### Args
/// - `renderer_t *renderer`: フォントを使用するレンダラー。
/// - `const char *path`: 読み込むTTFファイルのパス。
///
/// ### Returns
/// - `renderer_font_t *`: 作成したフォント。失敗時は`NULL`。
renderer_font_t *renderer_font_create(renderer_t *renderer, const char *path);

/// フォントとGPUアトラスを破棄する。
///
/// ### Args
/// - `renderer_font_t *font`: 破棄するフォント。
void renderer_font_destroy(renderer_font_t *font);

#endif
