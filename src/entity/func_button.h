/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_ENTITY_FUNC_BUTTON_H
#define VOLUME_ENTITY_FUNC_BUTTON_H

#include "entity/mover.h"
#include "entity/prop.h"

typedef enum func_button_state {
	FUNC_BUTTON_IDLE = MOVER_AT_START,
	FUNC_BUTTON_PRESSING = MOVER_MOVING_TO_END,
	FUNC_BUTTON_PRESSED = MOVER_AT_END,
	FUNC_BUTTON_RELEASING = MOVER_MOVING_TO_START,
} func_button_state_t;

typedef struct func_button {
	prop_t prop;
	mover_t mover;
	bool locked;
	bool enabled;
} func_button_t;

/// `func_button`エンティティクラスを登録する。
///
/// ### Returns
/// - `true`: 登録に成功した。
/// - `false`: 登録に失敗した。
bool func_button_register(void);
/// エンティティを`func_button`として取得する。
///
/// ### Args
/// - `entity_t *entity`: 変換するエンティティ。
///
/// ### Returns
/// - `func_button_t *`: 対応するボタン。型が異なる場合は`NULL`。
func_button_t *func_button_from_entity(entity_t *entity);
/// 読み取り専用エンティティを`func_button`として取得する。
///
/// ### Args
/// - `const entity_t *entity`: 変換するエンティティ。
///
/// ### Returns
/// - `const func_button_t *`: 対応するボタン。型が異なる場合は`NULL`。
const func_button_t *func_button_from_const_entity(const entity_t *entity);
/// ボタンが押し込まれた状態か調べる。
///
/// ### Args
/// - `const func_button_t *button`: 対象のボタン。
///
/// ### Returns
/// - `true`: 押し込まれている。
/// - `false`: 解放されている。
bool func_button_is_pressed(const func_button_t *button);
/// ボタンが使用可能か調べる。
///
/// ### Args
/// - `const func_button_t *button`: 対象のボタン。
///
/// ### Returns
/// - `true`: 使用可能である。
/// - `false`: 無効化されている。
bool func_button_is_enabled(const func_button_t *button);
/// ボタンの現在状態を取得する。
///
/// ### Args
/// - `const func_button_t *button`: 対象のボタン。
///
/// ### Returns
/// - `func_button_state_t`: 現在の押下状態。
func_button_state_t func_button_get_state(const func_button_t *button);

#endif
