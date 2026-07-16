/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_MAP_KEYVALUES_H
#define VOLUME_MAP_KEYVALUES_H

#include <stdbool.h>
#include <stddef.h>

typedef struct keyvalues_document keyvalues_document_t;
typedef struct keyvalues_node keyvalues_node_t;

keyvalues_document_t *
keyvalues_parse(const char *source, char *error, size_t error_size);
keyvalues_document_t *
keyvalues_load(const char *path, char *error, size_t error_size);
void keyvalues_destroy(keyvalues_document_t *document);
const keyvalues_node_t *
keyvalues_get_root(const keyvalues_document_t *document);
const char *keyvalues_node_get_key(const keyvalues_node_t *node);
const char *keyvalues_node_get_value(const keyvalues_node_t *node);
bool keyvalues_node_is_block(const keyvalues_node_t *node);
size_t keyvalues_node_get_child_count(const keyvalues_node_t *node);
const keyvalues_node_t *keyvalues_node_get_child(const keyvalues_node_t *node,
						 size_t index);
const keyvalues_node_t *keyvalues_node_find_child(const keyvalues_node_t *node,
						  const char *key);

#endif