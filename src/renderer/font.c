/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

#include "renderer/font_internal.h"
#include <epoxy/gl.h>
#include <stdio.h>
#include <stdlib.h>

enum {
	FONT_ATLAS_WIDTH = 1024,
	FONT_ATLAS_HEIGHT = 1024,
};

static const float font_baked_height = 64.0f;

static unsigned char *read_file(const char *path, size_t *size);

renderer_font_t *renderer_font_create(renderer_t *renderer, const char *path) {
	stbtt_bakedchar baked[RENDERER_FONT_CHARACTER_COUNT];
	renderer_font_t *font;
	unsigned char *pixels;
	unsigned char *data;
	stbtt_fontinfo info;
	float scale;
	size_t data_size;
	int ascent;
	int index;

	if (renderer == NULL || path == NULL || path[0] == '\0') {
		return NULL;
	}
	data = read_file(path, &data_size);
	if (data == NULL || data_size == 0 ||
	    !stbtt_InitFont(&info, data,
			    stbtt_GetFontOffsetForIndex(data, 0))) {
		free(data);
		return NULL;
	}
	pixels = calloc(FONT_ATLAS_WIDTH * FONT_ATLAS_HEIGHT, 1);
	font = calloc(1, sizeof(*font));
	if (pixels == NULL || font == NULL) {
		free(pixels);
		free(font);
		free(data);
		return NULL;
	}
	if (stbtt_BakeFontBitmap(data, 0, font_baked_height, pixels,
				 FONT_ATLAS_WIDTH, FONT_ATLAS_HEIGHT,
				 RENDERER_FONT_FIRST_CHARACTER,
				 RENDERER_FONT_CHARACTER_COUNT, baked) <= 0) {
		free(pixels);
		free(font);
		free(data);
		return NULL;
	}
	stbtt_GetFontVMetrics(&info, &ascent, NULL, NULL);
	scale = stbtt_ScaleForPixelHeight(&info, font_baked_height);
	font->atlas_width = FONT_ATLAS_WIDTH;
	font->atlas_height = FONT_ATLAS_HEIGHT;
	font->baked_height = font_baked_height;
	font->ascent = (float)ascent * scale;
	for (index = 0; index < RENDERER_FONT_CHARACTER_COUNT; index++) {
		font->glyphs[index].atlas_x0 = (float)baked[index].x0;
		font->glyphs[index].atlas_y0 = (float)baked[index].y0;
		font->glyphs[index].atlas_x1 = (float)baked[index].x1;
		font->glyphs[index].atlas_y1 = (float)baked[index].y1;
		font->glyphs[index].x_offset = baked[index].xoff;
		font->glyphs[index].y_offset = baked[index].yoff;
		font->glyphs[index].x_advance = baked[index].xadvance;
	}
	glGenTextures(1, &font->texture);
	glBindTexture(GL_TEXTURE_2D, font->texture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, FONT_ATLAS_WIDTH,
		     FONT_ATLAS_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
	free(pixels);
	free(data);
	if (font->texture == 0) {
		free(font);
		return NULL;
	}
	return font;
}

void renderer_font_destroy(renderer_font_t *font) {
	if (font == NULL) { return; }
	if (font->texture != 0) { glDeleteTextures(1, &font->texture); }
	free(font);
}

float renderer_font_measure_text(const renderer_font_t *font,
				 const float height,
				 const char *text) {
	float maximum_width;
	float line_width;
	float scale;
	unsigned char character;

	if (font == NULL || height <= 0.0f || text == NULL) { return 0.0f; }
	maximum_width = 0.0f;
	line_width = 0.0f;
	scale = height / font->baked_height;
	while (*text != '\0') {
		character = (unsigned char)*text++;
		if (character == '\n') {
			if (line_width > maximum_width) {
				maximum_width = line_width;
			}
			line_width = 0.0f;
			continue;
		}
		if (character < RENDERER_FONT_FIRST_CHARACTER ||
		    character >= 128) {
			character = '?';
		}
		line_width +=
			font->glyphs[character - RENDERER_FONT_FIRST_CHARACTER]
				.x_advance *
			scale;
	}
	return line_width > maximum_width ? line_width : maximum_width;
}

static unsigned char *read_file(const char *path, size_t *size) {
	unsigned char *data;
	long length;
	FILE *file;

	if (size != NULL) { *size = 0; }
	file = fopen(path, "rb");
	if (file == NULL || fseek(file, 0, SEEK_END) != 0) {
		if (file != NULL) { fclose(file); }
		return NULL;
	}
	length = ftell(file);
	if (length <= 0 || fseek(file, 0, SEEK_SET) != 0) {
		fclose(file);
		return NULL;
	}
	data = malloc((size_t)length);
	if (data == NULL ||
	    fread(data, 1, (size_t)length, file) != (size_t)length) {
		free(data);
		fclose(file);
		return NULL;
	}
	fclose(file);
	if (size != NULL) { *size = (size_t)length; }
	return data;
}
