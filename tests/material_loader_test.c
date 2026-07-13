/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#include "asset/material_loader.h"
#include "common.h"
#include <string.h>

static bool test_valid_material(void) {
	static const char source[] = "material\n"
				     "{\n"
				     "\t\"color\" \"0.5 0.75 1\"\n"
				     "\t\"ambient_strength\" \"0.125\"\n"
				     "\t\"specular_strength\" \"0.25\"\n"
				     "\t\"shininess\" \"16\"\n"
				     "}\n";
	material_t material;
	char error[256];

	CHECK(material_parse(source, &material, error, sizeof(error)));

	CHECK(material.color.x == 0.5f);
	CHECK(material.color.y == 0.75f);
	CHECK(material.color.z == 1.0f);
	CHECK(material.ambient_strength == 0.125f);
	CHECK(material.specular_strength == 0.25f);
	CHECK(material.shininess == 16.0f);

	return true;
}

static bool test_default_values(void) {
	static const char source[] = "material\n"
				     "{\n"
				     "\t\"color\" \"1 1 1\"\n"
				     "}\n";
	material_t material;
	char error[256];

	CHECK(material_parse(source, &material, error, sizeof(error)));

	CHECK(material.ambient_strength > 0.199f);
	CHECK(material.ambient_strength < 0.201f);
	CHECK(material.specular_strength == 0.5f);
	CHECK(material.shininess == 32.0f);

	return true;
}

static bool test_invalid_color(void) {
	static const char source[] = "material\n"
				     "{\n"
				     "\t\"color\" \"1 0\"\n"
				     "}\n";
	material_t material;
	char error[256];

	CHECK(!material_parse(source, &material, error, sizeof(error)));
	CHECK(strstr(error, "invalid material color") != NULL);

	return true;
}

static bool test_unknown_property(void) {
	static const char source[] = "material\n"
				     "{\n"
				     "\t\"metalness\" \"1\"\n"
				     "}\n";
	material_t material;
	char error[256];

	CHECK(!material_parse(source, &material, error, sizeof(error)));
	CHECK(strstr(error, "unknown material property") != NULL);

	return true;
}

static bool test_load_file(void) {
	material_t material;
	char error[256];

	CHECK(material_load(TEST_MATERIAL_FILE, &material, error,
			    sizeof(error)));

	CHECK(material.color.x == 0.5f);
	CHECK(material.color.y == 0.75f);
	CHECK(material.color.z == 1.0f);
	CHECK(material.shininess == 16.0f);

	return true;
}

int main(void) {
	static const test_case_t tests[] = {
		{"valid material",   test_valid_material  },
		{"default values",   test_default_values  },
		{"invalid color",	  test_invalid_color   },
		{"unknown property", test_unknown_property},
		{"load file",	      test_load_file	    },
	};

	return test_run_all(tests, sizeof(tests) / sizeof(tests[0]));
}