/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_ENTITY_PROP_BREAKABLE_H
#define VOLUME_ENTITY_PROP_BREAKABLE_H

#include "entity/health.h"
#include "entity/prop.h"

typedef struct prop_breakable {
	prop_t prop;
	health_t health;
} prop_breakable_t;

/// エンティティを`prop_breakable`として取得する。
///
/// ### Args
/// - `entity_t *entity`: 変換するエンティティ。
///
/// ### Returns
/// - `prop_breakable_t *`: 対応する壊せるprop。型が異なる場合は`NULL`。
prop_breakable_t *prop_breakable_from_entity(entity_t *entity);

/// 壊せるpropの現在ヘルスを取得する。
///
/// ### Args
/// - `const prop_breakable_t *prop`: 対象のprop。
///
/// ### Returns
/// - `float`: 現在ヘルス。引数が`NULL`の場合は`0`。
float prop_breakable_get_health(const prop_breakable_t *prop);

/// `prop_breakable`エンティティクラスを登録する。
///
/// ### Returns
/// - `true`: 登録に成功した。
/// - `false`: 登録に失敗した。
bool prop_breakable_register(void);

#endif
