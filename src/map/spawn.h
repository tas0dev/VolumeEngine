/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_MAP_SPAWN_H
#define VOLUME_MAP_SPAWN_H

#include "asset/manager.h"
#include "entity/world.h"
#include "map/map.h"
#include <stdbool.h>
#include <stddef.h>

/// 解析済みマップの全エンティティを三段階でワールドへ生成する。
///
/// 全エンティティの生成後にOutputを読み込み、最後にActivateを呼び出す。
///
/// ### Args
/// - `const map_t *map`: 生成元のマップ。
/// - `world_t *world`: 生成先のワールド。
/// - `asset_manager_t *assets`: アセット解決に使用するマネージャー。
/// - `char *error`: エラーメッセージの格納先。
/// - `size_t error_size`: エラー格納先のバイト数。
///
/// ### Returns
/// - `true`: 全エンティティの生成に成功した。
/// - `false`: いずれかの生成に失敗し、追加分をロールバックした。
bool map_spawn_entities(const map_t *map,
			world_t *world,
			asset_manager_t *assets,
			char *error,
			size_t error_size);
/// マップファイルを読み込み、ワールドへエンティティを生成する。
///
/// ### Args
/// - `world_t *world`: 読み込み先のワールド。
/// - `asset_manager_t *assets`: アセット解決に使用するマネージャー。
/// - `const char *path`: 読み込むマップファイルのパス。
/// - `char *error`: エラーメッセージの格納先。
/// - `size_t error_size`: エラー格納先のバイト数。
///
/// ### Returns
/// - `true`: 読み込みと生成に成功した。
/// - `false`: ファイル読込、解析、または生成に失敗した。
bool world_load_map(world_t *world,
		    asset_manager_t *assets,
		    const char *path,
		    char *error,
		    size_t error_size);

#endif
