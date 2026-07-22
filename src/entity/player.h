/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_ENTITY_PLAYER_H
#define VOLUME_ENTITY_PLAYER_H

#include "entity/entity.h"
#include "physics/character_controller.h"

typedef struct player player_t;

bool player_register(void);
player_t *player_from_entity(entity_t *entity);
const player_t *player_from_const_entity(const entity_t *entity);
entity_t *player_get_entity(player_t *player);
const entity_t *player_get_const_entity(const player_t *player);
vec3_t player_get_position(const player_t *player);
void player_move(player_t *player,
		 const character_move_input_t *input,
		 float delta_time);

#endif
