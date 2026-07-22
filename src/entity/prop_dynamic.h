/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_ENTITY_PROP_DYNAMIC_H
#define VOLUME_ENTITY_PROP_DYNAMIC_H

#include "entity/prop.h"

typedef prop_properties_t prop_dynamic_properties_t;
typedef prop_t prop_dynamic_t;

/// `prop_dynamic`用プロパティの既定値を作成する。
///
/// ### Returns
/// - `prop_dynamic_properties_t`: 既定値で初期化されたプロパティ。
prop_dynamic_properties_t prop_dynamic_properties_create(void);
/// 動的propを作成する。
///
/// ### Args
/// - `entity_id_t id`: 割り当てる一意なID。
/// - `const prop_dynamic_properties_t *properties`: 作成に使用するプロパティ。
///
/// ### Returns
/// - `prop_dynamic_t *`: 作成したprop。失敗時は`NULL`。
prop_dynamic_t *
prop_dynamic_create(entity_id_t id,
		    const prop_dynamic_properties_t *properties);
/// 動的propを破棄する。
///
/// ### Args
/// - `prop_dynamic_t *prop`: 破棄するprop。
void prop_dynamic_destroy(prop_dynamic_t *prop);
/// 動的propの基底エンティティを取得する。
///
/// ### Args
/// - `prop_dynamic_t *prop`: 対象のprop。
///
/// ### Returns
/// - `entity_t *`: 基底エンティティ。引数が`NULL`の場合は`NULL`。
entity_t *prop_dynamic_get_entity(prop_dynamic_t *prop);
/// 動的propの読み取り専用基底エンティティを取得する。
///
/// ### Args
/// - `const prop_dynamic_t *prop`: 対象のprop。
///
/// ### Returns
/// - `const entity_t *`: 基底エンティティ。引数が`NULL`の場合は`NULL`。
const entity_t *prop_dynamic_get_const_entity(const prop_dynamic_t *prop);
/// エンティティを動的propとして取得する。
///
/// ### Args
/// - `entity_t *entity`: 変換するエンティティ。
///
/// ### Returns
/// - `prop_dynamic_t *`: 対応するprop。型が異なる場合は`NULL`。
prop_dynamic_t *prop_dynamic_from_entity(entity_t *entity);
/// 読み取り専用エンティティを動的propとして取得する。
///
/// ### Args
/// - `const entity_t *entity`: 変換するエンティティ。
///
/// ### Returns
/// - `const prop_dynamic_t *`: 対応するprop。型が異なる場合は`NULL`。
const prop_dynamic_t *prop_dynamic_from_const_entity(const entity_t *entity);
/// `prop_dynamic`エンティティクラスを登録する。
///
/// ### Returns
/// - `true`: 登録に成功した。
/// - `false`: 登録に失敗した。
bool prop_dynamic_register(void);

#endif
