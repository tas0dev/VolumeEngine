/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_ENTITY_FUNC_BUTTON_H
#define VOLUME_ENTITY_FUNC_BUTTON_H

#include "entity/prop.h"

typedef struct func_button {
	prop_t prop;
	float wait;
	float wait_remaining;
	entity_id_t activator_id;
	bool pressed;
	bool locked;
	bool enabled;
} func_button_t;

bool func_button_register(void);
func_button_t *func_button_from_entity(entity_t *entity);
const func_button_t *func_button_from_const_entity(const entity_t *entity);
bool func_button_is_pressed(const func_button_t *button);
bool func_button_is_enabled(const func_button_t *button);

#endif
