/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_SHADER_H
#define VOLUME_SHADER_H

#include "math/mat4.h"
#include "math/vec3.h"

typedef struct shader shader_t;

/// 頂点・フラグメントシェーダーファイルからプログラムを作成する。
///
/// ### Args
/// - `const char *vertex_path`: 頂点シェーダーのパス。
/// - `const char *fragment_path`: フラグメントシェーダーのパス。
///
/// ### Returns
/// - `shader_t *`: 作成したシェーダー。失敗時は`NULL`。
shader_t *shader_create(const char *vertex_path, const char *fragment_path);
/// シェーダープログラムを破棄する。
///
/// ### Args
/// - `shader_t *shader`: 破棄するシェーダー。
void shader_destroy(shader_t *shader);
/// シェーダープログラムを描画に使用する。
///
/// ### Args
/// - `const shader_t *shader`: バインドするシェーダー。
void shader_bind(const shader_t *shader);
/// 現在のシェーダープログラムを解除する。
void shader_unbind(void);
/// シェーダーの4x4行列uniformを設定する。
///
/// ### Args
/// - `const shader_t *shader`: 対象のシェーダー。
/// - `const char *name`: uniform名。
/// - `const mat4_t *matrix`: 設定する行列。
void shader_set_mat4(const shader_t *shader,
		     const char *name,
		     const mat4_t *matrix);
/// シェーダーの3次元ベクトルuniformを設定する。
///
/// ### Args
/// - `const shader_t *shader`: 対象のシェーダー。
/// - `const char *name`: uniform名。
/// - `vec3_t value`: 設定する値。
void shader_set_vec3(const shader_t *shader, const char *name, vec3_t value);
/// シェーダーの浮動小数点uniformを設定する。
///
/// ### Args
/// - `const shader_t *shader`: 対象のシェーダー。
/// - `const char *name`: uniform名。
/// - `float value`: 設定する値。
void shader_set_float(const shader_t *shader, const char *name, float value);
/// シェーダーの整数uniformを設定する。
///
/// ### Args
/// - `const shader_t *shader`: 対象のシェーダー。
/// - `const char *name`: uniform名。
/// - `int value`: 設定する値。
void shader_set_int(const shader_t *shader, const char *name, int value);

#endif // VOLUME_SHADER_H
