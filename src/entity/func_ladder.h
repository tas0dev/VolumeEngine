/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_ENTITY_FUNC_LADDER_H
#define VOLUME_ENTITY_FUNC_LADDER_H

#include "entity/entity.h"

typedef struct func_ladder func_ladder_t;

/// `func_ladder`エンティティクラスを登録する。
///
/// ### Returns
/// - `true`: 登録に成功した。
/// - `false`: 登録に失敗した。
bool func_ladder_register(void);
/// エンティティを`func_ladder`として取得する。
///
/// ### Args
/// - `entity_t *entity`: 変換するエンティティ。
///
/// ### Returns
/// - `func_ladder_t *`: 対応する梯子。型が異なる場合は`NULL`。
func_ladder_t *func_ladder_from_entity(entity_t *entity);
/// 読み取り専用エンティティを`func_ladder`として取得する。
///
/// ### Args
/// - `const entity_t *entity`: 変換するエンティティ。
///
/// ### Returns
/// - `const func_ladder_t *`: 対応する梯子。型が異なる場合は`NULL`。
const func_ladder_t *func_ladder_from_const_entity(const entity_t *entity);
/// 梯子が所有する基底エンティティを取得する。
///
/// ### Args
/// - `func_ladder_t *ladder`: 対象の梯子。
///
/// ### Returns
/// - `entity_t *`: 基底エンティティ。引数が`NULL`の場合は`NULL`。
entity_t *func_ladder_get_entity(func_ladder_t *ladder);
/// 梯子面の外向き法線を取得する。
///
/// ### Args
/// - `const func_ladder_t *ladder`: 対象の梯子。
///
/// ### Returns
/// - `vec3_t`: 正規化された梯子面の法線。
vec3_t func_ladder_get_normal(const func_ladder_t *ladder);

#endif
