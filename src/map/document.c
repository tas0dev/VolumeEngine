/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "map/document.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct map_document_state {
	char *source;
	uint64_t revision;
} map_document_state_t;

struct map_document {
	map_t *map;
	char *path;
	map_document_state_t *undo;
	size_t undo_count;
	size_t undo_capacity;
	map_document_state_t *redo;
	size_t redo_count;
	size_t redo_capacity;
	uint64_t revision;
	uint64_t saved_revision;
	uint64_t next_revision;
	char *transaction_source;
	uint64_t transaction_revision;
	bool transaction_active;
	bool transaction_changed;
};

static char *duplicate_string(const char *text);
static bool
reserve_states(map_document_state_t **states, size_t *capacity, size_t count);
static bool push_state(map_document_state_t **states,
		       size_t *count,
		       size_t *capacity,
		       map_document_state_t state);
static void clear_states(map_document_state_t *states, size_t count);
static bool commit_source(map_document_t *document,
			  char *source,
			  uint64_t previous_revision);
static bool restore_from(map_document_t *document,
			 map_document_state_t **from,
			 size_t *from_count,
			 map_document_state_t **to,
			 size_t *to_count,
			 size_t *to_capacity);

map_document_t *map_document_create(void) {
	map_document_t *document;

	document = calloc(1, sizeof(*document));
	if (document == NULL) { return NULL; }
	document->map = map_create();
	if (document->map == NULL) {
		free(document);
		return NULL;
	}
	document->revision = 1;
	document->saved_revision = 0;
	document->next_revision = 2;
	return document;
}

map_document_t *
map_document_load(const char *path, char *error, const size_t error_size) {
	map_document_t *document;

	if (path == NULL) { return NULL; }
	document = calloc(1, sizeof(*document));
	if (document == NULL) { return NULL; }
	document->map = map_load(path, error, error_size);
	document->path = duplicate_string(path);
	if (document->map == NULL || document->path == NULL) {
		map_document_destroy(document);
		return NULL;
	}
	document->revision = 1;
	document->saved_revision = 1;
	document->next_revision = 2;
	return document;
}

void map_document_destroy(map_document_t *document) {
	if (document == NULL) { return; }
	clear_states(document->undo, document->undo_count);
	clear_states(document->redo, document->redo_count);
	free(document->undo);
	free(document->redo);
	free(document->path);
	free(document->transaction_source);
	map_destroy(document->map);
	free(document);
}

const map_t *map_document_get_map(const map_document_t *document) {
	return document == NULL ? NULL : document->map;
}

bool map_document_is_dirty(const map_document_t *document) {
	return document != NULL &&
	       (document->revision != document->saved_revision ||
		document->transaction_changed);
}

bool map_document_begin_transaction(map_document_t *document) {
	if (document == NULL || document->transaction_active) { return false; }
	document->transaction_source = map_serialize(document->map);
	if (document->transaction_source == NULL) { return false; }
	document->transaction_revision = document->revision;
	document->transaction_active = true;
	document->transaction_changed = false;
	return true;
}

bool map_document_end_transaction(map_document_t *document) {
	char *source;
	uint64_t revision;

	if (document == NULL || !document->transaction_active) { return false; }
	source = document->transaction_source;
	revision = document->transaction_revision;
	document->transaction_source = NULL;
	document->transaction_active = false;
	if (!document->transaction_changed) {
		free(source);
		return true;
	}
	document->transaction_changed = false;
	return commit_source(document, source, revision);
}

bool map_document_cancel_transaction(map_document_t *document) {
	map_t *map;
	char error[256];

	if (document == NULL || !document->transaction_active) { return false; }
	map = map_parse(document->transaction_source, error, sizeof(error));
	if (map == NULL) { return false; }
	map_destroy(document->map);
	document->map = map;
	document->revision = document->transaction_revision;
	free(document->transaction_source);
	document->transaction_source = NULL;
	document->transaction_active = false;
	document->transaction_changed = false;
	return true;
}

bool map_document_set_world_property(map_document_t *document,
				     const char *key,
				     const char *value) {
	map_entity_t *world;
	const char *old_value;
	char *source;
	uint64_t revision;

	if (document == NULL || key == NULL || value == NULL) { return false; }
	world = map_get_mutable_world(document->map);
	if (world == NULL) { return false; }
	old_value = map_entity_get_property(world, key);
	if (old_value != NULL && strcmp(old_value, value) == 0) { return true; }
	source = map_serialize(document->map);
	if (source == NULL) { return false; }
	revision = document->revision;
	if (!map_entity_set_property(world, key, value)) {
		free(source);
		return false;
	}
	return commit_source(document, source, revision);
}

bool map_document_set_entity_property(map_document_t *document,
				      const size_t entity_index,
				      const char *key,
				      const char *value) {
	map_entity_t *entity;
	const char *old_value;
	char *source;
	uint64_t revision;

	if (document == NULL || key == NULL || value == NULL) { return false; }
	entity = map_get_mutable_entity(document->map, entity_index);
	if (entity == NULL) { return false; }
	old_value = map_entity_get_property(entity, key);
	if (old_value != NULL && strcmp(old_value, value) == 0) { return true; }
	source = map_serialize(document->map);
	if (source == NULL) { return false; }
	revision = document->revision;
	if (!map_entity_set_property(entity, key, value)) {
		free(source);
		return false;
	}
	return commit_source(document, source, revision);
}

bool map_document_add_entity_property(map_document_t *document,
				      const size_t entity_index,
				      const char *key,
				      const char *value) {
	map_entity_t *entity;
	char *source;
	uint64_t revision;

	if (document == NULL) { return false; }
	entity = map_get_mutable_entity(document->map, entity_index);
	source = entity == NULL ? NULL : map_serialize(document->map);
	if (source == NULL) { return false; }
	revision = document->revision;
	if (!map_entity_add_property(entity, key, value)) {
		free(source);
		return false;
	}
	return commit_source(document, source, revision);
}

bool map_document_add_entity(map_document_t *document,
			     const char *classname,
			     size_t *entity_index) {
	char *source;
	size_t index;
	uint64_t revision;

	if (document == NULL) { return false; }
	source = map_serialize(document->map);
	if (source == NULL) { return false; }
	revision = document->revision;
	index = map_get_entity_count(document->map);
	if (map_add_entity(document->map, classname) == NULL) {
		free(source);
		return false;
	}
	if (!commit_source(document, source, revision)) { return false; }
	if (entity_index != NULL) { *entity_index = index; }
	return true;
}

bool map_document_remove_entity(map_document_t *document,
				const size_t entity_index) {
	char *source;
	uint64_t revision;

	if (document == NULL ||
	    entity_index >= map_get_entity_count(document->map)) {
		return false;
	}
	source = map_serialize(document->map);
	if (source == NULL) { return false; }
	revision = document->revision;
	if (!map_remove_entity(document->map, entity_index)) {
		free(source);
		return false;
	}
	return commit_source(document, source, revision);
}

bool map_document_undo(map_document_t *document) {
	return document != NULL &&
	       restore_from(document, &document->undo, &document->undo_count,
			    &document->redo, &document->redo_count,
			    &document->redo_capacity);
}

bool map_document_redo(map_document_t *document) {
	return document != NULL &&
	       restore_from(document, &document->redo, &document->redo_count,
			    &document->undo, &document->undo_count,
			    &document->undo_capacity);
}

bool map_document_can_undo(const map_document_t *document) {
	return document != NULL && document->undo_count > 0;
}

bool map_document_can_redo(const map_document_t *document) {
	return document != NULL && document->redo_count > 0;
}

bool map_document_save(map_document_t *document,
		       const char *path,
		       char *error,
		       const size_t error_size) {
	char *copy;
	const char *destination;

	if (document == NULL) { return false; }
	destination = path == NULL ? document->path : path;
	if (destination == NULL) {
		if (error != NULL && error_size > 0) {
			snprintf(error, error_size,
				 "document has no save path");
		}
		return false;
	}
	copy = path == NULL ? NULL : duplicate_string(path);
	if (path != NULL && copy == NULL) { return false; }
	if (!map_save(document->map, destination, error, error_size)) {
		free(copy);
		return false;
	}
	if (path != NULL) {
		free(document->path);
		document->path = copy;
	}
	document->saved_revision = document->revision;
	return true;
}

static bool commit_source(map_document_t *document,
			  char *source,
			  const uint64_t previous_revision) {
	map_document_state_t state;
	map_t *restored;
	char error[256];

	if (document->transaction_active) {
		free(source);
		document->transaction_changed = true;
		return true;
	}

	state.source = source;
	state.revision = previous_revision;
	if (!push_state(&document->undo, &document->undo_count,
			&document->undo_capacity, state)) {
		restored = map_parse(source, error, sizeof(error));
		if (restored != NULL) {
			map_destroy(document->map);
			document->map = restored;
		}
		free(source);
		return false;
	}
	clear_states(document->redo, document->redo_count);
	document->redo_count = 0;
	document->revision = document->next_revision++;
	return true;
}

static bool restore_from(map_document_t *document,
			 map_document_state_t **from,
			 size_t *from_count,
			 map_document_state_t **to,
			 size_t *to_count,
			 size_t *to_capacity) {
	map_document_state_t current;
	map_document_state_t restored;
	map_t *map;
	char error[256];

	if (*from_count == 0) { return false; }
	current.source = map_serialize(document->map);
	current.revision = document->revision;
	if (current.source == NULL ||
	    !push_state(to, to_count, to_capacity, current)) {
		free(current.source);
		return false;
	}
	restored = (*from)[*from_count - 1];
	map = map_parse(restored.source, error, sizeof(error));
	if (map == NULL) {
		free((*to)[*to_count - 1].source);
		(*to_count)--;
		return false;
	}
	(*from_count)--;
	map_destroy(document->map);
	document->map = map;
	document->revision = restored.revision;
	free(restored.source);
	return true;
}

static bool push_state(map_document_state_t **states,
		       size_t *count,
		       size_t *capacity,
		       const map_document_state_t state) {
	if (!reserve_states(states, capacity, *count + 1)) { return false; }
	(*states)[*count] = state;
	(*count)++;
	return true;
}

static bool reserve_states(map_document_state_t **states,
			   size_t *capacity,
			   const size_t count) {
	map_document_state_t *result;
	size_t new_capacity;

	if (count <= *capacity) { return true; }
	new_capacity = *capacity == 0 ? 16 : *capacity * 2;
	if (new_capacity < *capacity ||
	    new_capacity > SIZE_MAX / sizeof(**states)) {
		return false;
	}
	result = realloc(*states, new_capacity * sizeof(**states));
	if (result == NULL) { return false; }
	*states = result;
	*capacity = new_capacity;
	return true;
}

static void clear_states(map_document_state_t *states, const size_t count) {
	size_t index;
	for (index = 0; index < count; index++) {
		free(states[index].source);
	}
}

static char *duplicate_string(const char *text) {
	char *copy;
	size_t length;

	length = strlen(text);
	copy = malloc(length + 1);
	if (copy != NULL) { memcpy(copy, text, length + 1); }
	return copy;
}
