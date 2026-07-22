/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_ENTITY_PROP_STATIC_H
#define VOLUME_ENTITY_PROP_STATIC_H

#include "entity/prop.h"

typedef prop_properties_t prop_static_properties_t;
typedef prop_t prop_static_t;

/// `prop_static`用プロパティの既定値を作成する。
///
/// ### Returns
/// - `prop_static_properties_t`: 既定値で初期化されたプロパティ。
prop_static_properties_t prop_static_properties_create(void);
/// 静的propを作成する。
///
/// ### Args
/// - `entity_id_t id`: 割り当てる一意なID。
/// - `const prop_static_properties_t *properties`: 作成に使用するプロパティ。
///
/// ### Returns
/// - `prop_static_t *`: 作成したprop。失敗時は`NULL`。
prop_static_t *prop_static_create(entity_id_t id,
				  const prop_static_properties_t *properties);
/// 静的propを破棄する。
///
/// ### Args
/// - `prop_static_t *prop`: 破棄するprop。
void prop_static_destroy(prop_static_t *prop);
/// 静的propの基底エンティティを取得する。
///
/// ### Args
/// - `prop_static_t *prop`: 対象のprop。
///
/// ### Returns
/// - `entity_t *`: 基底エンティティ。引数が`NULL`の場合は`NULL`。
entity_t *prop_static_get_entity(prop_static_t *prop);
/// 静的propの読み取り専用基底エンティティを取得する。
///
/// ### Args
/// - `const prop_static_t *prop`: 対象のprop。
///
/// ### Returns
/// - `const entity_t *`: 基底エンティティ。引数が`NULL`の場合は`NULL`。
const entity_t *prop_static_get_const_entity(const prop_static_t *prop);
/// エンティティを静的propとして取得する。
///
/// ### Args
/// - `entity_t *entity`: 変換するエンティティ。
///
/// ### Returns
/// - `prop_static_t *`: 対応するprop。型が異なる場合は`NULL`。
prop_static_t *prop_static_from_entity(entity_t *entity);
/// 読み取り専用エンティティを静的propとして取得する。
///
/// ### Args
/// - `const entity_t *entity`: 変換するエンティティ。
///
/// ### Returns
/// - `const prop_static_t *`: 対応するprop。型が異なる場合は`NULL`。
const prop_static_t *prop_static_from_const_entity(const entity_t *entity);
/// `prop_static`エンティティクラスを登録する。
///
/// ### Returns
/// - `true`: 登録に成功した。
/// - `false`: 登録に失敗した。
bool prop_static_register(void);

#endif
