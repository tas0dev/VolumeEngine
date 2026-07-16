/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#ifndef VOLUME_ENTITY_ENTITY_H
#define VOLUME_ENTITY_ENTITY_H

#include "core/types.h"
#include "entity/properties.h"
#include "renderer/view.h"
#include "scene/transform.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint32_t entity_id_t;

typedef struct entity entity_t;
typedef struct entity_class entity_class_t;
struct asset_manager;

typedef struct entity_spawn_context {
	const entity_properties_t *properties;
	const entity_property_source_t *source;
	struct asset_manager *assets;
	char *error;
	size_t error_size;
} entity_spawn_context_t;

struct entity_class {
	const char *classname;
	entity_t *(*create)(entity_id_t id,
			    const entity_spawn_context_t *context);
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
	char *targetname;
	transform_t transform;
	bool active;
};

void entity_initialize(entity_t *entity,
		       entity_id_t id,
		       const entity_class_t *class);
entity_t *entity_create(const char *classname,
			entity_id_t id,
			const entity_spawn_context_t *context);
const char *entity_get_classname(const entity_t *entity);
void entity_update(entity_t *entity, float delta_time);
void entity_draw_shadow(entity_t *entity, renderer_t *renderer);
void entity_draw(entity_t *entity,
		 renderer_t *renderer,
		 const render_view_t *view);
void entity_destroy(entity_t *entity);
void entity_set_active(entity_t *entity, bool active);
bool entity_is_active(const entity_t *entity);
bool entity_register_class(const entity_class_t *class);
void entity_registry_shutdown(void);
bool entity_set_targetname(entity_t *entity, const char *targetname);
const char *entity_get_targetname(const entity_t *entity);

#endif