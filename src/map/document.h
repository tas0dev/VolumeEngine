/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_MAP_DOCUMENT_H
#define VOLUME_MAP_DOCUMENT_H

#include "map/map.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct map_document map_document_t;

/// 空の編集用Map Documentを作成する。
///
/// ### Returns
/// - `map_document_t *`: 作成したDocument。失敗時は`NULL`。
map_document_t *map_document_create(void);

/// `.volmap`ファイルから編集用Map Documentを読み込む。
///
/// ### Args
/// - `const char *path`: 読み込むマップパス。
/// - `char *error`: エラーメッセージの格納先。
/// - `size_t error_size`: エラー格納先のバイト数。
///
/// ### Returns
/// - `map_document_t *`: 読み込んだDocument。失敗時は`NULL`。
map_document_t *
map_document_load(const char *path, char *error, size_t error_size);

/// Map DocumentとUndo/Redo履歴を破棄する。
///
/// ### Args
/// - `map_document_t *document`: 破棄するDocument。
void map_document_destroy(map_document_t *document);

/// Documentが保持する読み取り専用マップを取得する。
///
/// ### Args
/// - `const map_document_t *document`: 対象のDocument。
///
/// ### Returns
/// - `const map_t *`: 現在のマップ。引数が`NULL`の場合は`NULL`。
const map_t *map_document_get_map(const map_document_t *document);

/// 未保存の変更があるか調べる。
///
/// ### Args
/// - `const map_document_t *document`: 対象のDocument。
///
/// ### Returns
/// - `true`: 未保存の変更がある。
/// - `false`: 保存済み、または引数が`NULL`。
bool map_document_is_dirty(const map_document_t *document);

/// 複数の編集を単一のUndo履歴へまとめるTransactionを開始する。
///
/// ### Args
/// - `map_document_t *document`: 編集するDocument。
///
/// ### Returns
/// - `true`: Transactionを開始した。
/// - `false`: 既に実行中、または状態保存に失敗した。
bool map_document_begin_transaction(map_document_t *document);

/// 実行中のTransactionを確定して単一のUndo履歴へ追加する。
///
/// ### Args
/// - `map_document_t *document`: 編集するDocument。
///
/// ### Returns
/// - `true`: 確定した、または変更がなかった。
/// - `false`: Transactionがない、または履歴保存に失敗した。
bool map_document_end_transaction(map_document_t *document);

/// 実行中のTransactionを破棄して開始前へ戻す。
///
/// ### Args
/// - `map_document_t *document`: 編集するDocument。
///
/// ### Returns
/// - `true`: 開始前へ戻した。
/// - `false`: Transactionがない、または復元に失敗した。
bool map_document_cancel_transaction(map_document_t *document);

/// worldspawnの最初に一致するPropertyを変更する。
///
/// ### Args
/// - `map_document_t *document`: 編集するDocument。
/// - `const char *key`: Property名。
/// - `const char *value`: 新しい値。
///
/// ### Returns
/// - `true`: 変更した、または同じ値だった。
/// - `false`: 引数不正またはメモリ不足。
bool map_document_set_world_property(map_document_t *document,
				     const char *key,
				     const char *value);

/// 指定エンティティの最初に一致するPropertyを変更する。
///
/// ### Args
/// - `map_document_t *document`: 編集するDocument。
/// - `size_t entity_index`: エンティティの添字。
/// - `const char *key`: Property名。
/// - `const char *value`: 新しい値。
///
/// ### Returns
/// - `true`: 変更した、または同じ値だった。
/// - `false`: 引数不正またはメモリ不足。
bool map_document_set_entity_property(map_document_t *document,
				      size_t entity_index,
				      const char *key,
				      const char *value);

/// 指定エンティティへ同名を許可してPropertyを追加する。
///
/// ### Args
/// - `map_document_t *document`: 編集するDocument。
/// - `size_t entity_index`: エンティティの添字。
/// - `const char *key`: Property名。
/// - `const char *value`: 値。
///
/// ### Returns
/// - `true`: 追加した。
/// - `false`: 引数不正またはメモリ不足。
bool map_document_add_entity_property(map_document_t *document,
				      size_t entity_index,
				      const char *key,
				      const char *value);

/// 指定クラスのエンティティを追加する。
///
/// ### Args
/// - `map_document_t *document`: 編集するDocument。
/// - `const char *classname`: 追加するクラス名。
/// - `size_t *entity_index`: 追加位置の格納先。不要なら`NULL`。
///
/// ### Returns
/// - `true`: 追加した。
/// - `false`: 引数不正またはメモリ不足。
bool map_document_add_entity(map_document_t *document,
			     const char *classname,
			     size_t *entity_index);

/// 指定添字のエンティティを削除する。
///
/// ### Args
/// - `map_document_t *document`: 編集するDocument。
/// - `size_t entity_index`: 削除するエンティティの添字。
///
/// ### Returns
/// - `true`: 削除した。
/// - `false`: 添字不正または履歴保存に失敗した。
bool map_document_remove_entity(map_document_t *document, size_t entity_index);

/// 直前の編集を取り消す。
///
/// ### Args
/// - `map_document_t *document`: 対象のDocument。
///
/// ### Returns
/// - `true`: Undoを適用した。
/// - `false`: 履歴がない、または復元に失敗した。
bool map_document_undo(map_document_t *document);

/// 直前に取り消した編集をやり直す。
///
/// ### Args
/// - `map_document_t *document`: 対象のDocument。
///
/// ### Returns
/// - `true`: Redoを適用した。
/// - `false`: 履歴がない、または復元に失敗した。
bool map_document_redo(map_document_t *document);

/// Undo可能な編集があるか調べる。
///
/// ### Args
/// - `const map_document_t *document`: 対象のDocument。
///
/// ### Returns
/// - `true`: Undo可能。
/// - `false`: 履歴がない。
bool map_document_can_undo(const map_document_t *document);

/// Redo可能な編集があるか調べる。
///
/// ### Args
/// - `const map_document_t *document`: 対象のDocument。
///
/// ### Returns
/// - `true`: Redo可能。
/// - `false`: 履歴がない。
bool map_document_can_redo(const map_document_t *document);

/// Documentを`.volmap`へ保存して現在状態を保存済みにする。
///
/// `path`が`NULL`の場合は直前の読込・保存パスを使用する。
///
/// ### Args
/// - `map_document_t *document`: 保存するDocument。
/// - `const char *path`: 保存先。既存パスを使う場合は`NULL`。
/// - `char *error`: エラーメッセージの格納先。
/// - `size_t error_size`: エラー格納先のバイト数。
///
/// ### Returns
/// - `true`: 保存に成功した。
/// - `false`: パスがない、または書き込みに失敗した。
bool map_document_save(map_document_t *document,
		       const char *path,
		       char *error,
		       size_t error_size);

#endif
