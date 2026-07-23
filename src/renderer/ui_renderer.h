/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_RENDERER_UI_RENDERER_H
#define VOLUME_RENDERER_UI_RENDERER_H

#include "renderer/renderer.h"

typedef struct ui_renderer ui_renderer_t;

ui_renderer_t *ui_renderer_create(void);
void ui_renderer_destroy(ui_renderer_t *renderer);
void ui_renderer_begin_frame(ui_renderer_t *renderer);
void ui_renderer_draw_rectangle(ui_renderer_t *renderer,
				float x,
				float y,
				float width,
				float height,
				renderer_color_t color);
void ui_renderer_draw_text(ui_renderer_t *renderer,
			   float x,
			   float y,
			   float scale,
			   renderer_color_t color,
			   const char *text);
void ui_renderer_flush(ui_renderer_t *renderer, int width, int height);

#endif
