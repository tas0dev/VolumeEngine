/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_DEBUG_DEBUG_UI_H
#define VOLUME_DEBUG_DEBUG_UI_H

#include "renderer/renderer.h"

typedef struct debug_ui {
	renderer_t *renderer;
	float x;
	float y;
	float line_height;
	float label_width;
	float scale;
	renderer_color_t text_color;
	renderer_color_t label_color;
	renderer_color_t shadow_color;
} debug_ui_t;

void debug_ui_begin(debug_ui_t *ui, renderer_t *renderer, float x, float y);
void debug_ui_set_line_height(debug_ui_t *ui, float line_height);
void debug_ui_set_label_width(debug_ui_t *ui, float label_width);
void debug_ui_draw_text(debug_ui_t *ui, const char *text);
void debug_ui_draw_row(debug_ui_t *ui, const char *label, const char *value);
void debug_ui_add_spacing(debug_ui_t *ui, float spacing);

#endif