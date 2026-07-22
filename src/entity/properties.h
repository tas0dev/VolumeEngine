/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_ENTITY_PROPERTIES_H
#define VOLUME_ENTITY_PROPERTIES_H

#include "math/vec3.h"
#include "scene/transform.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct entity_property_source {
	const void *context;
	/// 任意の読み取り元からプロパティ値を取得する。
	///
	/// ### Args
	/// - `const void *context`: 読み取り元固有のコンテキスト。
	/// - `const char *key`: 取得するプロパティ名。
	///
	/// ### Returns
	/// - `const char *`: プロパティ値。存在しない場合は`NULL`。
	const char *(*get)(const void *context, const char *key);
} entity_property_source_t;

typedef struct entity_properties {
	const char *targetname;
	transform_t transform;
} entity_properties_t;

/// エンティティ共通プロパティの既定値を作成する。
///
/// ### Returns
/// - `entity_properties_t`: 既定値で初期化されたプロパティ。
entity_properties_t entity_properties_create(void);
/// プロパティソースから指定キーの文字列値を取得する。
///
/// ### Args
/// - `const entity_property_source_t *source`: 読み取り元。
/// - `const char *key`: 取得するキー。
///
/// ### Returns
/// - `const char *`: 対応する値。存在しない場合は`NULL`。
const char *entity_property_get(const entity_property_source_t *source,
				const char *key);
/// 文字列を浮動小数点数として解析する。
///
/// ### Args
/// - `const char *text`: 解析する文字列。
/// - `float *value`: 解析結果の格納先。
///
/// ### Returns
/// - `true`: 文字列全体を正常に解析した。
/// - `false`: 形式または引数が不正だった。
bool entity_property_parse_float(const char *text, float *value);
/// 文字列を3次元ベクトルとして解析する。
///
/// ### Args
/// - `const char *text`: 解析する文字列。
/// - `vec3_t *value`: 解析結果の格納先。
///
/// ### Returns
/// - `true`: 3要素を正常に解析した。
/// - `false`: 形式または引数が不正だった。
bool entity_property_parse_vec3(const char *text, vec3_t *value);
/// 文字列を真偽値として解析する。
///
/// ### Args
/// - `const char *text`: 解析する文字列。
/// - `bool *value`: 解析結果の格納先。
///
/// ### Returns
/// - `true`: 真偽値を正常に解析した。
/// - `false`: 形式または引数が不正だった。
bool entity_property_parse_bool(const char *text, bool *value);
/// ソースからエンティティ共通プロパティを読み込む。
///
/// ### Args
/// - `entity_properties_t *properties`: 読み込み先。
/// - `const entity_property_source_t *source`: プロパティの読み取り元。
/// - `char *error`: エラーメッセージの格納先。
/// - `size_t error_size`: エラー格納先のバイト数。
///
/// ### Returns
/// - `true`: 読み込みに成功した。
/// - `false`: プロパティが不正だった。
bool entity_properties_load(entity_properties_t *properties,
			    const entity_property_source_t *source,
			    char *error,
			    size_t error_size);

#endif
