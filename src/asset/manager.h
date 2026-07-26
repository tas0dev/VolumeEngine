/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_ASSET_MANAGER_H
#define VOLUME_ASSET_MANAGER_H

#include "audio/sound.h"
#include "renderer/material.h"
#include "renderer/mesh.h"
#include "renderer/texture.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct asset_manager asset_manager_t;

/// カレントディレクトリを基準とするアセットマネージャーを作成する。
///
/// ### Returns
/// - `asset_manager_t *`: 作成したマネージャー。失敗時は`NULL`。
asset_manager_t *asset_manager_create(void);
/// アセットマネージャーと所有する全リソースを破棄する。
///
/// ### Args
/// - `asset_manager_t *manager`: 破棄するマネージャー。
void asset_manager_destroy(asset_manager_t *manager);
/// パスとメッシュをマネージャーへ登録する。
///
/// ### Args
/// - `asset_manager_t *manager`: 登録先のマネージャー。
/// - `const char *path`: 検索に使用する論理パス。
/// - `mesh_t *mesh`: 所有権を移すメッシュ。
///
/// ### Returns
/// - `true`: 登録に成功した。
/// - `false`: パス重複、引数不正、またはメモリ不足で失敗した。
bool asset_manager_register_mesh(asset_manager_t *manager,
				 const char *path,
				 mesh_t *mesh);
/// パスとマテリアルをマネージャーへ登録する。
///
/// ### Args
/// - `asset_manager_t *manager`: 登録先のマネージャー。
/// - `const char *path`: 検索に使用する論理パス。
/// - `material_t *material`: 所有権を移すマテリアル。
///
/// ### Returns
/// - `true`: 登録に成功した。
/// - `false`: パス重複、引数不正、またはメモリ不足で失敗した。
bool asset_manager_register_material(asset_manager_t *manager,
				     const char *path,
				     material_t *material);
/// パスとサウンドをマネージャーへ登録する。
///
/// マネージャーはサウンドの所有権を取得しない。
///
/// ### Args
/// - `asset_manager_t *manager`: 登録先のマネージャー。
/// - `const char *path`: 検索に使用する論理パス。
/// - `audio_sound_t *sound`: 登録するサウンド。
///
/// ### Returns
/// - `true`: 登録に成功した。
/// - `false`: パス重複、引数不正、またはメモリ不足で失敗した。
bool asset_manager_register_sound(asset_manager_t *manager,
				  const char *path,
				  audio_sound_t *sound);
/// 登録済みサウンドをパスから取得する。
///
/// ### Args
/// - `const asset_manager_t *manager`: 検索するマネージャー。
/// - `const char *path`: 論理アセットパス。
///
/// ### Returns
/// - `audio_sound_t *`: 登録済みサウンド。存在しない場合は`NULL`。
audio_sound_t *asset_manager_get_sound(const asset_manager_t *manager,
				       const char *path);
/// 登録済みメッシュをパスから取得する。
///
/// ### Args
/// - `const asset_manager_t *manager`: 検索するマネージャー。
/// - `const char *path`: 登録時の論理パス。
///
/// ### Returns
/// - `mesh_t *`: 登録済みメッシュ。存在しない場合は`NULL`。
mesh_t *asset_manager_get_mesh(const asset_manager_t *manager,
			       const char *path);
/// 登録済みマテリアルをパスから取得する。
///
/// ### Args
/// - `const asset_manager_t *manager`: 検索するマネージャー。
/// - `const char *path`: 登録時の論理パス。
///
/// ### Returns
/// - `material_t *`: 登録済みマテリアル。存在しない場合は`NULL`。
material_t *asset_manager_get_material(const asset_manager_t *manager,
				       const char *path);
/// 指定ルートディレクトリを基準とするアセットマネージャーを作成する。
///
/// ### Args
/// - `const char *root_path`: アセットパスの基準ディレクトリ。
///
/// ### Returns
/// - `asset_manager_t *`: 作成したマネージャー。失敗時は`NULL`。
asset_manager_t *asset_manager_create_at(const char *root_path);
/// メッシュを読み込み、キャッシュへ登録して取得する。
///
/// ### Args
/// - `asset_manager_t *manager`: 使用するマネージャー。
/// - `const char *path`: ルートからのアセットパス。
/// - `char *error`: エラーメッセージの格納先。
/// - `size_t error_size`: エラー格納先のバイト数。
///
/// ### Returns
/// - `mesh_t *`: 読み込み済みまたは新規メッシュ。失敗時は`NULL`。
mesh_t *asset_manager_load_mesh(asset_manager_t *manager,
				const char *path,
				char *error,
				size_t error_size);
/// マテリアルを読み込み、参照テクスチャとともにキャッシュへ登録する。
///
/// ### Args
/// - `asset_manager_t *manager`: 使用するマネージャー。
/// - `const char *path`: ルートからのアセットパス。
/// - `char *error`: エラーメッセージの格納先。
/// - `size_t error_size`: エラー格納先のバイト数。
///
/// ### Returns
/// - `material_t *`: 読み込み済みまたは新規マテリアル。失敗時は`NULL`。
material_t *asset_manager_load_material(asset_manager_t *manager,
					const char *path,
					char *error,
					size_t error_size);
/// WAVサウンドを読み込み、キャッシュへ登録して取得する。
///
/// ### Args
/// - `asset_manager_t *manager`: 使用するマネージャー。
/// - `const char *path`: ルートからのアセットパス。
/// - `char *error`: エラーメッセージの格納先。
/// - `size_t error_size`: エラー格納先のバイト数。
///
/// ### Returns
/// - `audio_sound_t *`: 読み込み済みまたは新規サウンド。失敗時は`NULL`。
audio_sound_t *asset_manager_load_sound(asset_manager_t *manager,
					const char *path,
					char *error,
					size_t error_size);
/// 登録済みテクスチャをパスから取得する。
///
/// ### Args
/// - `const asset_manager_t *manager`: 検索するマネージャー。
/// - `const char *path`: 登録時の論理パス。
///
/// ### Returns
/// - `texture_t *`: 登録済みテクスチャ。存在しない場合は`NULL`。
texture_t *asset_manager_get_texture(const asset_manager_t *manager,
				     const char *path);
/// テクスチャを読み込み、キャッシュへ登録して取得する。
///
/// ### Args
/// - `asset_manager_t *manager`: 使用するマネージャー。
/// - `const char *path`: ルートからのアセットパス。
/// - `char *error`: エラーメッセージの格納先。
/// - `size_t error_size`: エラー格納先のバイト数。
///
/// ### Returns
/// - `texture_t *`: 読み込み済みまたは新規テクスチャ。失敗時は`NULL`。
texture_t *asset_manager_load_texture(asset_manager_t *manager,
				      const char *path,
				      char *error,
				      size_t error_size);

#endif
