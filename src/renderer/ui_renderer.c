/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "renderer/ui_renderer.h"

#include "core/path.h"
#include "renderer/shader.h"
#include <epoxy/gl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

enum {
	FONT_FIRST_CHARACTER = 32,
	FONT_CHARACTER_COUNT = 64,
	FONT_COLUMNS = 8,
	FONT_CELL_WIDTH = 6,
	FONT_CELL_HEIGHT = 8,
	FONT_WIDTH = FONT_COLUMNS * FONT_CELL_WIDTH,
	FONT_HEIGHT = 8 * FONT_CELL_HEIGHT,
};

typedef struct ui_vertex {
	float x;
	float y;
	float u;
	float v;
	float r;
	float g;
	float b;
	float a;
} ui_vertex_t;

struct ui_renderer {
	shader_t *shader;
	GLuint vertex_array;
	GLuint vertex_buffer;
	GLuint font_texture;
	ui_vertex_t *vertices;
	size_t vertex_count;
	size_t vertex_capacity;
};

static bool reserve_vertices(ui_renderer_t *renderer, size_t count);
static void append_quad(ui_renderer_t *renderer,
			float x,
			float y,
			float width,
			float height,
			float u0,
			float v0,
			float u1,
			float v1,
			renderer_color_t color);
static void glyph_rows(char character, uint8_t rows[7]);
static bool create_font_texture(ui_renderer_t *renderer);
static shader_t *load_ui_shader(void);

ui_renderer_t *ui_renderer_create(void) {
	ui_renderer_t *renderer;

	renderer = calloc(1, sizeof(*renderer));
	if (renderer == NULL) { return NULL; }

	renderer->shader = load_ui_shader();
	if (renderer->shader == NULL || !create_font_texture(renderer)) {
		ui_renderer_destroy(renderer);
		return NULL;
	}

	glGenVertexArrays(1, &renderer->vertex_array);
	glGenBuffers(1, &renderer->vertex_buffer);
	glBindVertexArray(renderer->vertex_array);
	glBindBuffer(GL_ARRAY_BUFFER, renderer->vertex_buffer);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ui_vertex_t),
			      (const void *)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ui_vertex_t),
			      (const void *)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(ui_vertex_t),
			      (const void *)(4 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return renderer;
}

void ui_renderer_destroy(ui_renderer_t *renderer) {
	if (renderer == NULL) { return; }

	if (renderer->font_texture != 0) {
		glDeleteTextures(1, &renderer->font_texture);
	}
	if (renderer->vertex_buffer != 0) {
		glDeleteBuffers(1, &renderer->vertex_buffer);
	}
	if (renderer->vertex_array != 0) {
		glDeleteVertexArrays(1, &renderer->vertex_array);
	}
	shader_destroy(renderer->shader);
	free(renderer->vertices);
	free(renderer);
}

void ui_renderer_begin_frame(ui_renderer_t *renderer) {
	if (renderer != NULL) { renderer->vertex_count = 0; }
}

void ui_renderer_draw_rectangle(ui_renderer_t *renderer,
				const float x,
				const float y,
				const float width,
				const float height,
				const renderer_color_t color) {
	const float u = 0.5f / (float)FONT_WIDTH;
	const float v = 0.5f / (float)FONT_HEIGHT;

	if (renderer == NULL || width <= 0.0f || height <= 0.0f) { return; }
	append_quad(renderer, x, y, width, height, u, v, u, v, color);
}

void ui_renderer_draw_text(ui_renderer_t *renderer,
			   float x,
			   float y,
			   const float scale,
			   const renderer_color_t color,
			   const char *text) {
	const float start_x = x;
	unsigned int index;
	unsigned int column;
	unsigned int row;
	unsigned char character;
	float u0;
	float v0;
	float u1;
	float v1;

	if (renderer == NULL || text == NULL || scale <= 0.0f) { return; }

	while (*text != '\0') {
		character = (unsigned char)*text++;
		if (character == '\n') {
			x = start_x;
			y += FONT_CELL_HEIGHT * scale;
			continue;
		}
		if (character >= 'a' && character <= 'z') {
			character = (unsigned char)(character - 'a' + 'A');
		}
		if (character < FONT_FIRST_CHARACTER ||
		    character >= FONT_FIRST_CHARACTER + FONT_CHARACTER_COUNT) {
			character = '?';
		}
		if (character != ' ') {
			index = character - FONT_FIRST_CHARACTER;
			column = index % FONT_COLUMNS;
			row = index / FONT_COLUMNS;
			u0 = (float)(column * FONT_CELL_WIDTH) / FONT_WIDTH;
			v0 = (float)(row * FONT_CELL_HEIGHT) / FONT_HEIGHT;
			u1 = (float)(column * FONT_CELL_WIDTH + 5) / FONT_WIDTH;
			v1 = (float)(row * FONT_CELL_HEIGHT + 7) / FONT_HEIGHT;
			append_quad(renderer, x, y, 5.0f * scale, 7.0f * scale,
				    u0, v0, u1, v1, color);
		}
		x += FONT_CELL_WIDTH * scale;
	}
}

void ui_renderer_flush(ui_renderer_t *renderer,
		       const int width,
		       const int height) {
	if (renderer == NULL || renderer->vertex_count == 0 || width <= 0 ||
	    height <= 0) {
		return;
	}

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	shader_bind(renderer->shader);
	shader_set_vec2(renderer->shader, "screen_size", (float)width,
			(float)height);
	shader_set_int(renderer->shader, "font_texture", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderer->font_texture);
	glBindVertexArray(renderer->vertex_array);
	glBindBuffer(GL_ARRAY_BUFFER, renderer->vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER,
		     renderer->vertex_count * sizeof(*renderer->vertices),
		     renderer->vertices, GL_STREAM_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, (GLsizei)renderer->vertex_count);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	shader_unbind();
	glDisable(GL_BLEND);
}

static bool reserve_vertices(ui_renderer_t *renderer, const size_t count) {
	size_t capacity;
	ui_vertex_t *vertices;

	if (count <= renderer->vertex_capacity) { return true; }
	capacity = renderer->vertex_capacity == 0 ? 1024
						  : renderer->vertex_capacity;
	while (capacity < count) {
		if (capacity > SIZE_MAX / 2) { return false; }
		capacity *= 2;
	}
	vertices = realloc(renderer->vertices, capacity * sizeof(*vertices));
	if (vertices == NULL) { return false; }
	renderer->vertices = vertices;
	renderer->vertex_capacity = capacity;
	return true;
}

static void append_quad(ui_renderer_t *renderer,
			const float x,
			const float y,
			const float width,
			const float height,
			const float u0,
			const float v0,
			const float u1,
			const float v1,
			const renderer_color_t color) {
	ui_vertex_t quad[6];

	if (!reserve_vertices(renderer, renderer->vertex_count + 6)) { return; }
	quad[0] =
		(ui_vertex_t){x, y, u0, v0, color.r, color.g, color.b, color.a};
	quad[1] = (ui_vertex_t){x + width, y,	    u1,	     v0,
				color.r,   color.g, color.b, color.a};
	quad[2] = (ui_vertex_t){x + width, y + height, u1,	v1,
				color.r,   color.g,    color.b, color.a};
	quad[3] = quad[0];
	quad[4] = quad[2];
	quad[5] = (ui_vertex_t){x,	 y + height, u0,      v1,
				color.r, color.g,    color.b, color.a};
	memcpy(&renderer->vertices[renderer->vertex_count], quad, sizeof(quad));
	renderer->vertex_count += 6;
}

static bool create_font_texture(ui_renderer_t *renderer) {
	uint8_t pixels[FONT_WIDTH * FONT_HEIGHT] = {0};
	uint8_t rows[7];
	unsigned int character;
	unsigned int index;
	unsigned int cell_x;
	unsigned int cell_y;
	unsigned int x;
	unsigned int y;

	pixels[0] = 255;
	for (character = FONT_FIRST_CHARACTER + 1;
	     character < FONT_FIRST_CHARACTER + FONT_CHARACTER_COUNT;
	     character++) {
		glyph_rows((char)character, rows);
		index = character - FONT_FIRST_CHARACTER;
		cell_x = (index % FONT_COLUMNS) * FONT_CELL_WIDTH;
		cell_y = (index / FONT_COLUMNS) * FONT_CELL_HEIGHT;
		for (y = 0; y < 7; y++) {
			for (x = 0; x < 5; x++) {
				if ((rows[y] & (1u << (4 - x))) != 0) {
					pixels[(cell_y + y) * FONT_WIDTH +
					       cell_x + x] = 255;
				}
			}
		}
	}

	glGenTextures(1, &renderer->font_texture);
	glBindTexture(GL_TEXTURE_2D, renderer->font_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, FONT_WIDTH, FONT_HEIGHT, 0,
		     GL_RED, GL_UNSIGNED_BYTE, pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
	return renderer->font_texture != 0;
}

#define SET_GLYPH(a, b, c, d, e, f, g) \
	do {                           \
		rows[0] = (a);         \
		rows[1] = (b);         \
		rows[2] = (c);         \
		rows[3] = (d);         \
		rows[4] = (e);         \
		rows[5] = (f);         \
		rows[6] = (g);         \
	} while (0)

static void glyph_rows(const char character, uint8_t rows[7]) {
	SET_GLYPH(0, 0, 0, 0, 0, 0, 0);
	switch (character) {
	case '!': SET_GLYPH(4, 4, 4, 4, 4, 0, 4); break;
	case '"': SET_GLYPH(10, 10, 10, 0, 0, 0, 0); break;
	case '#': SET_GLYPH(10, 31, 10, 10, 31, 10, 0); break;
	case '%': SET_GLYPH(17, 2, 4, 8, 17, 0, 0); break;
	case '&': SET_GLYPH(12, 18, 20, 8, 21, 18, 13); break;
	case '\'': SET_GLYPH(4, 4, 8, 0, 0, 0, 0); break;
	case '(': SET_GLYPH(2, 4, 8, 8, 8, 4, 2); break;
	case ')': SET_GLYPH(8, 4, 2, 2, 2, 4, 8); break;
	case '*': SET_GLYPH(0, 21, 14, 31, 14, 21, 0); break;
	case '+': SET_GLYPH(0, 4, 4, 31, 4, 4, 0); break;
	case ',': SET_GLYPH(0, 0, 0, 0, 4, 4, 8); break;
	case '-': SET_GLYPH(0, 0, 0, 31, 0, 0, 0); break;
	case '.': SET_GLYPH(0, 0, 0, 0, 0, 12, 12); break;
	case '/': SET_GLYPH(1, 2, 4, 8, 16, 0, 0); break;
	case '0': SET_GLYPH(14, 17, 19, 21, 25, 17, 14); break;
	case '1': SET_GLYPH(4, 12, 4, 4, 4, 4, 14); break;
	case '2': SET_GLYPH(14, 17, 1, 2, 4, 8, 31); break;
	case '3': SET_GLYPH(30, 1, 1, 14, 1, 1, 30); break;
	case '4': SET_GLYPH(2, 6, 10, 18, 31, 2, 2); break;
	case '5': SET_GLYPH(31, 16, 16, 30, 1, 1, 30); break;
	case '6': SET_GLYPH(14, 16, 16, 30, 17, 17, 14); break;
	case '7': SET_GLYPH(31, 1, 2, 4, 8, 8, 8); break;
	case '8': SET_GLYPH(14, 17, 17, 14, 17, 17, 14); break;
	case '9': SET_GLYPH(14, 17, 17, 15, 1, 1, 14); break;
	case ':': SET_GLYPH(0, 12, 12, 0, 12, 12, 0); break;
	case ';': SET_GLYPH(0, 12, 12, 0, 4, 4, 8); break;
	case '<': SET_GLYPH(2, 4, 8, 16, 8, 4, 2); break;
	case '=': SET_GLYPH(0, 0, 31, 0, 31, 0, 0); break;
	case '>': SET_GLYPH(8, 4, 2, 1, 2, 4, 8); break;
	case '?': SET_GLYPH(14, 17, 1, 2, 4, 0, 4); break;
	case '@': SET_GLYPH(14, 17, 23, 21, 23, 16, 14); break;
	case 'A': SET_GLYPH(14, 17, 17, 31, 17, 17, 17); break;
	case 'B': SET_GLYPH(30, 17, 17, 30, 17, 17, 30); break;
	case 'C': SET_GLYPH(14, 17, 16, 16, 16, 17, 14); break;
	case 'D': SET_GLYPH(30, 17, 17, 17, 17, 17, 30); break;
	case 'E': SET_GLYPH(31, 16, 16, 30, 16, 16, 31); break;
	case 'F': SET_GLYPH(31, 16, 16, 30, 16, 16, 16); break;
	case 'G': SET_GLYPH(14, 17, 16, 23, 17, 17, 15); break;
	case 'H': SET_GLYPH(17, 17, 17, 31, 17, 17, 17); break;
	case 'I': SET_GLYPH(14, 4, 4, 4, 4, 4, 14); break;
	case 'J': SET_GLYPH(7, 2, 2, 2, 2, 18, 12); break;
	case 'K': SET_GLYPH(17, 18, 20, 24, 20, 18, 17); break;
	case 'L': SET_GLYPH(16, 16, 16, 16, 16, 16, 31); break;
	case 'M': SET_GLYPH(17, 27, 21, 21, 17, 17, 17); break;
	case 'N': SET_GLYPH(17, 25, 21, 19, 17, 17, 17); break;
	case 'O': SET_GLYPH(14, 17, 17, 17, 17, 17, 14); break;
	case 'P': SET_GLYPH(30, 17, 17, 30, 16, 16, 16); break;
	case 'Q': SET_GLYPH(14, 17, 17, 17, 21, 18, 13); break;
	case 'R': SET_GLYPH(30, 17, 17, 30, 20, 18, 17); break;
	case 'S': SET_GLYPH(15, 16, 16, 14, 1, 1, 30); break;
	case 'T': SET_GLYPH(31, 4, 4, 4, 4, 4, 4); break;
	case 'U': SET_GLYPH(17, 17, 17, 17, 17, 17, 14); break;
	case 'V': SET_GLYPH(17, 17, 17, 17, 17, 10, 4); break;
	case 'W': SET_GLYPH(17, 17, 17, 21, 21, 21, 10); break;
	case 'X': SET_GLYPH(17, 17, 10, 4, 10, 17, 17); break;
	case 'Y': SET_GLYPH(17, 17, 10, 4, 4, 4, 4); break;
	case 'Z': SET_GLYPH(31, 1, 2, 4, 8, 16, 31); break;
	case '[': SET_GLYPH(14, 8, 8, 8, 8, 8, 14); break;
	case '\\': SET_GLYPH(16, 8, 4, 2, 1, 0, 0); break;
	case ']': SET_GLYPH(14, 2, 2, 2, 2, 2, 14); break;
	case '^': SET_GLYPH(4, 10, 17, 0, 0, 0, 0); break;
	case '_': SET_GLYPH(0, 0, 0, 0, 0, 0, 31); break;
	default: break;
	}
}

#undef SET_GLYPH

static shader_t *load_ui_shader(void) {
	char *vertex_path;
	char *fragment_path;
	shader_t *shader;

	vertex_path = path_from_executable("assets/engine/shaders/ui.vert");
	fragment_path = path_from_executable("assets/engine/shaders/ui.frag");
	if (vertex_path == NULL || fragment_path == NULL) {
		free(vertex_path);
		free(fragment_path);
		return NULL;
	}
	shader = shader_create(vertex_path, fragment_path);
	free(vertex_path);
	free(fragment_path);
	return shader;
}
