/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_ENTITY_FUNC_DOOR_H
#define VOLUME_ENTITY_FUNC_DOOR_H

#include "entity/prop.h"

typedef enum func_door_state {
	FUNC_DOOR_CLOSED,
	FUNC_DOOR_OPENING,
	FUNC_DOOR_OPEN,
	FUNC_DOOR_CLOSING,
} func_door_state_t;

typedef struct func_door {
	prop_t prop;
	vec3_t closed_position;
	vec3_t open_position;
	float speed;
	float wait;
	float wait_remaining;
	entity_id_t activator_id;
	entity_id_t blocker_id;
	func_door_state_t state;
	bool locked;
	bool blocked;
} func_door_t;

func_door_t *func_door_from_entity(entity_t *entity);
const func_door_t *func_door_from_const_entity(const entity_t *entity);
func_door_state_t func_door_get_state(const func_door_t *door);
bool func_door_is_blocked(const func_door_t *door);
bool func_door_register(void);

#endif
