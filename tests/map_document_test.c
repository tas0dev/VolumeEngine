/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "common.h"
#include "map/document.h"
#include "map/map.h"
#include <stdlib.h>
#include <string.h>

static bool test_mutable_map_round_trip(void) {
	map_entity_t *entity;
	const map_entity_t *parsed_entity;
	map_t *map;
	map_t *parsed;
	char *source;
	const char *key;
	const char *value;
	char error[256];

	map = map_create();
	CHECK(map != NULL);
	entity = map_add_entity(map, "logic_relay");
	CHECK(entity != NULL);
	CHECK(map_entity_set_property(entity, "targetname", "relay"));
	CHECK(map_entity_set_property(entity, "description",
				      "say \"hello\"\nnext"));
	CHECK(map_entity_add_property(entity, "OnTrigger", "door,Open,,0,-1"));
	CHECK(map_entity_add_property(entity, "OnTrigger",
				      "sound,PlaySound,,0,-1"));
	source = map_serialize(map);
	CHECK(source != NULL);
	parsed = map_parse(source, error, sizeof(error));
	CHECK(parsed != NULL);
	CHECK(map_get_entity_count(parsed) == 1);
	parsed_entity = map_get_entity(parsed, 0);
	CHECK(strcmp(map_entity_get_property(parsed_entity, "description"),
		     "say \"hello\"\nnext") == 0);
	CHECK(map_entity_get_property_count(parsed_entity) == 5);
	CHECK(map_entity_get_property_at(parsed_entity, 3, &key, &value));
	CHECK(strcmp(key, "OnTrigger") == 0);
	CHECK(strcmp(value, "door,Open,,0,-1") == 0);
	CHECK(map_entity_get_property_at(parsed_entity, 4, &key, &value));
	CHECK(strcmp(key, "OnTrigger") == 0);
	CHECK(strcmp(value, "sound,PlaySound,,0,-1") == 0);

	free(source);
	map_destroy(parsed);
	map_destroy(map);
	return true;
}

static bool test_document_undo_redo_and_save(void) {
	map_document_t *document;
	map_document_t *loaded;
	const map_entity_t *entity;
	size_t index;
	char error[256];

	document = map_document_create();
	CHECK(document != NULL);
	CHECK(map_document_is_dirty(document));
	CHECK(map_document_save(document, TEST_MAP_DOCUMENT_FILE, error,
				sizeof(error)));
	CHECK(!map_document_is_dirty(document));
	CHECK(map_document_set_world_property(document, "skyname", "test_sky"));
	CHECK(map_document_is_dirty(document));
	CHECK(map_document_undo(document));
	CHECK(!map_document_is_dirty(document));
	CHECK(map_document_add_entity(document, "prop_static", &index));
	CHECK(index == 0);
	CHECK(map_document_set_entity_property(document, index, "origin",
					       "1 2 3"));
	CHECK(map_document_add_entity_property(document, index, "OnBreak",
					       "relay,Trigger,,0,-1"));
	CHECK(map_document_is_dirty(document));
	CHECK(map_document_can_undo(document));
	CHECK(map_document_undo(document));
	entity = map_get_entity(map_document_get_map(document), 0);
	CHECK(entity != NULL);
	CHECK(map_entity_get_property(entity, "OnBreak") == NULL);
	CHECK(map_document_undo(document));
	entity = map_get_entity(map_document_get_map(document), 0);
	CHECK(map_entity_get_property(entity, "origin") == NULL);
	CHECK(map_document_redo(document));
	entity = map_get_entity(map_document_get_map(document), 0);
	CHECK(strcmp(map_entity_get_property(entity, "origin"), "1 2 3") == 0);
	CHECK(map_document_save(document, NULL, error, sizeof(error)));
	CHECK(!map_document_is_dirty(document));
	map_document_destroy(document);

	loaded =
		map_document_load(TEST_MAP_DOCUMENT_FILE, error, sizeof(error));
	CHECK(loaded != NULL);
	CHECK(!map_document_is_dirty(loaded));
	CHECK(map_get_entity_count(map_document_get_map(loaded)) == 1);
	map_document_destroy(loaded);
	return true;
}

static bool test_document_transaction(void) {
	map_document_t *document;
	size_t index;

	document = map_document_create();
	CHECK(document != NULL);
	CHECK(map_document_begin_transaction(document));
	CHECK(map_document_add_entity(document, "prop_static", &index));
	CHECK(map_document_set_entity_property(document, index, "origin",
					       "1 2 3"));
	CHECK(map_document_set_entity_property(document, index, "scale",
					       "2 2 2"));
	CHECK(map_document_end_transaction(document));
	CHECK(map_get_entity_count(map_document_get_map(document)) == 1);
	CHECK(map_document_undo(document));
	CHECK(map_get_entity_count(map_document_get_map(document)) == 0);
	CHECK(map_document_redo(document));
	CHECK(map_get_entity_count(map_document_get_map(document)) == 1);
	CHECK(map_document_begin_transaction(document));
	CHECK(map_document_set_entity_property(document, 0, "origin", "9 9 9"));
	CHECK(map_document_cancel_transaction(document));
	CHECK(strcmp(map_entity_get_property(
			     map_get_entity(map_document_get_map(document), 0),
			     "origin"),
		     "1 2 3") == 0);
	map_document_destroy(document);
	return true;
}

int main(void) {
	static const test_case_t tests[] = {
		{"mutable map round trip",	   test_mutable_map_round_trip},
		{"document undo redo and save",
		 test_document_undo_redo_and_save				 },
		{"document transaction",	 test_document_transaction  },
	};

	return test_run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
