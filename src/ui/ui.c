/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "ui/ui.h"
#include <stdlib.h>

struct ui_context {
	renderer_t *renderer;
	input_t *input;
	const renderer_font_t *font;
	ui_style_t style;
};

static bool contains(ui_rect_t rectangle, float x, float y);

ui_context_t *ui_context_create(renderer_t *renderer,
				input_t *input,
				const renderer_font_t *font) {
	ui_context_t *ui;

	if (renderer == NULL || input == NULL || font == NULL) { return NULL; }
	ui = calloc(1, sizeof(*ui));
	if (ui == NULL) { return NULL; }
	ui->renderer = renderer;
	ui->input = input;
	ui->font = font;
	ui->style.panel_color = (renderer_color_t){0.09f, 0.10f, 0.12f, 1.0f};
	ui->style.button_color = (renderer_color_t){0.17f, 0.19f, 0.23f, 1.0f};
	ui->style.button_hover_color =
		(renderer_color_t){0.24f, 0.28f, 0.34f, 1.0f};
	ui->style.button_pressed_color =
		(renderer_color_t){0.12f, 0.15f, 0.19f, 1.0f};
	ui->style.button_disabled_color =
		(renderer_color_t){0.10f, 0.11f, 0.13f, 1.0f};
	ui->style.text_color = (renderer_color_t){0.91f, 0.93f, 0.96f, 1.0f};
	ui->style.text_disabled_color =
		(renderer_color_t){0.43f, 0.46f, 0.50f, 1.0f};
	ui->style.text_height = 18.0f;
	return ui;
}

void ui_context_destroy(ui_context_t *ui) { free(ui); }

void ui_context_set_style(ui_context_t *ui, const ui_style_t *style) {
	if (ui != NULL && style != NULL) { ui->style = *style; }
}

void ui_panel(ui_context_t *ui, const ui_rect_t rectangle) {
	if (ui == NULL) { return; }
	renderer_draw_rectangle(ui->renderer, rectangle.x, rectangle.y,
				rectangle.width, rectangle.height,
				ui->style.panel_color);
}

void ui_label(ui_context_t *ui,
	      const float x,
	      const float y,
	      const float height,
	      const renderer_color_t color,
	      const char *text) {
	if (ui == NULL || text == NULL) { return; }
	renderer_draw_text_with_font(
		ui->renderer, ui->font, x, y,
		height > 0.0f ? height : ui->style.text_height, color, text);
}

bool ui_button(ui_context_t *ui,
	       const ui_rect_t rectangle,
	       const char *label,
	       const bool enabled) {
	renderer_color_t background;
	renderer_color_t text_color;
	float mouse_x;
	float mouse_y;
	float text_width;
	float text_x;
	float text_y;
	bool hovered;
	bool pressed;

	if (ui == NULL || label == NULL) { return false; }
	input_get_mouse_position(ui->input, &mouse_x, &mouse_y);
	hovered = enabled && contains(rectangle, mouse_x, mouse_y);
	pressed = hovered &&
		  input_mouse_button_down(ui->input, INPUT_MOUSE_BUTTON_LEFT);
	background = !enabled  ? ui->style.button_disabled_color
		     : pressed ? ui->style.button_pressed_color
		     : hovered ? ui->style.button_hover_color
			       : ui->style.button_color;
	text_color =
		enabled ? ui->style.text_color : ui->style.text_disabled_color;
	renderer_draw_rectangle(ui->renderer, rectangle.x, rectangle.y,
				rectangle.width, rectangle.height, background);
	text_width = renderer_font_measure_text(ui->font, ui->style.text_height,
						label);
	text_x = rectangle.x + (rectangle.width - text_width) * 0.5f;
	text_y =
		rectangle.y + (rectangle.height - ui->style.text_height) * 0.5f;
	ui_label(ui, text_x, text_y, ui->style.text_height, text_color, label);
	return hovered &&
	       input_mouse_button_pressed(ui->input, INPUT_MOUSE_BUTTON_LEFT);
}

static bool contains(const ui_rect_t rectangle, const float x, const float y) {
	return x >= rectangle.x && x <= rectangle.x + rectangle.width &&
	       y >= rectangle.y && y <= rectangle.y + rectangle.height;
}
