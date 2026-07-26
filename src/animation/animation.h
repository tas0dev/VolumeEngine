/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_ANIMATION_ANIMATION_H
#define VOLUME_ANIMATION_ANIMATION_H

#include "math/mat4.h"
#include "math/vec3.h"
#include <stdbool.h>
#include <stddef.h>

#define ANIMATION_MAX_BONES 64
#define ANIMATION_MAX_NODES 256

typedef struct animation_quaternion {
	float x;
	float y;
	float z;
	float w;
} animation_quaternion_t;

typedef struct animation_vector_key {
	float time;
	vec3_t value;
} animation_vector_key_t;

typedef struct animation_rotation_key {
	float time;
	animation_quaternion_t value;
} animation_rotation_key_t;

typedef struct animation_channel {
	size_t node_index;
	animation_vector_key_t *positions;
	size_t position_count;
	animation_rotation_key_t *rotations;
	size_t rotation_count;
	animation_vector_key_t *scales;
	size_t scale_count;
} animation_channel_t;

typedef struct animation_clip {
	char *name;
	float duration;
	float ticks_per_second;
	animation_channel_t *channels;
	size_t channel_count;
} animation_clip_t;

typedef struct animation_node {
	char *name;
	int parent_index;
	int bone_index;
	mat4_t bind_transform;
} animation_node_t;

typedef struct animation_set {
	animation_node_t *nodes;
	size_t node_count;
	mat4_t inverse_bind_matrices[ANIMATION_MAX_BONES];
	size_t bone_count;
	mat4_t inverse_root_transform;
	animation_clip_t *clips;
	size_t clip_count;
} animation_set_t;

typedef struct animator {
	const animation_set_t *set;
	const animation_clip_t *clip;
	float time;
	bool looping;
	bool playing;
	mat4_t bone_matrices[ANIMATION_MAX_BONES];
	mat4_t global_transforms[ANIMATION_MAX_NODES];
} animator_t;

/// Animation Setが所有する階層・Clipデータを破棄する。
///
/// ### Args
/// - `animation_set_t *set`: 破棄するAnimation Set。
void animation_set_destroy(animation_set_t *set);

/// 名前からAnimation Clipを検索する。
///
/// ### Args
/// - `const animation_set_t *set`: 検索するAnimation Set。
/// - `const char *name`: Blender Action由来のClip名。
///
/// ### Returns
/// - `const animation_clip_t *`: 見つかったClip。存在しない場合は`NULL`。
const animation_clip_t *animation_set_find_clip(const animation_set_t *set,
						const char *name);

/// AnimatorをAnimation SetのBind Poseで初期化する。
///
/// ### Args
/// - `animator_t *animator`: 初期化するAnimator。
/// - `const animation_set_t *set`: 再生対象のAnimation Set。
///
/// ### Returns
/// - `true`: 初期化した。
/// - `false`: 引数またはSkeletonが不正だった。
bool animator_initialize(animator_t *animator, const animation_set_t *set);

/// 名前を指定してAnimation Clipを先頭から再生する。
///
/// ### Args
/// - `animator_t *animator`: 対象のAnimator。
/// - `const char *clip_name`: 再生するClip名。
/// - `bool looping`: ループ再生する場合は`true`。
///
/// ### Returns
/// - `true`: Clipを見つけて再生を開始した。
/// - `false`: Clipが存在しない、または引数が不正だった。
bool animator_play(animator_t *animator, const char *clip_name, bool looping);

/// Animatorの再生時刻とBone行列を更新する。
///
/// ### Args
/// - `animator_t *animator`: 更新するAnimator。
/// - `float delta_time`: 経過秒数。
void animator_update(animator_t *animator, float delta_time);

/// 現在Clipが再生中かを取得する。
///
/// ### Args
/// - `const animator_t *animator`: 対象のAnimator。
///
/// ### Returns
/// - `true`: Clipを再生中。
/// - `false`: 停止中または引数が不正。
bool animator_is_playing(const animator_t *animator);

/// GPU Skinning用Bone行列配列を取得する。
///
/// ### Args
/// - `const animator_t *animator`: 対象のAnimator。
/// - `size_t *count`: Bone数の格納先。不要なら`NULL`。
///
/// ### Returns
/// - `const mat4_t *`: Bone行列配列。無効な場合は`NULL`。
const mat4_t *animator_get_bone_matrices(const animator_t *animator,
					 size_t *count);

#endif
