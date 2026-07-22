/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_MAP_MAP_H
#define VOLUME_MAP_MAP_H

#include "math/vec3.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct map map_t;
typedef struct map_entity map_entity_t;

/// 文字列からVolumeEngineマップを解析する。
///
/// ### Args
/// - `const char *source`: 解析するマップ文字列。
/// - `char *error`: エラーメッセージの格納先。
/// - `size_t error_size`: エラー格納先のバイト数。
///
/// ### Returns
/// - `map_t *`: 解析したマップ。失敗時は`NULL`。
map_t *map_parse(const char *source, char *error, size_t error_size);
/// ファイルからVolumeEngineマップを読み込む。
///
/// ### Args
/// - `const char *path`: 読み込むマップファイルのパス。
/// - `char *error`: エラーメッセージの格納先。
/// - `size_t error_size`: エラー格納先のバイト数。
///
/// ### Returns
/// - `map_t *`: 読み込んだマップ。失敗時は`NULL`。
map_t *map_load(const char *path, char *error, size_t error_size);
/// マップと解析済みデータを破棄する。
///
/// ### Args
/// - `map_t *map`: 破棄するマップ。
void map_destroy(map_t *map);
/// マップのworldブロックを取得する。
///
/// ### Args
/// - `const map_t *map`: 対象のマップ。
///
/// ### Returns
/// - `const map_entity_t *`: worldブロック。存在しない場合は`NULL`。
const map_entity_t *map_get_world(const map_t *map);
/// マップに含まれる通常エンティティ数を取得する。
///
/// ### Args
/// - `const map_t *map`: 対象のマップ。
///
/// ### Returns
/// - `size_t`: worldブロックを除くエンティティ数。
size_t map_get_entity_count(const map_t *map);
/// 指定添字のマップエンティティを取得する。
///
/// ### Args
/// - `const map_t *map`: 対象のマップ。
/// - `size_t index`: 取得するエンティティの添字。
///
/// ### Returns
/// - `const map_entity_t *`: 対応するエンティティ。範囲外の場合は`NULL`。
const map_entity_t *map_get_entity(const map_t *map, size_t index);
/// マップエンティティから指定キーの文字列値を取得する。
///
/// ### Args
/// - `const map_entity_t *entity`: 対象のマップエンティティ。
/// - `const char *key`: 取得するプロパティ名。
///
/// ### Returns
/// - `const char *`: プロパティ値。存在しない場合は`NULL`。
const char *map_entity_get_property(const map_entity_t *entity,
				    const char *key);
/// マップエンティティのプロパティ数を取得する。
///
/// ### Args
/// - `const map_entity_t *entity`: 対象のマップエンティティ。
///
/// ### Returns
/// - `size_t`: プロパティ数。
size_t map_entity_get_property_count(const map_entity_t *entity);
/// 指定添字のプロパティ名と値を取得する。
///
/// ### Args
/// - `const map_entity_t *entity`: 対象のマップエンティティ。
/// - `size_t index`: 取得するプロパティの添字。
/// - `const char **key`: キー文字列ポインタの格納先。
/// - `const char **value`: 値文字列ポインタの格納先。
///
/// ### Returns
/// - `true`: プロパティを取得した。
/// - `false`: 添字または引数が不正だった。
bool map_entity_get_property_at(const map_entity_t *entity,
				size_t index,
				const char **key,
				const char **value);
/// 指定プロパティを3次元ベクトルとして取得する。
///
/// ### Args
/// - `const map_entity_t *entity`: 対象のマップエンティティ。
/// - `const char *key`: 取得するプロパティ名。
/// - `vec3_t *value`: 解析結果の格納先。
///
/// ### Returns
/// - `true`: プロパティを正常に解析した。
/// - `false`: プロパティがない、または形式が不正だった。
bool map_entity_get_vec3(const map_entity_t *entity,
			 const char *key,
			 vec3_t *value);
/// 指定プロパティを真偽値として取得する。
///
/// ### Args
/// - `const map_entity_t *entity`: 対象のマップエンティティ。
/// - `const char *key`: 取得するプロパティ名。
/// - `bool *value`: 解析結果の格納先。
///
/// ### Returns
/// - `true`: プロパティを正常に解析した。
/// - `false`: プロパティがない、または形式が不正だった。
bool map_entity_get_bool(const map_entity_t *entity,
			 const char *key,
			 bool *value);
/// 指定プロパティを浮動小数点数として取得する。
///
/// ### Args
/// - `const map_entity_t *entity`: 対象のマップエンティティ。
/// - `const char *key`: 取得するプロパティ名。
/// - `float *value`: 解析結果の格納先。
///
/// ### Returns
/// - `true`: プロパティを正常に解析した。
/// - `false`: プロパティがない、または形式が不正だった。
bool map_entity_get_float(const map_entity_t *entity,
			  const char *key,
			  float *value);

#endif
