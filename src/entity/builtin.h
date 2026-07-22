/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_ENTITY_BUILTIN_H
#define VOLUME_ENTITY_BUILTIN_H

#include <stdbool.h>

/// エンジン組み込みの全エンティティクラスを登録する。
///
/// ### Returns
/// - `true`: 全クラスの登録に成功した。
/// - `false`: いずれかのクラス登録に失敗した。
bool entity_register_builtin_classes(void);

#endif
