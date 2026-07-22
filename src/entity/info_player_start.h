/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_ENTITY_INFO_PLAYER_START_H
#define VOLUME_ENTITY_INFO_PLAYER_START_H

#include "entity/entity.h"

typedef struct info_player_start {
	entity_t entity;
} info_player_start_t;

/// `info_player_start`エンティティクラスを登録する。
///
/// ### Returns
/// - `true`: 登録に成功した。
/// - `false`: 登録に失敗した。
bool info_player_start_register(void);
/// エンティティをプレイヤースポーン地点として取得する。
///
/// ### Args
/// - `entity_t *entity`: 変換するエンティティ。
///
/// ### Returns
/// - `info_player_start_t *`: 対応するスポーン地点。型が異なる場合は`NULL`。
info_player_start_t *info_player_start_from_entity(entity_t *entity);
/// 読み取り専用エンティティをプレイヤースポーン地点として取得する。
///
/// ### Args
/// - `const entity_t *entity`: 変換するエンティティ。
///
/// ### Returns
/// - `const info_player_start_t *`: 対応するスポーン地点。型が異なる場合は`NULL`。
const info_player_start_t *
info_player_start_from_const_entity(const entity_t *entity);

#endif
