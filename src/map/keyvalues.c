/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#include "map/keyvalues.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct keyvalues_node {
	char *key;
	char *value;
	keyvalues_node_t **children;
	size_t child_count;
	size_t child_capacity;
};

struct keyvalues_document {
	keyvalues_node_t *root;
};

typedef struct parser {
	const char *source;
	size_t position;
	size_t line;
	char *error;
	size_t error_size;
} parser_t;

static void set_error(char *error, size_t error_size, const char *format, ...);
static void parser_set_error(parser_t *parser, const char *format, ...);
static char *duplicate_string(const char *string);
static keyvalues_node_t *node_create(const char *key, const char *value);
static void node_destroy(keyvalues_node_t *node);
static bool node_add_child(keyvalues_node_t *node, keyvalues_node_t *child);
static void parser_skip_ignored(parser_t *parser);
static bool
token_append(char **buffer, size_t *length, size_t *capacity, char character);
static char *parser_parse_quoted_token(parser_t *parser);
static char *parser_parse_bare_token(parser_t *parser);
static char *parser_parse_token(parser_t *parser);
static bool parser_parse_nodes(parser_t *parser,
			       keyvalues_node_t *parent,
			       bool expect_closing_brace);

static void
set_error(char *error, const size_t error_size, const char *format, ...) {
	va_list arguments;

	if (error == NULL || error_size == 0) { return; }

	va_start(arguments, format);
	vsnprintf(error, error_size, format, arguments);
	va_end(arguments);
}

static void parser_set_error(parser_t *parser, const char *format, ...) {
	va_list arguments;
	char message[512];

	if (parser == NULL || parser->error == NULL ||
	    parser->error_size == 0) {
		return;
	}

	va_start(arguments, format);
	vsnprintf(message, sizeof(message), format, arguments);
	va_end(arguments);

	snprintf(parser->error, parser->error_size, "line %zu: %s",
		 parser->line, message);
}

static char *duplicate_string(const char *string) {
	char *copy;
	size_t length;

	length = strlen(string);
	copy = malloc(length + 1);
	if (copy == NULL) { return NULL; }

	memcpy(copy, string, length + 1);

	return copy;
}

static keyvalues_node_t *node_create(const char *key, const char *value) {
	keyvalues_node_t *node;

	node = calloc(1, sizeof(*node));
	if (node == NULL) { return NULL; }

	if (key != NULL) {
		node->key = duplicate_string(key);
		if (node->key == NULL) {
			free(node);
			return NULL;
		}
	}

	if (value != NULL) {
		node->value = duplicate_string(value);
		if (node->value == NULL) {
			free(node->key);
			free(node);
			return NULL;
		}
	}

	return node;
}

static void node_destroy(keyvalues_node_t *node) {
	size_t index;

	if (node == NULL) { return; }

	for (index = 0; index < node->child_count; index++) {
		node_destroy(node->children[index]);
	}

	free(node->children);
	free(node->value);
	free(node->key);
	free(node);
}

static bool node_add_child(keyvalues_node_t *node, keyvalues_node_t *child) {
	keyvalues_node_t **children;
	size_t capacity;

	if (node == NULL) { return false; }

	if (node->child_count == node->child_capacity) {
		capacity = node->child_capacity == 0 ? 8
						     : node->child_capacity * 2;

		if (capacity < node->child_capacity ||
		    capacity > SIZE_MAX / sizeof(*node->children)) {
			return false;
		}

		children = realloc(node->children,
				   capacity * sizeof(*node->children));
		if (children == NULL) { return false; }

		node->children = children;
		node->child_capacity = capacity;
	}

	node->children[node->child_count] = child;
	node->child_count++;

	return true;
}

static void parser_skip_ignored(parser_t *parser) {
	char character;

	while (true) {
		character = parser->source[parser->position];

		while (character != '\0' && isspace((unsigned char)character)) {
			if (character == '\n') { parser->line++; }

			parser->position++;
			character = parser->source[parser->position];
		}

		if (parser->source[parser->position] != '/' ||
		    parser->source[parser->position + 1] != '/') {
			return;
		}

		parser->position += 2;

		while (parser->source[parser->position] != '\0' &&
		       parser->source[parser->position] != '\n') {
			parser->position++;
		}
	}
}

static bool token_append(char **buffer,
			 size_t *length,
			 size_t *capacity,
			 const char character) {
	char *new_buffer;
	size_t new_capacity;

	if (*length + 1 >= *capacity) {
		new_capacity = *capacity == 0 ? 32 : *capacity * 2;

		if (new_capacity < *capacity) { return false; }

		new_buffer = realloc(*buffer, new_capacity);
		if (new_buffer == NULL) { return false; }

		*buffer = new_buffer;
		*capacity = new_capacity;
	}

	(*buffer)[*length] = character;
	(*length)++;
	(*buffer)[*length] = '\0';

	return true;
}

static char *parser_parse_quoted_token(parser_t *parser) {
	char *buffer;
	size_t length;
	size_t capacity;
	char character;
	char escaped;

	buffer = NULL;
	length = 0;
	capacity = 0;

	parser->position++;

	while (true) {
		character = parser->source[parser->position];

		if (character == '\0') {
			parser_set_error(parser, "unterminated quoted string");
			free(buffer);
			return NULL;
		}

		if (character == '"') {
			parser->position++;

			if (buffer == NULL) { buffer = calloc(1, 1); }

			return buffer;
		}

		if (character == '\n') { parser->line++; }

		if (character != '\\') {
			if (!token_append(&buffer, &length, &capacity,
					  character)) {
				parser_set_error(parser, "out of memory");
				free(buffer);
				return NULL;
			}

			parser->position++;
			continue;
		}

		parser->position++;
		escaped = parser->source[parser->position];

		if (escaped == '\0') {
			parser_set_error(parser,
					 "unterminated escape sequence");
			free(buffer);
			return NULL;
		}

		switch (escaped) {
		case '"': character = '"'; break;
		case '\\': character = '\\'; break;
		case 'n': character = '\n'; break;
		case 'r': character = '\r'; break;
		case 't': character = '\t'; break;
		default:
			if (!token_append(&buffer, &length, &capacity, '\\')) {
				parser_set_error(parser, "out of memory");
				free(buffer);
				return NULL;
			}

			character = escaped;
			break;
		}

		if (!token_append(&buffer, &length, &capacity, character)) {
			parser_set_error(parser, "out of memory");
			free(buffer);
			return NULL;
		}

		parser->position++;
	}
}

static char *parser_parse_bare_token(parser_t *parser) {
	const char *start;
	char *token;
	size_t length;
	char character;

	start = &parser->source[parser->position];

	while (true) {
		character = parser->source[parser->position];

		if (character == '\0' || isspace((unsigned char)character) ||
		    character == '{' || character == '}') {
			break;
		}

		parser->position++;
	}

	length = (size_t)(&parser->source[parser->position] - start);
	if (length == 0) {
		parser_set_error(parser, "expected token");
		return NULL;
	}

	token = malloc(length + 1);
	if (token == NULL) {
		parser_set_error(parser, "out of memory");
		return NULL;
	}

	memcpy(token, start, length);
	token[length] = '\0';

	return token;
}

static char *parser_parse_token(parser_t *parser) {
	char character;

	parser_skip_ignored(parser);
	character = parser->source[parser->position];

	if (character == '\0' || character == '{' || character == '}') {
		parser_set_error(parser, "expected token");
		return NULL;
	}

	if (character == '"') { return parser_parse_quoted_token(parser); }

	return parser_parse_bare_token(parser);
}

static bool parser_parse_nodes(parser_t *parser,
			       keyvalues_node_t *parent,
			       const bool expect_closing_brace) {
	keyvalues_node_t *node;
	char *key;
	char *value;
	char character;

	while (true) {
		parser_skip_ignored(parser);
		character = parser->source[parser->position];

		if (character == '\0') {
			if (expect_closing_brace) {
				parser_set_error(parser,
						 "expected closing brace");
				return false;
			}

			return true;
		}

		if (character == '}') {
			if (!expect_closing_brace) {
				parser_set_error(parser,
						 "unexpected closing brace");
				return false;
			}

			parser->position++;
			return true;
		}

		key = parser_parse_token(parser);
		if (key == NULL) { return false; }

		parser_skip_ignored(parser);
		character = parser->source[parser->position];

		if (character == '{') {
			parser->position++;

			node = node_create(key, NULL);
			free(key);

			if (node == NULL) {
				parser_set_error(parser, "out of memory");
				return false;
			}

			if (!node_add_child(parent, node)) {
				parser_set_error(parser, "out of memory");
				node_destroy(node);
				return false;
			}

			if (!parser_parse_nodes(parser, node, true)) {
				return false;
			}

			continue;
		}

		value = parser_parse_token(parser);
		if (value == NULL) {
			free(key);
			return false;
		}

		node = node_create(key, value);
		free(value);
		free(key);

		if (node == NULL) {
			parser_set_error(parser, "out of memory");
			return false;
		}

		if (!node_add_child(parent, node)) {
			parser_set_error(parser, "out of memory");
			node_destroy(node);
			return false;
		}
	}
}

keyvalues_document_t *
keyvalues_parse(const char *source, char *error, const size_t error_size) {
	keyvalues_document_t *document;
	parser_t parser;

	if (error != NULL && error_size > 0) { error[0] = '\0'; }

	if (source == NULL) {
		set_error(error, error_size, "source is null");
		return NULL;
	}

	document = calloc(1, sizeof(*document));
	if (document == NULL) {
		set_error(error, error_size, "out of memory");
		return NULL;
	}

	document->root = node_create(NULL, NULL);
	if (document->root == NULL) {
		set_error(error, error_size, "out of memory");
		free(document);
		return NULL;
	}

	parser.source = source;
	parser.position = 0;
	parser.line = 1;
	parser.error = error;
	parser.error_size = error_size;

	if (!parser_parse_nodes(&parser, document->root, false)) {
		keyvalues_destroy(document);
		return NULL;
	}

	return document;
}

keyvalues_document_t *
keyvalues_load(const char *path, char *error, const size_t error_size) {
	keyvalues_document_t *document;
	FILE *file;
	char *source;
	long file_size;
	size_t read_size;

	if (error != NULL && error_size > 0) { error[0] = '\0'; }

	if (path == NULL) {
		set_error(error, error_size, "path is null");
		return NULL;
	}

	file = fopen(path, "rb");
	if (file == NULL) {
		set_error(error, error_size, "failed to open %s", path);
		return NULL;
	}

	if (fseek(file, 0, SEEK_END) != 0) {
		set_error(error, error_size, "failed to seek %s", path);
		fclose(file);
		return NULL;
	}

	file_size = ftell(file);
	if (file_size < 0 || (uintmax_t)file_size > (uintmax_t)(SIZE_MAX - 1)) {
		set_error(error, error_size, "invalid file size for %s", path);
		fclose(file);
		return NULL;
	}

	if (fseek(file, 0, SEEK_SET) != 0) {
		set_error(error, error_size, "failed to seek %s", path);
		fclose(file);
		return NULL;
	}

	source = malloc((size_t)file_size + 1);
	if (source == NULL) {
		set_error(error, error_size, "out of memory");
		fclose(file);
		return NULL;
	}

	read_size = fread(source, 1, (size_t)file_size, file);
	fclose(file);

	if (read_size != (size_t)file_size) {
		set_error(error, error_size, "failed to read %s", path);
		free(source);
		return NULL;
	}

	source[read_size] = '\0';
	document = keyvalues_parse(source, error, error_size);
	free(source);

	return document;
}

void keyvalues_destroy(keyvalues_document_t *document) {
	if (document == NULL) { return; }

	node_destroy(document->root);
	free(document);
}

const keyvalues_node_t *
keyvalues_get_root(const keyvalues_document_t *document) {
	if (document == NULL) { return NULL; }

	return document->root;
}

const char *keyvalues_node_get_key(const keyvalues_node_t *node) {
	if (node == NULL) { return NULL; }

	return node->key;
}

const char *keyvalues_node_get_value(const keyvalues_node_t *node) {
	if (node == NULL) { return NULL; }

	return node->value;
}

bool keyvalues_node_is_block(const keyvalues_node_t *node) {
	if (node == NULL) { return false; }

	return node->value == NULL;
}

size_t keyvalues_node_get_child_count(const keyvalues_node_t *node) {
	if (node == NULL) { return 0; }

	return node->child_count;
}

const keyvalues_node_t *keyvalues_node_get_child(const keyvalues_node_t *node,
						 const size_t index) {
	if (node == NULL || index >= node->child_count) { return NULL; }

	return node->children[index];
}

const keyvalues_node_t *keyvalues_node_find_child(const keyvalues_node_t *node,
						  const char *key) {
	size_t index;

	if (node == NULL || key == NULL) { return NULL; }

	for (index = 0; index < node->child_count; index++) {
		if (node->children[index]->key != NULL &&
		    strcmp(node->children[index]->key, key) == 0) {
			return node->children[index];
		}
	}

	return NULL;
}
