/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_ENTITY_INFO_PLAYER_START_H
#define VOLUME_ENTITY_INFO_PLAYER_START_H

#include "entity/entity.h"

typedef struct info_player_start {
	entity_t entity;
} info_player_start_t;

bool info_player_start_register(void);
info_player_start_t *info_player_start_from_entity(entity_t *entity);
const info_player_start_t *
info_player_start_from_const_entity(const entity_t *entity);

#endif
