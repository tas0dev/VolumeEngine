/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_MAP_KEYVALUES_H
#define VOLUME_MAP_KEYVALUES_H

#include <stdbool.h>
#include <stddef.h>

typedef struct keyvalues_document keyvalues_document_t;
typedef struct keyvalues_node keyvalues_node_t;

/// 空のKeyValuesドキュメントを作成する。
///
/// ### Returns
/// - `keyvalues_document_t *`: 作成したドキュメント。失敗時は`NULL`。
keyvalues_document_t *keyvalues_create(void);

/// KeyValues文字列を解析する。
///
/// ### Args
/// - `const char *source`: 解析する文字列。
/// - `char *error`: エラーメッセージの格納先。
/// - `size_t error_size`: エラー格納先のバイト数。
///
/// ### Returns
/// - `keyvalues_document_t *`: 解析したドキュメント。失敗時は`NULL`。
keyvalues_document_t *
keyvalues_parse(const char *source, char *error, size_t error_size);
/// KeyValuesファイルを読み込んで解析する。
///
/// ### Args
/// - `const char *path`: 読み込むファイルのパス。
/// - `char *error`: エラーメッセージの格納先。
/// - `size_t error_size`: エラー格納先のバイト数。
///
/// ### Returns
/// - `keyvalues_document_t *`: 読み込んだドキュメント。失敗時は`NULL`。
keyvalues_document_t *
keyvalues_load(const char *path, char *error, size_t error_size);
/// KeyValuesドキュメントを破棄する。
///
/// ### Args
/// - `keyvalues_document_t *document`: 破棄するドキュメント。
void keyvalues_destroy(keyvalues_document_t *document);
/// ドキュメントのルートノードを取得する。
///
/// ### Args
/// - `const keyvalues_document_t *document`: 対象のドキュメント。
///
/// ### Returns
/// - `const keyvalues_node_t *`: ルートノード。引数が`NULL`の場合は`NULL`。
const keyvalues_node_t *
keyvalues_get_root(const keyvalues_document_t *document);
/// ドキュメントの編集可能なルートノードを取得する。
///
/// ### Args
/// - `keyvalues_document_t *document`: 対象のドキュメント。
///
/// ### Returns
/// - `keyvalues_node_t *`: ルートノード。引数が`NULL`の場合は`NULL`。
keyvalues_node_t *keyvalues_get_mutable_root(keyvalues_document_t *document);

/// キーと値からKeyValuesノードを作成する。
///
/// `value`が`NULL`の場合はブロックノードを作成する。
///
/// ### Args
/// - `const char *key`: ノードのキー。
/// - `const char *value`: 値。ブロックの場合は`NULL`。
///
/// ### Returns
/// - `keyvalues_node_t *`: 作成したノード。失敗時は`NULL`。
keyvalues_node_t *keyvalues_node_create(const char *key, const char *value);

/// ノードとすべての子ノードを破棄する。
///
/// ### Args
/// - `keyvalues_node_t *node`: 破棄する未所有ノード。
void keyvalues_node_destroy(keyvalues_node_t *node);

/// ブロック末尾へ子ノードを追加して所有権を移す。
///
/// ### Args
/// - `keyvalues_node_t *parent`: 追加先のブロック。
/// - `keyvalues_node_t *child`: 追加する未所有ノード。
///
/// ### Returns
/// - `true`: 追加に成功した。
/// - `false`: 引数不正またはメモリ不足。
bool keyvalues_node_add_child(keyvalues_node_t *parent,
			      keyvalues_node_t *child);

/// 指定位置の子ノードを削除して破棄する。
///
/// ### Args
/// - `keyvalues_node_t *parent`: 対象のブロック。
/// - `size_t index`: 削除する子ノードの添字。
///
/// ### Returns
/// - `true`: 削除した。
/// - `false`: 添字または引数が不正だった。
bool keyvalues_node_remove_child(keyvalues_node_t *parent, size_t index);

/// 値ノードの文字列値を置き換える。
///
/// ### Args
/// - `keyvalues_node_t *node`: 対象の値ノード。
/// - `const char *value`: 新しい値。
///
/// ### Returns
/// - `true`: 変更した。
/// - `false`: ブロック、引数不正、またはメモリ不足。
bool keyvalues_node_set_value(keyvalues_node_t *node, const char *value);
/// ノードのキーを取得する。
///
/// ### Args
/// - `const keyvalues_node_t *node`: 対象のノード。
///
/// ### Returns
/// - `const char *`: キー文字列。取得できない場合は`NULL`。
const char *keyvalues_node_get_key(const keyvalues_node_t *node);
/// 値ノードの文字列値を取得する。
///
/// ### Args
/// - `const keyvalues_node_t *node`: 対象のノード。
///
/// ### Returns
/// - `const char *`: 値文字列。ブロックまたは無効な場合は`NULL`。
const char *keyvalues_node_get_value(const keyvalues_node_t *node);
/// ノードが子要素を持つブロックか調べる。
///
/// ### Args
/// - `const keyvalues_node_t *node`: 対象のノード。
///
/// ### Returns
/// - `true`: ブロックノードである。
/// - `false`: 値ノードまたは無効な引数である。
bool keyvalues_node_is_block(const keyvalues_node_t *node);
/// ブロックノードの子要素数を取得する。
///
/// ### Args
/// - `const keyvalues_node_t *node`: 対象のノード。
///
/// ### Returns
/// - `size_t`: 子要素数。
size_t keyvalues_node_get_child_count(const keyvalues_node_t *node);
/// 指定添字の子ノードを取得する。
///
/// ### Args
/// - `const keyvalues_node_t *node`: 対象のブロックノード。
/// - `size_t index`: 取得する子要素の添字。
///
/// ### Returns
/// - `const keyvalues_node_t *`: 子ノード。範囲外の場合は`NULL`。
const keyvalues_node_t *keyvalues_node_get_child(const keyvalues_node_t *node,
						 size_t index);
/// 指定添字の編集可能な子ノードを取得する。
///
/// ### Args
/// - `keyvalues_node_t *node`: 対象のブロックノード。
/// - `size_t index`: 取得する子要素の添字。
///
/// ### Returns
/// - `keyvalues_node_t *`: 子ノード。範囲外の場合は`NULL`。
keyvalues_node_t *keyvalues_node_get_mutable_child(keyvalues_node_t *node,
						   size_t index);
/// 指定キーに一致する最初の子ノードを検索する。
///
/// ### Args
/// - `const keyvalues_node_t *node`: 検索するブロックノード。
/// - `const char *key`: 検索するキー。
///
/// ### Returns
/// - `const keyvalues_node_t *`: 見つかった子ノード。存在しない場合は`NULL`。
const keyvalues_node_t *keyvalues_node_find_child(const keyvalues_node_t *node,
						  const char *key);

/// KeyValuesドキュメントを正規化された文字列へ変換する。
///
/// ### Args
/// - `const keyvalues_document_t *document`: 対象のドキュメント。
///
/// ### Returns
/// - `char *`: `free`で解放する文字列。失敗時は`NULL`。
char *keyvalues_serialize(const keyvalues_document_t *document);

/// KeyValuesドキュメントをファイルへ保存する。
///
/// ### Args
/// - `const keyvalues_document_t *document`: 保存するドキュメント。
/// - `const char *path`: 保存先パス。
/// - `char *error`: エラーメッセージの格納先。
/// - `size_t error_size`: エラー格納先のバイト数。
///
/// ### Returns
/// - `true`: 保存に成功した。
/// - `false`: 変換または書き込みに失敗した。
bool keyvalues_save(const keyvalues_document_t *document,
		    const char *path,
		    char *error,
		    size_t error_size);

#endif
