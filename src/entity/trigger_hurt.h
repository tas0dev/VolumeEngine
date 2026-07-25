/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_ENTITY_TRIGGER_HURT_H
#define VOLUME_ENTITY_TRIGGER_HURT_H

#include <stdbool.h>

/// Source互換の`trigger_hurt`エンティティクラスを登録する。
///
/// `damage`を毎秒ダメージとして扱い、0.5秒ごとに適用する。
/// `damagecap`、`damagetype`、`damagemodel`、`StartDisabled`、
/// `SetDamage`入力、`OnHurtPlayer`出力に対応する。
///
/// ### Returns
/// - `true`: 登録に成功した。
/// - `false`: 登録に失敗した。
bool trigger_hurt_register(void);

#endif
