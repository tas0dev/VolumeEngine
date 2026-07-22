/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_ENTITY_FUNC_BUTTON_H
#define VOLUME_ENTITY_FUNC_BUTTON_H

#include "entity/prop.h"

typedef enum func_button_state {
	FUNC_BUTTON_IDLE,
	FUNC_BUTTON_PRESSING,
	FUNC_BUTTON_PRESSED,
	FUNC_BUTTON_RELEASING,
} func_button_state_t;

typedef struct func_button {
	prop_t prop;
	vec3_t released_position;
	vec3_t pressed_position;
	float speed;
	float wait;
	float wait_remaining;
	entity_id_t activator_id;
	func_button_state_t state;
	bool locked;
	bool enabled;
} func_button_t;

bool func_button_register(void);
func_button_t *func_button_from_entity(entity_t *entity);
const func_button_t *func_button_from_const_entity(const entity_t *entity);
bool func_button_is_pressed(const func_button_t *button);
bool func_button_is_enabled(const func_button_t *button);
func_button_state_t func_button_get_state(const func_button_t *button);

#endif
