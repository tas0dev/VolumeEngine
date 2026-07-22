/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_ENTITY_LIGHT_ENVIRONMENT_H
#define VOLUME_ENTITY_LIGHT_ENVIRONMENT_H

#include "entity/entity.h"
#include "math/vec3.h"

typedef struct light_environment_properties {
	entity_properties_t entity;
	vec3_t color;
	float intensity;
} light_environment_properties_t;

typedef struct light_environment {
	entity_t entity;
	vec3_t color;
	float intensity;
} light_environment_t;

/// 環境光源プロパティの既定値を作成する。
///
/// ### Returns
/// - `light_environment_properties_t`: 既定値で初期化されたプロパティ。
light_environment_properties_t light_environment_properties_create(void);
/// 環境光源エンティティを作成する。
///
/// ### Args
/// - `entity_id_t id`: 割り当てる一意なID。
/// - `const light_environment_properties_t *properties`: 作成に使用するプロパティ。
///
/// ### Returns
/// - `light_environment_t *`: 作成した環境光源。失敗時は`NULL`。
light_environment_t *
light_environment_create(entity_id_t id,
			 const light_environment_properties_t *properties);
/// エンティティを環境光源として取得する。
///
/// ### Args
/// - `entity_t *entity`: 変換するエンティティ。
///
/// ### Returns
/// - `light_environment_t *`: 対応する環境光源。型が異なる場合は`NULL`。
light_environment_t *light_environment_from_entity(entity_t *entity);
/// 読み取り専用エンティティを環境光源として取得する。
///
/// ### Args
/// - `const entity_t *entity`: 変換するエンティティ。
///
/// ### Returns
/// - `const light_environment_t *`: 対応する環境光源。型が異なる場合は`NULL`。
const light_environment_t *
light_environment_from_const_entity(const entity_t *entity);
/// 環境光源が照らすワールド空間の方向を取得する。
///
/// ### Args
/// - `const light_environment_t *light`: 対象の環境光源。
///
/// ### Returns
/// - `vec3_t`: 正規化された照射方向。
vec3_t light_environment_get_direction(const light_environment_t *light);
/// 環境光源の色を取得する。
///
/// ### Args
/// - `const light_environment_t *light`: 対象の環境光源。
///
/// ### Returns
/// - `vec3_t`: RGB形式の光源色。
vec3_t light_environment_get_color(const light_environment_t *light);
/// 環境光源の強度を取得する。
///
/// ### Args
/// - `const light_environment_t *light`: 対象の環境光源。
///
/// ### Returns
/// - `float`: 光源強度。
float light_environment_get_intensity(const light_environment_t *light);
/// `light_environment`エンティティクラスを登録する。
///
/// ### Returns
/// - `true`: 登録に成功した。
/// - `false`: 登録に失敗した。
bool light_environment_register(void);

#endif
