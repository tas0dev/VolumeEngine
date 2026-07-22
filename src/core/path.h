/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_CORE_PATH_H
#define VOLUME_CORE_PATH_H

/// 実行ファイルのディレクトリを基準にパスを作成する。
///
/// ### Args
/// - `const char *path`: 基準ディレクトリへ連結する相対パス。
///
/// ### Returns
/// - `char *`: 動的確保されたパス。失敗時は`NULL`。
char *path_from_executable(const char *path);

#endif
