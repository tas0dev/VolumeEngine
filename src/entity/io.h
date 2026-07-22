/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_ENTITY_IO_H
#define VOLUME_ENTITY_IO_H

#include "core/types.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct entity_input_context {
	world_t *world;
	entity_t *activator;
	entity_t *caller;
	const char *parameter;
} entity_input_context_t;

typedef struct entity_output_connection {
	char *output_name;
	char *target_name;
	char *input_name;
	char *parameter;
	float delay;
	int remaining_fires;
} entity_output_connection_t;

/// エンティティへ出力接続を追加する。
///
/// ### Args
/// - `entity_t *entity`: 接続を所有するエンティティ。
/// - `const char *output_name`: 発火元の出力名。
/// - `const char *target_name`: 入力を送るターゲット名。
/// - `const char *input_name`: 呼び出す入力名。
/// - `const char *parameter`: 入力へ渡す文字列引数。
/// - `float delay`: 発火までの遅延秒数。
/// - `int maximum_fires`: 最大発火回数。負数は無制限。
///
/// ### Returns
/// - `true`: 接続を追加した。
/// - `false`: 値が不正またはメモリ不足で追加できなかった。
bool entity_add_output(entity_t *entity,
		       const char *output_name,
		       const char *target_name,
		       const char *input_name,
		       const char *parameter,
		       float delay,
		       int maximum_fires);
/// Source形式の文字列から出力接続を追加する。
///
/// ### Args
/// - `entity_t *entity`: 接続を所有するエンティティ。
/// - `const char *output_name`: 発火元の出力名。
/// - `const char *value`: 接続定義文字列。
/// - `char *error`: エラーメッセージの格納先。
/// - `size_t error_size`: エラー格納先のバイト数。
///
/// ### Returns
/// - `true`: 解析と追加に成功した。
/// - `false`: 定義が不正または追加に失敗した。
bool entity_add_output_from_string(entity_t *entity,
				   const char *output_name,
				   const char *value,
				   char *error,
				   size_t error_size);
/// エンティティが保持する出力接続数を取得する。
///
/// ### Args
/// - `const entity_t *entity`: 対象のエンティティ。
///
/// ### Returns
/// - `size_t`: 出力接続数。
size_t entity_get_output_count(const entity_t *entity);
/// 指定位置の出力接続を取得する。
///
/// ### Args
/// - `const entity_t *entity`: 対象のエンティティ。
/// - `size_t index`: 取得する接続の添字。
///
/// ### Returns
/// - `const entity_output_connection_t *`: 接続。範囲外の場合は`NULL`。
const entity_output_connection_t *entity_get_output(const entity_t *entity,
						    size_t index);
/// エンティティへ名前付き入力を送る。
///
/// ### Args
/// - `entity_t *entity`: 入力を受け取るエンティティ。
/// - `const char *input_name`: 入力名。
/// - `const entity_input_context_t *context`: 呼び出しコンテキスト。
///
/// ### Returns
/// - `true`: 入力を受理した。
/// - `false`: 入力が未対応または引数が不正だった。
bool entity_accept_input(entity_t *entity,
			 const char *input_name,
			 const entity_input_context_t *context);
/// エンティティが所有するI/O接続を破棄する。
///
/// ### Args
/// - `entity_t *entity`: 対象のエンティティ。
void entity_io_destroy(entity_t *entity);

#endif
