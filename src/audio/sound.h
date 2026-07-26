/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_AUDIO_SOUND_H
#define VOLUME_AUDIO_SOUND_H

#include <stddef.h>

typedef struct audio_sound audio_sound_t;

/// WAVファイルを読み込み、エンジンの再生形式へ変換する。
///
/// ### Args
/// - `const char *path`: 読み込むWAVファイルのパス。
/// - `char *error`: エラーメッセージの格納先。
/// - `size_t error_size`: エラー格納先のバイト数。
///
/// ### Returns
/// - `audio_sound_t *`: 作成したサウンド。失敗時は`NULL`。
audio_sound_t *
audio_sound_load_wav(const char *path, char *error, size_t error_size);

/// 短い正弦波サウンドを作成する。
///
/// ゲーム固有の仮効果音や音声システムの確認に使用できる。
///
/// ### Args
/// - `float frequency`: 周波数。単位はHz。
/// - `float duration`: 長さ。単位は秒。
///
/// ### Returns
/// - `audio_sound_t *`: 作成したサウンド。失敗時は`NULL`。
audio_sound_t *audio_sound_create_tone(float frequency, float duration);

/// サウンドのPCMデータを破棄する。
///
/// ### Args
/// - `audio_sound_t *sound`: 破棄するサウンド。
void audio_sound_destroy(audio_sound_t *sound);

#endif
