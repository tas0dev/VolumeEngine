/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_AUDIO_AUDIO_H
#define VOLUME_AUDIO_AUDIO_H

#include "audio/sound.h"
#include "core/types.h"
#include "math/vec3.h"
#include <stdbool.h>
#include <stdint.h>

typedef uint32_t audio_voice_id_t;

#define AUDIO_INVALID_VOICE ((audio_voice_id_t)0)

typedef struct audio_play_params {
	float volume;
	bool looping;
	bool spatial;
	vec3_t position;
	float minimum_distance;
	float maximum_distance;
} audio_play_params_t;

/// 音声再生システムを作成する。
///
/// 出力デバイスが存在しない場合も無音状態のシステムを返す。
///
/// ### Returns
/// - `audio_system_t *`: 作成した音声システム。メモリ不足時は`NULL`。
audio_system_t *audio_system_create(void);

/// 音声再生を停止し、システムを破棄する。
///
/// ### Args
/// - `audio_system_t *audio`: 破棄する音声システム。
void audio_system_destroy(audio_system_t *audio);

/// 既定の再生パラメーターを作成する。
///
/// ### Returns
/// - `audio_play_params_t`: 音量1の非ループ2D再生設定。
audio_play_params_t audio_play_params_create(void);

/// サウンドの再生を開始する。
///
/// ### Args
/// - `audio_system_t *audio`: 使用する音声システム。
/// - `const audio_sound_t *sound`: 再生するサウンド。
/// - `const audio_play_params_t *params`: 再生設定。`NULL`なら既定値。
///
/// ### Returns
/// - `audio_voice_id_t`: 再生音のID。失敗時は`AUDIO_INVALID_VOICE`。
audio_voice_id_t audio_system_play(audio_system_t *audio,
				   const audio_sound_t *sound,
				   const audio_play_params_t *params);

/// 指定した再生音を停止する。
///
/// ### Args
/// - `audio_system_t *audio`: 使用する音声システム。
/// - `audio_voice_id_t voice`: 停止する再生音ID。
void audio_system_stop(audio_system_t *audio, audio_voice_id_t voice);

/// すべての再生音を停止する。
///
/// ### Args
/// - `audio_system_t *audio`: 使用する音声システム。
void audio_system_stop_all(audio_system_t *audio);

/// 再生音が現在有効か調べる。
///
/// ### Args
/// - `audio_system_t *audio`: 使用する音声システム。
/// - `audio_voice_id_t voice`: 調べる再生音ID。
///
/// ### Returns
/// - `true`: 再生中。
/// - `false`: 停止または無効なID。
bool audio_system_is_playing(audio_system_t *audio, audio_voice_id_t voice);

/// 3D再生音のワールド座標を更新する。
///
/// ### Args
/// - `audio_system_t *audio`: 使用する音声システム。
/// - `audio_voice_id_t voice`: 更新する再生音ID。
/// - `vec3_t position`: 新しいワールド座標。
void audio_system_set_voice_position(audio_system_t *audio,
				     audio_voice_id_t voice,
				     vec3_t position);

/// 3D音声のリスナー状態を更新する。
///
/// ### Args
/// - `audio_system_t *audio`: 使用する音声システム。
/// - `vec3_t position`: リスナーのワールド座標。
/// - `vec3_t forward`: 正面方向。
/// - `vec3_t up`: 上方向。
void audio_system_set_listener(audio_system_t *audio,
			       vec3_t position,
			       vec3_t forward,
			       vec3_t up);

#endif
