/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_CORE_LOG_H
#define VOLUME_CORE_LOG_H

/// 情報レベルの整形済みログを出力する。
///
/// ### Args
/// - `const char *fmt`: `printf`形式のフォーマット文字列。
/// - `...`: フォーマットへ渡す値。
void log_info(const char *fmt, ...);
/// エラーレベルの整形済みログを出力する。
///
/// ### Args
/// - `const char *fmt`: `printf`形式のフォーマット文字列。
/// - `...`: フォーマットへ渡す値。
void log_error(const char *fmt, ...);

#endif // VOLUME_CORE_LOG_H
