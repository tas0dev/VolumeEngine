/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_RENDERER_FONT_INTERNAL_H
#define VOLUME_RENDERER_FONT_INTERNAL_H

#include "renderer/font.h"

#define RENDERER_FONT_FIRST_CHARACTER 32
#define RENDERER_FONT_CHARACTER_COUNT 96

typedef struct renderer_font_glyph {
	float atlas_x0;
	float atlas_y0;
	float atlas_x1;
	float atlas_y1;
	float x_offset;
	float y_offset;
	float x_advance;
} renderer_font_glyph_t;

struct renderer_font {
	unsigned int texture;
	int atlas_width;
	int atlas_height;
	float baked_height;
	float ascent;
	renderer_font_glyph_t glyphs[RENDERER_FONT_CHARACTER_COUNT];
};

#endif
