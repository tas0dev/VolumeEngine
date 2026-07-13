/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#include "asset/manager.h"
#include "common.h"

static unsigned char mesh_marker_a;
static unsigned char mesh_marker_b;

static bool test_create(void) {
	asset_manager_t *manager;

	manager = asset_manager_create();
	CHECK(manager != NULL);

	asset_manager_destroy(manager);

	return true;
}

static bool test_mesh_registry(void) {
	asset_manager_t *manager;
	mesh_t *mesh;

	manager = asset_manager_create();
	CHECK(manager != NULL);

	mesh = (mesh_t *)(void *)&mesh_marker_a;

	CHECK(asset_manager_register_mesh(manager, "models/test_cube", mesh));
	CHECK(asset_manager_get_mesh(manager, "models/test_cube") == mesh);
	CHECK(asset_manager_get_mesh(manager, "models/missing") == NULL);

	asset_manager_destroy(manager);

	return true;
}

static bool test_material_registry(void) {
	asset_manager_t *manager;
	material_t material;

	manager = asset_manager_create();
	CHECK(manager != NULL);

	material = (material_t){0};

	CHECK(asset_manager_register_material(manager, "materials/default",
					      &material));
	CHECK(asset_manager_get_material(manager, "materials/default") ==
	      &material);
	CHECK(asset_manager_get_material(manager, "materials/missing") == NULL);

	asset_manager_destroy(manager);

	return true;
}

static bool test_duplicate_path(void) {
	asset_manager_t *manager;
	mesh_t *mesh_a;
	mesh_t *mesh_b;

	manager = asset_manager_create();
	CHECK(manager != NULL);

	mesh_a = (mesh_t *)(void *)&mesh_marker_a;
	mesh_b = (mesh_t *)(void *)&mesh_marker_b;

	CHECK(asset_manager_register_mesh(manager, "models/test", mesh_a));
	CHECK(!asset_manager_register_mesh(manager, "models/test", mesh_b));
	CHECK(asset_manager_get_mesh(manager, "models/test") == mesh_a);

	asset_manager_destroy(manager);

	return true;
}

static bool test_separate_namespaces(void) {
	asset_manager_t *manager;
	material_t material;
	mesh_t *mesh;

	manager = asset_manager_create();
	CHECK(manager != NULL);

	material = (material_t){0};
	mesh = (mesh_t *)(void *)&mesh_marker_a;

	CHECK(asset_manager_register_mesh(manager, "shared", mesh));
	CHECK(asset_manager_register_material(manager, "shared", &material));
	CHECK(asset_manager_get_mesh(manager, "shared") == mesh);
	CHECK(asset_manager_get_material(manager, "shared") == &material);

	asset_manager_destroy(manager);

	return true;
}

static bool test_invalid_arguments(void) {
	asset_manager_t *manager;
	material_t material;
	mesh_t *mesh;

	manager = asset_manager_create();
	CHECK(manager != NULL);

	material = (material_t){0};
	mesh = (mesh_t *)(void *)&mesh_marker_a;

	CHECK(!asset_manager_register_mesh(NULL, "mesh", mesh));
	CHECK(!asset_manager_register_mesh(manager, NULL, mesh));
	CHECK(!asset_manager_register_mesh(manager, "", mesh));
	CHECK(!asset_manager_register_mesh(manager, "mesh", NULL));

	CHECK(!asset_manager_register_material(NULL, "material", &material));
	CHECK(!asset_manager_register_material(manager, NULL, &material));
	CHECK(!asset_manager_register_material(manager, "", &material));
	CHECK(!asset_manager_register_material(manager, "material", NULL));

	CHECK(asset_manager_get_mesh(NULL, "mesh") == NULL);
	CHECK(asset_manager_get_mesh(manager, NULL) == NULL);
	CHECK(asset_manager_get_material(NULL, "material") == NULL);
	CHECK(asset_manager_get_material(manager, NULL) == NULL);

	asset_manager_destroy(manager);

	return true;
}

int main(void) {
	static const test_case_t tests[] = {
		{"create",		   test_create	      },
		{"mesh registry",	  test_mesh_registry	    },
		{"material registry",   test_material_registry  },
		{"duplicate path",	   test_duplicate_path     },
		{"separate namespaces", test_separate_namespaces},
		{"invalid arguments",   test_invalid_arguments  },
	};

	return test_run_all(tests, sizeof(tests) / sizeof(tests[0]));
}