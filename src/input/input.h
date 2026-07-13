/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#ifndef VOLUME_INPUT_INPUT_H
#define VOLUME_INPUT_INPUT_H

#include "core/types.h"
#include <stdbool.h>

typedef enum input_key {
	INPUT_KEY_A,
	INPUT_KEY_B,
	INPUT_KEY_C,
	INPUT_KEY_D,
	INPUT_KEY_E,
	INPUT_KEY_F,
	INPUT_KEY_G,
	INPUT_KEY_H,
	INPUT_KEY_I,
	INPUT_KEY_J,
	INPUT_KEY_K,
	INPUT_KEY_L,
	INPUT_KEY_M,
	INPUT_KEY_N,
	INPUT_KEY_O,
	INPUT_KEY_P,
	INPUT_KEY_Q,
	INPUT_KEY_R,
	INPUT_KEY_S,
	INPUT_KEY_T,
	INPUT_KEY_U,
	INPUT_KEY_V,
	INPUT_KEY_W,
	INPUT_KEY_X,
	INPUT_KEY_Y,
	INPUT_KEY_Z,
	INPUT_KEY_SPACE,
	INPUT_KEY_CONTROL,
	INPUT_KEY_ESCAPE,
	INPUT_KEY_COUNT,
} input_key_t;

typedef enum input_mouse_button {
	INPUT_MOUSE_BUTTON_LEFT,
	INPUT_MOUSE_BUTTON_MIDDLE,
	INPUT_MOUSE_BUTTON_RIGHT,
	INPUT_MOUSE_BUTTON_COUNT,
} input_mouse_button_t;

input_t *input_create(void);
void input_destroy(input_t *input);
void input_begin_frame(input_t *input);
bool input_key_down(const input_t *input, input_key_t key);
bool input_key_pressed(const input_t *input, input_key_t key);
bool input_key_released(const input_t *input, input_key_t key);
bool input_mouse_button_down(const input_t *input, input_mouse_button_t button);
bool input_mouse_button_pressed(const input_t *input,
				input_mouse_button_t button);
bool input_mouse_button_released(const input_t *input,
				 input_mouse_button_t button);
void input_get_mouse_position(const input_t *input, float *x, float *y);
void input_get_mouse_delta(const input_t *input, float *x, float *y);
void input_set_key(input_t *input, input_key_t key, bool down);
void input_set_mouse_button(input_t *input,
			    input_mouse_button_t button,
			    bool down);
void input_set_mouse_position(input_t *input, float x, float y);
void input_add_mouse_delta(input_t *input, float x, float y);

#endif