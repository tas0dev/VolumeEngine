/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_EXAMPLE_NPC_ENEMY_H
#define VOLUME_EXAMPLE_NPC_ENEMY_H

#include "entity/entity.h"
#include <stdbool.h>

typedef struct sandbox_npc_enemy sandbox_npc_enemy_t;

typedef enum sandbox_npc_enemy_state {
	SANDBOX_NPC_ENEMY_IDLE,
	SANDBOX_NPC_ENEMY_CHASE,
	SANDBOX_NPC_ENEMY_ATTACK,
} sandbox_npc_enemy_state_t;

/// sandbox用の敵NPCエンティティクラスを登録する。
///
/// ### Returns
/// - `true`: 登録に成功した。
/// - `false`: 重複またはメモリ不足により登録できなかった。
bool sandbox_npc_enemy_register(void);

/// エンティティをsandbox用の敵NPCとして取得する。
///
/// ### Args
/// - `entity_t *entity`: 変換するエンティティ。
///
/// ### Returns
/// - `sandbox_npc_enemy_t *`: 対応する敵NPC。型が異なる場合は`NULL`。
sandbox_npc_enemy_t *sandbox_npc_enemy_from_entity(entity_t *entity);

/// 敵NPCの現在状態を取得する。
///
/// ### Args
/// - `const sandbox_npc_enemy_t *enemy`: 対象の敵NPC。
///
/// ### Returns
/// - `sandbox_npc_enemy_state_t`: 現在のAI状態。
sandbox_npc_enemy_state_t
sandbox_npc_enemy_get_state(const sandbox_npc_enemy_t *enemy);

/// 敵NPCの現在ヘルスを取得する。
///
/// ### Args
/// - `const sandbox_npc_enemy_t *enemy`: 対象の敵NPC。
///
/// ### Returns
/// - `float`: 現在ヘルス。引数が`NULL`の場合は`0`。
float sandbox_npc_enemy_get_health(const sandbox_npc_enemy_t *enemy);

#endif
