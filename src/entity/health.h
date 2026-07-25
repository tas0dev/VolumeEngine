/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_ENTITY_HEALTH_H
#define VOLUME_ENTITY_HEALTH_H

#include <stdbool.h>

typedef struct health {
	float current;
	float maximum;
	bool dead;
} health_t;

/// ヘルスを指定した最大値で初期化する。
///
/// ### Args
/// - `health_t *health`: 初期化するヘルス。
/// - `float maximum`: 最大ヘルス。
///
/// ### Returns
/// - `true`: 初期化に成功した。
/// - `false`: 最大値または引数が不正だった。
bool health_initialize(health_t *health, float maximum);

/// ヘルスからダメージ量を減算する。
///
/// ### Args
/// - `health_t *health`: 対象のヘルス。
/// - `float amount`: 適用するダメージ量。
///
/// ### Returns
/// - `float`: 実際に減少したヘルス量。
float health_apply_damage(health_t *health, float amount);

/// 現在のヘルス値を取得する。
///
/// ### Args
/// - `const health_t *health`: 対象のヘルス。
///
/// ### Returns
/// - `float`: 現在値。引数が`NULL`の場合は`0`。
float health_get_current(const health_t *health);

/// 最大ヘルス値を取得する。
///
/// ### Args
/// - `const health_t *health`: 対象のヘルス。
///
/// ### Returns
/// - `float`: 最大値。引数が`NULL`の場合は`0`。
float health_get_maximum(const health_t *health);

/// ヘルスが残っているか調べる。
///
/// ### Args
/// - `const health_t *health`: 対象のヘルス。
///
/// ### Returns
/// - `true`: 生存している。
/// - `false`: 死亡している、または引数が`NULL`。
bool health_is_alive(const health_t *health);

#endif
