/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_ASSET_MATERIAL_LOADER_H
#define VOLUME_ASSET_MATERIAL_LOADER_H

#include "renderer/material.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct material_definition {
	material_t material;
	char *albedo_texture_path;
	char *normal_texture_path;
} material_definition_t;

/// 文字列からテクスチャ参照を含むマテリアル定義を解析する。
///
/// ### Args
/// - `const char *source`: 解析する定義文字列。
/// - `material_definition_t *definition`: 解析結果の格納先。
/// - `char *error`: エラーメッセージの格納先。
/// - `size_t error_size`: エラー格納先のバイト数。
///
/// ### Returns
/// - `true`: 解析に成功した。
/// - `false`: 定義または引数が不正だった。
bool material_definition_parse(const char *source,
			       material_definition_t *definition,
			       char *error,
			       size_t error_size);
/// ファイルからテクスチャ参照を含むマテリアル定義を読み込む。
///
/// ### Args
/// - `const char *path`: 読み込む定義ファイルのパス。
/// - `material_definition_t *definition`: 読み込み結果の格納先。
/// - `char *error`: エラーメッセージの格納先。
/// - `size_t error_size`: エラー格納先のバイト数。
///
/// ### Returns
/// - `true`: 読み込みに成功した。
/// - `false`: ファイルまたは定義が不正だった。
bool material_definition_load(const char *path,
			      material_definition_t *definition,
			      char *error,
			      size_t error_size);
/// マテリアル定義が所有する文字列を破棄する。
///
/// ### Args
/// - `material_definition_t *definition`: 破棄する定義。
void material_definition_destroy(material_definition_t *definition);
/// 文字列からテクスチャを除くマテリアル値を解析する。
///
/// ### Args
/// - `const char *source`: 解析する定義文字列。
/// - `material_t *material`: 解析結果の格納先。
/// - `char *error`: エラーメッセージの格納先。
/// - `size_t error_size`: エラー格納先のバイト数。
///
/// ### Returns
/// - `true`: 解析に成功した。
/// - `false`: 定義または引数が不正だった。
bool material_parse(const char *source,
		    material_t *material,
		    char *error,
		    size_t error_size);
/// ファイルからテクスチャを除くマテリアル値を読み込む。
///
/// ### Args
/// - `const char *path`: 読み込む定義ファイルのパス。
/// - `material_t *material`: 読み込み結果の格納先。
/// - `char *error`: エラーメッセージの格納先。
/// - `size_t error_size`: エラー格納先のバイト数。
///
/// ### Returns
/// - `true`: 読み込みに成功した。
/// - `false`: ファイルまたは定義が不正だった。
bool material_load(const char *path,
		   material_t *material,
		   char *error,
		   size_t error_size);

#endif
