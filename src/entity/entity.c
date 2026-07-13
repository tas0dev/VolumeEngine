/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#include "entity/entity.h"
#include "prop_static.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef struct entity_registry {
	const entity_class_t **classes;
	size_t count;
	size_t capacity;
} entity_registry_t;

static entity_registry_t registry;

static bool registry_reserve(size_t capacity);
static const entity_class_t *registry_find(const char *classname);

void entity_initialize(entity_t *entity,
		       const entity_id_t id,
		       const entity_class_t *class) {
	if (entity == NULL) { return; }

	entity->id = id;
	entity->class = class;
	entity->transform = transform_create();
	entity->active = true;
}

entity_t *entity_create(const char *classname,
			const entity_id_t id,
			const entity_properties_t *properties) {
	const entity_class_t *class;

	if (classname == NULL || id == 0 || properties == NULL) { return NULL; }

	class = registry_find(classname);
	if (class == NULL) { return NULL; }

	return class->create(id, properties);
}

const char *entity_get_classname(const entity_t *entity) {
	if (entity == NULL || entity->class == NULL) { return NULL; }

	return entity->class->classname;
}

void entity_update(entity_t *entity, const float delta_time) {
	if (entity == NULL || !entity->active || entity->class == NULL ||
	    entity->class->update == NULL) {
		return;
	}

	entity->class->update(entity, delta_time);
}

void entity_draw_shadow(entity_t *entity, renderer_t *renderer) {
	if (entity == NULL || renderer == NULL || !entity->active ||
	    entity->class == NULL || entity->class->draw_shadow == NULL) {
		return;
	}

	entity->class->draw_shadow(entity, renderer);
}

void entity_draw(entity_t *entity,
		 renderer_t *renderer,
		 const render_view_t *view) {
	if (entity == NULL || renderer == NULL || view == NULL ||
	    !entity->active || entity->class == NULL ||
	    entity->class->draw == NULL) {
		return;
	}

	entity->class->draw(entity, renderer, view);
}

void entity_destroy(entity_t *entity) {
	if (entity == NULL || entity->class == NULL ||
	    entity->class->destroy == NULL) {
		return;
	}

	entity->class->destroy(entity);
}

void entity_set_active(entity_t *entity, const bool active) {
	if (entity == NULL) { return; }

	entity->active = active;
}

bool entity_is_active(const entity_t *entity) {
	if (entity == NULL) { return false; }

	return entity->active;
}

bool entity_register_class(const entity_class_t *class) {
	const entity_class_t *registered;
	size_t capacity;

	if (class == NULL || class->classname == NULL ||
	    class->create == NULL) {
		return false;
	}

	registered = registry_find(class->classname);
	if (registered != NULL) { return registered == class; }

	if (registry.count == registry.capacity) {
		capacity = registry.capacity == 0 ? 16 : registry.capacity * 2;

		if (!registry_reserve(capacity)) { return false; }
	}

	registry.classes[registry.count] = class;
	registry.count++;

	return true;
}

static bool registry_reserve(const size_t capacity) {
	const entity_class_t **classes;

	if (capacity <= registry.capacity) { return true; }

	classes =
		realloc(registry.classes, capacity * sizeof(*registry.classes));
	if (classes == NULL) { return false; }

	registry.classes = classes;
	registry.capacity = capacity;

	return true;
}

static const entity_class_t *registry_find(const char *classname) {
	size_t index;

	if (classname == NULL) { return NULL; }

	for (index = 0; index < registry.count; index++) {
		if (strcmp(registry.classes[index]->classname, classname) ==
		    0) {
			return registry.classes[index];
		}
	}

	return NULL;
}