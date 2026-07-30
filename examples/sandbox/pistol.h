/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_EXAMPLE_SANDBOX_PISTOL_H
#define VOLUME_EXAMPLE_SANDBOX_PISTOL_H

#include "asset/manager.h"
#include "audio/audio.h"
#include "fps/effects.h"
#include "fps/recoil.h"
#include "renderer/renderer.h"
#include "weapon/hitscan_weapon.h"

typedef struct sandbox_pistol {
	hitscan_weapon_t weapon;
	audio_system_t *audio;
	audio_sound_t *fire_sound;
	audio_sound_t *reload_sound;
	audio_voice_id_t fire_voice;
	audio_voice_id_t reload_voice;
	const mesh_t *view_model_mesh;
	const material_t *view_model_material;
	fps_effect_system_t *effects;
	fps_recoil_t recoil;
	animator_t animator;
	bool has_animator;
	bool reload_sound_event;
	float muzzle_flash_time;
	float bob_time;
	float bob_amount;
	float sway_pitch;
	float sway_yaw;
	float recoil_direction;
} sandbox_pistol_t;

/// sandbox固有のピストルを初期化する。
///
/// ### Args
/// - `sandbox_pistol_t *pistol`: 初期化するピストル。
/// - `asset_manager_t *assets`: ViewModelを読み込むアセット管理。
/// - `audio_system_t *audio`: 効果音を再生する音声システム。
/// - `char *error`: エラーメッセージの格納先。
/// - `size_t error_size`: エラー格納先のバイト数。
///
/// ### Returns
/// - `true`: 初期化に成功した。
/// - `false`: 武器設定またはゲームアセットの読み込みに失敗した。
bool sandbox_pistol_initialize(sandbox_pistol_t *pistol,
			       asset_manager_t *assets,
			       audio_system_t *audio,
			       char *error,
			       size_t error_size);

/// ピストルの発射クールダウンを更新する。
///
/// ### Args
/// - `sandbox_pistol_t *pistol`: 更新するピストル。
/// - `float delta_time`: 経過秒数。
/// - `float movement_speed`: プレイヤーの水平移動速度。
/// - `bool grounded`: プレイヤーが接地している場合は`true`。
void sandbox_pistol_update(sandbox_pistol_t *pistol,
			   float delta_time,
			   float movement_speed,
			   bool grounded);

/// マウス視点移動をViewModelのswayへ加える。
///
/// ### Args
/// - `sandbox_pistol_t *pistol`: 対象のピストル。
/// - `float yaw_delta`: このフレームの左右角度変化。
/// - `float pitch_delta`: このフレームの上下角度変化。
void sandbox_pistol_add_look_delta(sandbox_pistol_t *pistol,
				   float yaw_delta,
				   float pitch_delta);

/// ピストルを発射し、成功時にゲーム固有の発射音を再生する。
///
/// ### Args
/// - `sandbox_pistol_t *pistol`: 発射するピストル。
/// - `world_t *world`: 射撃対象のワールド。
/// - `entity_t *owner`: 射撃者。
/// - `vec3_t origin`: 射撃開始位置。
/// - `vec3_t direction`: 射撃方向。
/// - `collision_trace_t *trace`: 命中結果の格納先。不要なら`NULL`。
///
/// ### Returns
/// - `true`: 発射した。
/// - `false`: 弾切れまたはクールダウン中だった。
bool sandbox_pistol_fire(sandbox_pistol_t *pistol,
			 world_t *world,
			 entity_t *owner,
			 vec3_t origin,
			 vec3_t direction,
			 collision_trace_t *trace);

/// ピストルをリロードし、成功時にゲーム固有の効果音を再生する。
///
/// ### Args
/// - `sandbox_pistol_t *pistol`: リロードするピストル。
///
/// ### Returns
/// - `true`: 1発以上装填した。
/// - `false`: リロードできなかった。
bool sandbox_pistol_reload(sandbox_pistol_t *pistol);

/// ピストルの弾薬とクールダウンを初期状態へ戻す。
///
/// ### Args
/// - `sandbox_pistol_t *pistol`: リセットするピストル。
///
/// ### Returns
/// - `true`: リセットに成功した。
/// - `false`: 引数が不正だった。
bool sandbox_pistol_reset(sandbox_pistol_t *pistol);

/// 現在の視点リコイル角度を取得する。
///
/// ### Args
/// - `const sandbox_pistol_t *pistol`: 対象のピストル。
///
/// ### Returns
/// - `fps_recoil_offset_t`: カメラへ加えるpitchとyaw。
fps_recoil_offset_t sandbox_pistol_get_recoil(const sandbox_pistol_t *pistol);

/// sandbox固有のピストルViewModelを描画する。
///
/// ### Args
/// - `const sandbox_pistol_t *pistol`: 描画するピストル。
/// - `renderer_t *renderer`: 使用するレンダラー。
/// - `const render_view_t *world_view`: ワールドの光源設定。
void sandbox_pistol_draw(const sandbox_pistol_t *pistol,
			 renderer_t *renderer,
			 const render_view_t *world_view);

/// 現在のマガジン内弾数を取得する。
///
/// ### Args
/// - `const sandbox_pistol_t *pistol`: 対象のピストル。
///
/// ### Returns
/// - `int`: マガジン内弾数。
int sandbox_pistol_get_ammo(const sandbox_pistol_t *pistol);

/// 現在の予備弾薬数を取得する。
///
/// ### Args
/// - `const sandbox_pistol_t *pistol`: 対象のピストル。
///
/// ### Returns
/// - `int`: 予備弾薬数。
int sandbox_pistol_get_reserve_ammo(const sandbox_pistol_t *pistol);

/// ピストル固有の実行時リソースを破棄する。
///
/// ### Args
/// - `sandbox_pistol_t *pistol`: 破棄するピストル。
void sandbox_pistol_destroy(sandbox_pistol_t *pistol);

#endif
