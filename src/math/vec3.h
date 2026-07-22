/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_MATH_VEC3_H
#define VOLUME_MATH_VEC3_H

typedef struct vec3 {
	float x;
	float y;
	float z;
} vec3_t;

/// 3つの成分から3次元ベクトルを作成する。
///
/// ### Args
/// - `float x`: X成分。
/// - `float y`: Y成分。
/// - `float z`: Z成分。
///
/// ### Returns
/// - `vec3_t`: 作成したベクトル。
vec3_t vec3_create(float x, float y, float z);
/// 2つのベクトルを加算する。
///
/// ### Args
/// - `vec3_t left`: 左辺のベクトル。
/// - `vec3_t right`: 右辺のベクトル。
///
/// ### Returns
/// - `vec3_t`: 成分ごとの加算結果。
vec3_t vec3_add(vec3_t left, vec3_t right);
/// 左辺ベクトルから右辺ベクトルを減算する。
///
/// ### Args
/// - `vec3_t left`: 左辺のベクトル。
/// - `vec3_t right`: 右辺のベクトル。
///
/// ### Returns
/// - `vec3_t`: 成分ごとの減算結果。
vec3_t vec3_subtract(vec3_t left, vec3_t right);
/// ベクトルをスカラー倍する。
///
/// ### Args
/// - `vec3_t vector`: 対象のベクトル。
/// - `float scalar`: 乗算する値。
///
/// ### Returns
/// - `vec3_t`: スカラー倍したベクトル。
vec3_t vec3_scale(vec3_t vector, float scalar);
/// 2つのベクトルの内積を計算する。
///
/// ### Args
/// - `vec3_t left`: 1つ目のベクトル。
/// - `vec3_t right`: 2つ目のベクトル。
///
/// ### Returns
/// - `float`: 内積。
float vec3_dot(vec3_t left, vec3_t right);
/// 2つのベクトルの外積を計算する。
///
/// ### Args
/// - `vec3_t left`: 1つ目のベクトル。
/// - `vec3_t right`: 2つ目のベクトル。
///
/// ### Returns
/// - `vec3_t`: 両方に直交する外積ベクトル。
vec3_t vec3_cross(vec3_t left, vec3_t right);
/// ベクトルの長さを計算する。
///
/// ### Args
/// - `vec3_t vector`: 対象のベクトル。
///
/// ### Returns
/// - `float`: ユークリッド長。
float vec3_length(vec3_t vector);
/// ベクトルを単位長へ正規化する。
///
/// ### Args
/// - `vec3_t vector`: 対象のベクトル。
///
/// ### Returns
/// - `vec3_t`: 正規化したベクトル。長さが0の場合はゼロベクトル。
vec3_t vec3_normalize(vec3_t vector);

#endif
