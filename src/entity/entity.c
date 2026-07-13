/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#include "entity/entity.h"

#include "prop_static.h"

#include <string.h>

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
	prop_static_t *prop;

	if (classname == NULL || properties == NULL) { return NULL; }

	if (strcmp(classname, "prop_static") == 0) {
		prop = prop_static_create(id, properties->mesh,
					  properties->material);
		if (prop == NULL) { return NULL; }

		prop->entity.transform = properties->transform;
		prop->casts_shadow = properties->casts_shadow;

		return prop_static_get_entity(prop);
	}

	return NULL;
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