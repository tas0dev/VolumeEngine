/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "input/input.h"
#include <stdlib.h>
#include <string.h>

struct input {
	bool keys[INPUT_KEY_COUNT];
	bool previous_keys[INPUT_KEY_COUNT];
	bool mouse_buttons[INPUT_MOUSE_BUTTON_COUNT];
	bool previous_mouse_buttons[INPUT_MOUSE_BUTTON_COUNT];
	float mouse_x;
	float mouse_y;
	float mouse_delta_x;
	float mouse_delta_y;
	float mouse_wheel_x;
	float mouse_wheel_y;
};

input_t *input_create(void) {
	input_t *input;

	input = calloc(1, sizeof(*input));
	if (input == NULL) { return NULL; }

	return input;
}

void input_destroy(input_t *input) { free(input); }

void input_begin_frame(input_t *input) {
	if (input == NULL) { return; }

	memcpy(input->previous_keys, input->keys, sizeof(input->keys));

	memcpy(input->previous_mouse_buttons, input->mouse_buttons,
	       sizeof(input->mouse_buttons));

	input->mouse_delta_x = 0.0f;
	input->mouse_delta_y = 0.0f;
	input->mouse_wheel_x = 0.0f;
	input->mouse_wheel_y = 0.0f;
}

bool input_key_down(const input_t *input, input_key_t const key) {
	if (input == NULL || key >= INPUT_KEY_COUNT) { return false; }

	return input->keys[key];
}

bool input_key_pressed(const input_t *input, const input_key_t key) {
	if (input == NULL || key >= INPUT_KEY_COUNT) { return false; }

	return input->keys[key] && !input->previous_keys[key];
}

bool input_key_released(const input_t *input, const input_key_t key) {
	if (input == NULL || key >= INPUT_KEY_COUNT) { return false; }

	return !input->keys[key] && input->previous_keys[key];
}

bool input_mouse_button_down(const input_t *input,
			     const input_mouse_button_t button) {
	if (input == NULL || button >= INPUT_MOUSE_BUTTON_COUNT) {
		return false;
	}

	return input->mouse_buttons[button];
}

bool input_mouse_button_pressed(const input_t *input,
				const input_mouse_button_t button) {
	if (input == NULL || button >= INPUT_MOUSE_BUTTON_COUNT) {
		return false;
	}

	return input->mouse_buttons[button] &&
	       !input->previous_mouse_buttons[button];
}

bool input_mouse_button_released(const input_t *input,
				 const input_mouse_button_t button) {
	if (input == NULL || button >= INPUT_MOUSE_BUTTON_COUNT) {
		return false;
	}

	return !input->mouse_buttons[button] &&
	       input->previous_mouse_buttons[button];
}

void input_get_mouse_position(const input_t *input, float *x, float *y) {
	if (x != NULL) { *x = 0.0f; }

	if (y != NULL) { *y = 0.0f; }

	if (input == NULL) { return; }

	if (x != NULL) { *x = input->mouse_x; }

	if (y != NULL) { *y = input->mouse_y; }
}

void input_get_mouse_delta(const input_t *input, float *x, float *y) {
	if (x != NULL) { *x = 0.0f; }

	if (y != NULL) { *y = 0.0f; }

	if (input == NULL) { return; }

	if (x != NULL) { *x = input->mouse_delta_x; }

	if (y != NULL) { *y = input->mouse_delta_y; }
}

void input_get_mouse_wheel(const input_t *input, float *x, float *y) {
	if (x != NULL) { *x = 0.0f; }
	if (y != NULL) { *y = 0.0f; }
	if (input == NULL) { return; }
	if (x != NULL) { *x = input->mouse_wheel_x; }
	if (y != NULL) { *y = input->mouse_wheel_y; }
}

void input_set_key(input_t *input, const input_key_t key, const bool down) {
	if (input == NULL || key >= INPUT_KEY_COUNT) { return; }

	input->keys[key] = down;
}

void input_set_mouse_button(input_t *input,
			    const input_mouse_button_t button,
			    const bool down) {
	if (input == NULL || button >= INPUT_MOUSE_BUTTON_COUNT) { return; }

	input->mouse_buttons[button] = down;
}

void input_set_mouse_position(input_t *input, const float x, const float y) {
	if (input == NULL) { return; }

	input->mouse_x = x;
	input->mouse_y = y;
}

void input_add_mouse_delta(input_t *input, const float x, const float y) {
	if (input == NULL) { return; }

	input->mouse_delta_x += x;
	input->mouse_delta_y += y;
}

void input_add_mouse_wheel(input_t *input, const float x, const float y) {
	if (input == NULL) { return; }
	input->mouse_wheel_x += x;
	input->mouse_wheel_y += y;
}
