/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#include "common.h"
#include "map/keyvalues.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static bool test_basic_document(void) {
	static const char source[] = "world\n"
				     "{\n"
				     "\t\"classname\" \"worldspawn\"\n"
				     "}\n"
				     "entity\n"
				     "{\n"
				     "\t\"classname\" \"prop_static\"\n"
				     "\t\"model\" \"models/crate.obj\"\n"
				     "\t\"origin\" \"1 2 3\"\n"
				     "}\n"
				     "entity\n"
				     "{\n"
				     "\t\"classname\" \"prop_static\"\n"
				     "\t\"origin\" \"4 5 6\"\n"
				     "}\n";
	keyvalues_document_t *document;
	const keyvalues_node_t *root;
	const keyvalues_node_t *world;
	const keyvalues_node_t *entity;
	char error[256];

	document = keyvalues_parse(source, error, sizeof(error));
	CHECK(document != NULL);

	root = keyvalues_get_root(document);
	CHECK(root != NULL);
	CHECK(keyvalues_node_get_child_count(root) == 3);

	world = keyvalues_node_get_child(root, 0);
	CHECK(world != NULL);
	CHECK(keyvalues_node_is_block(world));
	CHECK(strcmp(keyvalues_node_get_key(world), "world") == 0);
	CHECK(strcmp(test_keyvalues_get_value(world, "classname"),
		     "worldspawn") == 0);

	entity = keyvalues_node_get_child(root, 1);
	CHECK(entity != NULL);
	CHECK(keyvalues_node_is_block(entity));
	CHECK(strcmp(keyvalues_node_get_key(entity), "entity") == 0);
	CHECK(strcmp(test_keyvalues_get_value(entity, "classname"),
		     "prop_static") == 0);
	CHECK(strcmp(test_keyvalues_get_value(entity, "model"),
		     "models/crate.obj") == 0);
	CHECK(strcmp(test_keyvalues_get_value(entity, "origin"), "1 2 3") == 0);

	entity = keyvalues_node_get_child(root, 2);
	CHECK(entity != NULL);
	CHECK(strcmp(test_keyvalues_get_value(entity, "origin"), "4 5 6") == 0);

	keyvalues_destroy(document);

	return true;
}

static bool test_comments_and_bare_tokens(void) {
	static const char source[] = "// map comment\n"
				     "entity\n"
				     "{\n"
				     "\tclassname prop_static\n"
				     "\t// property comment\n"
				     "\tproperties\n"
				     "\t{\n"
				     "\t\tvalue \"nested value\"\n"
				     "\t}\n"
				     "}\n";
	keyvalues_document_t *document;
	const keyvalues_node_t *root;
	const keyvalues_node_t *entity;
	const keyvalues_node_t *properties;
	char error[256];

	document = keyvalues_parse(source, error, sizeof(error));
	CHECK(document != NULL);

	root = keyvalues_get_root(document);
	CHECK(keyvalues_node_get_child_count(root) == 1);

	entity = keyvalues_node_get_child(root, 0);
	CHECK(entity != NULL);
	CHECK(strcmp(test_keyvalues_get_value(entity, "classname"),
		     "prop_static") == 0);

	properties = keyvalues_node_find_child(entity, "properties");
	CHECK(properties != NULL);
	CHECK(keyvalues_node_is_block(properties));
	CHECK(strcmp(test_keyvalues_get_value(properties, "value"),
		     "nested value") == 0);

	keyvalues_destroy(document);

	return true;
}

static bool test_escaped_string(void) {
	static const char source[] =
		"entity\n"
		"{\n"
		"\t\"value\" \"line 1\\nline 2 \\\"quoted\\\" \\\\ path\"\n"
		"}\n";
	static const char expected[] = "line 1\nline 2 \"quoted\" \\ path";
	keyvalues_document_t *document;
	const keyvalues_node_t *root;
	const keyvalues_node_t *entity;
	char error[256];

	document = keyvalues_parse(source, error, sizeof(error));
	CHECK(document != NULL);

	root = keyvalues_get_root(document);
	entity = keyvalues_node_get_child(root, 0);

	CHECK(entity != NULL);
	CHECK(strcmp(test_keyvalues_get_value(entity, "value"), expected) == 0);

	keyvalues_destroy(document);

	return true;
}

static bool test_unclosed_block(void) {
	static const char source[] = "entity\n"
				     "{\n"
				     "\t\"classname\" \"prop_static\"\n";
	keyvalues_document_t *document;
	char error[256];

	document = keyvalues_parse(source, error, sizeof(error));

	CHECK(document == NULL);
	CHECK(strstr(error, "expected closing brace") != NULL);

	return true;
}

static bool test_unexpected_closing_brace(void) {
	static const char source[] = "}\n";
	keyvalues_document_t *document;
	char error[256];

	document = keyvalues_parse(source, error, sizeof(error));

	CHECK(document == NULL);
	CHECK(strstr(error, "unexpected closing brace") != NULL);

	return true;
}

static bool test_load_file(void) {
	keyvalues_document_t *document;
	const keyvalues_node_t *root;
	const keyvalues_node_t *entity;
	char error[256];

	document = keyvalues_load(TEST_KEYVALUES_FILE, error, sizeof(error));
	CHECK(document != NULL);

	root = keyvalues_get_root(document);
	CHECK(keyvalues_node_get_child_count(root) == 2);

	entity = keyvalues_node_get_child(root, 1);
	CHECK(entity != NULL);
	CHECK(strcmp(keyvalues_node_get_key(entity), "entity") == 0);
	CHECK(strcmp(test_keyvalues_get_value(entity, "classname"),
		     "prop_static") == 0);
	CHECK(strcmp(test_keyvalues_get_value(entity, "origin"), "0 1 0") == 0);

	keyvalues_destroy(document);

	return true;
}

int main(void) {
	static const test_case_t tests[] = {
		{"basic document",	   test_basic_document	      },
		{"comments and bare tokens", test_comments_and_bare_tokens},
		{"escaped string",	   test_escaped_string	      },
		{"unclosed block",	   test_unclosed_block	      },
		{"unexpected closing brace", test_unexpected_closing_brace},
		{"load file", test_load_file},
	};

	return test_run_all(tests, sizeof(tests) / sizeof(tests[0]));
}