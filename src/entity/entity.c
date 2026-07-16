/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
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
	entity->targetname = NULL;
	entity->transform = transform_create();
	entity->active = true;
}

entity_t *entity_create(const char *classname,
			const entity_id_t id,
			const entity_spawn_context_t *context) {
	const entity_class_t *class;

	if (classname == NULL || id == 0 || context == NULL ||
	    context->properties == NULL) {
		return NULL;
	}

	class = registry_find(classname);
	if (class == NULL) { return NULL; }

	return class->create(id, context);
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
	const entity_class_t *class;

	if (entity == NULL) { return; }

	class = entity->class;

	free(entity->targetname);
	entity->targetname = NULL;

	if (class == NULL || class->destroy == NULL) { return; }

	class->destroy(entity);
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

void entity_registry_shutdown(void) {
	free(registry.classes);

	registry.classes = NULL;
	registry.count = 0;
	registry.capacity = 0;
}

bool entity_set_targetname(entity_t *entity, const char *targetname) {
	char *copy;
	size_t length;

	if (entity == NULL) { return false; }

	copy = NULL;

	if (targetname != NULL && targetname[0] != '\0') {
		length = strlen(targetname);

		copy = malloc(length + 1);
		if (copy == NULL) { return false; }

		memcpy(copy, targetname, length + 1);
	}

	free(entity->targetname);
	entity->targetname = copy;

	return true;
}

const char *entity_get_targetname(const entity_t *entity) {
	if (entity == NULL) { return NULL; }

	return entity->targetname;
}