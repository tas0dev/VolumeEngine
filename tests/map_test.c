/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#include "common.h"
#include "map/map.h"
#include <string.h>

static bool test_valid_map(void) {
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
	map_t *map;
	const map_entity_t *world;
	const map_entity_t *entity;
	char error[256];

	map = map_parse(source, error, sizeof(error));
	CHECK(map != NULL);

	world = map_get_world(map);
	CHECK(world != NULL);
	CHECK(strcmp(map_entity_get_property(world, "classname"),
		     "worldspawn") == 0);

	CHECK(map_get_entity_count(map) == 2);

	entity = map_get_entity(map, 0);
	CHECK(entity != NULL);
	CHECK(strcmp(map_entity_get_property(entity, "classname"),
		     "prop_static") == 0);
	CHECK(strcmp(map_entity_get_property(entity, "model"),
		     "models/crate.obj") == 0);
	CHECK(strcmp(map_entity_get_property(entity, "origin"), "1 2 3") == 0);

	entity = map_get_entity(map, 1);
	CHECK(entity != NULL);
	CHECK(strcmp(map_entity_get_property(entity, "origin"), "4 5 6") == 0);

	map_destroy(map);

	return true;
}

static bool test_missing_world(void) {
	static const char source[] = "entity\n"
				     "{\n"
				     "\t\"classname\" \"prop_static\"\n"
				     "}\n";
	map_t *map;
	char error[256];

	map = map_parse(source, error, sizeof(error));

	CHECK(map == NULL);
	CHECK(strstr(error, "world block is required") != NULL);

	return true;
}

static bool test_duplicate_world(void) {
	static const char source[] = "world\n"
				     "{\n"
				     "\t\"classname\" \"worldspawn\"\n"
				     "}\n"
				     "world\n"
				     "{\n"
				     "\t\"classname\" \"worldspawn\"\n"
				     "}\n";
	map_t *map;
	char error[256];

	map = map_parse(source, error, sizeof(error));

	CHECK(map == NULL);
	CHECK(strstr(error, "multiple world blocks") != NULL);

	return true;
}

static bool test_missing_entity_classname(void) {
	static const char source[] = "world\n"
				     "{\n"
				     "\t\"classname\" \"worldspawn\"\n"
				     "}\n"
				     "entity\n"
				     "{\n"
				     "\t\"origin\" \"0 0 0\"\n"
				     "}\n";
	map_t *map;
	char error[256];

	map = map_parse(source, error, sizeof(error));

	CHECK(map == NULL);
	CHECK(strstr(error, "entity block has no classname") != NULL);

	return true;
}

static bool test_nested_entity_block(void) {
	static const char source[] = "world\n"
				     "{\n"
				     "\t\"classname\" \"worldspawn\"\n"
				     "}\n"
				     "entity\n"
				     "{\n"
				     "\t\"classname\" \"prop_static\"\n"
				     "\tproperties\n"
				     "\t{\n"
				     "\t\t\"value\" \"test\"\n"
				     "\t}\n"
				     "}\n";
	map_t *map;
	char error[256];

	map = map_parse(source, error, sizeof(error));

	CHECK(map == NULL);
	CHECK(strstr(error, "nested block") != NULL);

	return true;
}

static bool test_typed_properties(void) {
	static const char source[] = "world\n"
				     "{\n"
				     "\t\"classname\" \"worldspawn\"\n"
				     "}\n"
				     "entity\n"
				     "{\n"
				     "\t\"classname\" \"prop_static\"\n"
				     "\t\"origin\" \"1.5 -2 3.25\"\n"
				     "\t\"angles\" \"0 90 0\"\n"
				     "\t\"scale\" \"2 2 2\"\n"
				     "\t\"casts_shadow\" \"0\"\n"
				     "}\n";
	const map_entity_t *entity;
	map_t *map;
	vec3_t value;
	bool casts_shadow;
	char error[256];

	map = map_parse(source, error, sizeof(error));
	CHECK(map != NULL);

	entity = map_get_entity(map, 0);
	CHECK(entity != NULL);

	CHECK(map_entity_get_vec3(entity, "origin", &value));
	CHECK(value.x == 1.5f);
	CHECK(value.y == -2.0f);
	CHECK(value.z == 3.25f);

	CHECK(map_entity_get_vec3(entity, "angles", &value));
	CHECK(value.x == 0.0f);
	CHECK(value.y == 90.0f);
	CHECK(value.z == 0.0f);

	CHECK(map_entity_get_vec3(entity, "scale", &value));
	CHECK(value.x == 2.0f);
	CHECK(value.y == 2.0f);
	CHECK(value.z == 2.0f);

	CHECK(map_entity_get_bool(entity, "casts_shadow", &casts_shadow));
	CHECK(!casts_shadow);

	map_destroy(map);

	return true;
}

static bool test_invalid_typed_properties(void) {
	static const char source[] = "world\n"
				     "{\n"
				     "\t\"classname\" \"worldspawn\"\n"
				     "}\n"
				     "entity\n"
				     "{\n"
				     "\t\"classname\" \"prop_static\"\n"
				     "\t\"origin\" \"1 2\"\n"
				     "\t\"angles\" \"0 90 0 extra\"\n"
				     "\t\"casts_shadow\" \"maybe\"\n"
				     "}\n";
	const map_entity_t *entity;
	map_t *map;
	vec3_t value;
	bool boolean;
	char error[256];

	map = map_parse(source, error, sizeof(error));
	CHECK(map != NULL);

	entity = map_get_entity(map, 0);
	CHECK(entity != NULL);

	CHECK(!map_entity_get_vec3(entity, "origin", &value));
	CHECK(!map_entity_get_vec3(entity, "angles", &value));
	CHECK(!map_entity_get_bool(entity, "casts_shadow", &boolean));

	map_destroy(map);

	return true;
}

int main(void) {
	static const test_case_t tests[] = {
		{"valid map",		      test_valid_map		    },
		{"missing world", test_missing_world},
		{"duplicate world",	    test_duplicate_world		},
		{"missing entity classname", test_missing_entity_classname},
		{"nested entity block",	test_nested_entity_block	},
		{"typed properties",	     test_typed_properties	  },
		{"invalid typed properties", test_invalid_typed_properties},
	};

	return test_run_all(tests, sizeof(tests) / sizeof(tests[0]));
}