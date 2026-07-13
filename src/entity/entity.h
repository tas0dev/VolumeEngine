/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#ifndef VOLUME_ENTITY_ENTITY_H
#define VOLUME_ENTITY_ENTITY_H

#include "core/types.h"
#include "renderer/view.h"
#include "scene/transform.h"
#include <stdbool.h>
#include <stdint.h>

typedef uint32_t entity_id_t;

typedef struct entity entity_t;
typedef struct entity_class entity_class_t;

struct entity_class {
	const char *classname;
	void (*update)(entity_t *entity, float delta_time);
	void (*draw_shadow)(entity_t *entity, renderer_t *renderer);
	void (*draw)(entity_t *entity,
		     renderer_t *renderer,
		     const render_view_t *view);
	void (*destroy)(entity_t *entity);
};

struct entity {
	entity_id_t id;
	const entity_class_t *class;
	transform_t transform;
	bool active;
};

entity_t entity_create(entity_id_t id, const entity_class_t *class);
const char *entity_get_classname(const entity_t *entity);
void entity_update(entity_t *entity, float delta_time);
void entity_draw_shadow(entity_t *entity, renderer_t *renderer);
void entity_draw(entity_t *entity,
		 renderer_t *renderer,
		 const render_view_t *view);
void entity_destroy(entity_t *entity);
void entity_set_active(entity_t *entity, bool active);
bool entity_is_active(const entity_t *entity);

#endif