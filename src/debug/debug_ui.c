/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "debug/debug_ui.h"

static void draw_shadowed_text(const debug_ui_t *ui,
			       float x,
			       float y,
			       renderer_color_t color,
			       const char *text);

void debug_ui_begin(debug_ui_t *ui,
		    renderer_t *renderer,
		    const float x,
		    const float y) {
	if (ui == NULL) { return; }

	ui->renderer = renderer;
	ui->x = x;
	ui->y = y;
	ui->line_height = 14.0f;
	ui->label_width = 88.0f;
	ui->scale = 1.0f;
	ui->text_color = (renderer_color_t){0.9f, 0.9f, 0.9f, 1.0f};
	ui->label_color = (renderer_color_t){0.68f, 0.68f, 0.68f, 1.0f};
	ui->shadow_color = (renderer_color_t){0.0f, 0.0f, 0.0f, 0.9f};
}

void debug_ui_set_line_height(debug_ui_t *ui, const float line_height) {
	if (ui == NULL || line_height <= 0.0f) { return; }

	ui->line_height = line_height;
}

void debug_ui_set_label_width(debug_ui_t *ui, const float label_width) {
	if (ui == NULL || label_width < 0.0f) { return; }

	ui->label_width = label_width;
}

void debug_ui_draw_text(debug_ui_t *ui, const char *text) {
	if (ui == NULL || ui->renderer == NULL || text == NULL) { return; }

	draw_shadowed_text(ui, ui->x, ui->y, ui->text_color, text);
	ui->y += ui->line_height;
}

void debug_ui_draw_row(debug_ui_t *ui, const char *label, const char *value) {
	if (ui == NULL || ui->renderer == NULL || label == NULL ||
	    value == NULL) {
		return;
	}

	draw_shadowed_text(ui, ui->x, ui->y, ui->label_color, label);
	draw_shadowed_text(ui, ui->x + ui->label_width, ui->y, ui->text_color,
			   value);
	ui->y += ui->line_height;
}

void debug_ui_add_spacing(debug_ui_t *ui, const float spacing) {
	if (ui == NULL || spacing < 0.0f) { return; }

	ui->y += spacing;
}

static void draw_shadowed_text(const debug_ui_t *ui,
			       const float x,
			       const float y,
			       const renderer_color_t color,
			       const char *text) {
	renderer_draw_text(ui->renderer, x + 1.0f, y + 1.0f, ui->scale,
			   ui->shadow_color, text);
	renderer_draw_text(ui->renderer, x, y, ui->scale, color, text);
}
