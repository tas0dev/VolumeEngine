/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "npc_enemy.h"
#include "entity/damage.h"
#include "entity/health.h"
#include "entity/player.h"
#include "entity/prop_internal.h"
#include "entity/world.h"
#include "physics/character_controller.h"
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct sandbox_npc_enemy {
	prop_t prop;
	health_t health;
	character_controller_t controller;
	entity_t *target;
	vec3_t last_known_position;
	sandbox_npc_enemy_state_t state;
	float detection_range;
	float attack_range;
	float move_speed;
	float attack_damage;
	float attack_interval;
	float attack_cooldown;
	float target_memory;
	float memory_remaining;
	float height;
	bool saw_target;
};

static entity_t *create_entity(entity_id_t id,
			       const entity_spawn_context_t *context);
static void update(entity_t *entity, float delta_time);
static bool take_damage(entity_t *entity, const damage_info_t *damage);
static void
set_error(const entity_spawn_context_t *context, const char *format, ...);

static const entity_class_t npc_enemy_class = {
	.classname = "npc_enemy",
	.create = create_entity,
	.update = update,
	.draw_shadow = prop_internal_draw_shadow,
	.draw = prop_internal_draw,
	.take_damage = take_damage,
	.destroy = prop_internal_destroy,
};

static bool load_positive_float(const entity_spawn_context_t *context,
				const char *name,
				float *value) {
	const char *text;

	text = entity_property_get(context->source, name);
	if (text == NULL) { return true; }
	if (!entity_property_parse_float(text, value) || *value <= 0.0f) {
		set_error(context, "invalid positive npc_enemy %s: \"%s\"",
			  name, text);
		return false;
	}
	return true;
}

static entity_t *create_entity(const entity_id_t id,
			       const entity_spawn_context_t *context) {
	sandbox_npc_enemy_t *enemy;
	vec3_t spawn_position;
	vec3_t collider_half_extents;
	float maximum_health;
	float radius;

	if (context == NULL || context->properties == NULL ||
	    context->source == NULL) {
		return NULL;
	}
	enemy = calloc(1, sizeof(*enemy));
	if (enemy == NULL) { return NULL; }
	if (!prop_internal_initialize(&enemy->prop, id, &npc_enemy_class,
				      context)) {
		free(enemy);
		return NULL;
	}

	maximum_health = 60.0f;
	enemy->detection_range = 12.0f;
	enemy->attack_range = 1.4f;
	enemy->move_speed = 2.4f;
	enemy->attack_damage = 10.0f;
	enemy->attack_interval = 0.8f;
	enemy->target_memory = 2.0f;
	enemy->height = 1.7f;
	radius = 0.35f;
	if (!load_positive_float(context, "health", &maximum_health) ||
	    !load_positive_float(context, "detection_range",
				 &enemy->detection_range) ||
	    !load_positive_float(context, "attack_range",
				 &enemy->attack_range) ||
	    !load_positive_float(context, "move_speed", &enemy->move_speed) ||
	    !load_positive_float(context, "attack_damage",
				 &enemy->attack_damage) ||
	    !load_positive_float(context, "attack_interval",
				 &enemy->attack_interval) ||
	    !load_positive_float(context, "target_memory",
				 &enemy->target_memory) ||
	    !load_positive_float(context, "height", &enemy->height) ||
	    !load_positive_float(context, "radius", &radius) ||
	    !health_initialize(&enemy->health, maximum_health)) {
		entity_destroy(&enemy->prop.entity);
		return NULL;
	}
	if (fabsf(enemy->prop.entity.transform.scale.x) <= 0.0001f ||
	    fabsf(enemy->prop.entity.transform.scale.y) <= 0.0001f ||
	    fabsf(enemy->prop.entity.transform.scale.z) <= 0.0001f) {
		set_error(context,
			  "npc_enemy scale components must be non-zero");
		entity_destroy(&enemy->prop.entity);
		return NULL;
	}

	spawn_position = context->properties->transform.position;
	enemy->controller = character_controller_create(spawn_position, radius,
							enemy->height);
	enemy->controller.maximum_speed = enemy->move_speed;
	enemy->prop.entity.transform.position.y += enemy->height * 0.5f;
	collider_half_extents = vec3_create(
		radius / fabsf(enemy->prop.entity.transform.scale.x),
		(enemy->height * 0.5f) /
			fabsf(enemy->prop.entity.transform.scale.y),
		radius / fabsf(enemy->prop.entity.transform.scale.z));
	entity_set_collider(&enemy->prop.entity,
			    collider_create_box(vec3_create(0.0f, 0.0f, 0.0f),
						collider_half_extents));
	entity_set_collision_filter(
		&enemy->prop.entity, COLLISION_LAYER_DYNAMIC,
		COLLISION_LAYER_WORLD_STATIC | COLLISION_LAYER_DYNAMIC |
			COLLISION_LAYER_PLAYER);
	enemy->prop.entity.collider_follows_transform = true;
	enemy->state = SANDBOX_NPC_ENEMY_IDLE;
	return &enemy->prop.entity;
}

static player_t *find_player(sandbox_npc_enemy_t *enemy) {
	entity_t *entity;
	player_t *player;

	if (enemy->target != NULL && !enemy->target->pending_destroy) {
		player = player_from_entity(enemy->target);
		if (player != NULL && player_is_alive(player)) {
			return player;
		}
	}
	enemy->target = NULL;
	if (enemy->prop.entity.world == NULL) { return NULL; }
	entity = world_find_by_classname(enemy->prop.entity.world, "player");
	player = player_from_entity(entity);
	if (player == NULL || !player_is_alive(player)) { return NULL; }
	enemy->target = entity;
	return player;
}

static bool can_see_player(const sandbox_npc_enemy_t *enemy,
			   const player_t *player,
			   float *distance) {
	collision_filter_t filter;
	collision_trace_t trace;
	vec3_t start;
	vec3_t end;
	vec3_t offset;

	start = enemy->controller.position;
	start.y += enemy->height * 0.85f;
	end = player_get_view_position(player);
	offset = vec3_subtract(end, start);
	*distance = vec3_length(offset);
	if (*distance > enemy->detection_range) { return false; }
	filter.layer = COLLISION_LAYER_DYNAMIC;
	filter.mask = COLLISION_LAYER_WORLD_STATIC | COLLISION_LAYER_DYNAMIC |
		      COLLISION_LAYER_PLAYER;
	filter.ignored_entity_id = enemy->prop.entity.id;
	if (!collision_world_trace_ray_filtered(
		    world_get_const_collision_world(enemy->prop.entity.world),
		    start, end, filter, &trace)) {
		return true;
	}
	return trace.entity_id == player_get_const_entity(player)->id;
}

static void set_state(sandbox_npc_enemy_t *enemy,
		      const sandbox_npc_enemy_state_t state,
		      entity_t *activator) {
	if (enemy->state == state) { return; }
	if ((state == SANDBOX_NPC_ENEMY_CHASE ||
	     state == SANDBOX_NPC_ENEMY_ATTACK) &&
	    !enemy->saw_target) {
		(void)world_fire_output(enemy->prop.entity.world,
					&enemy->prop.entity, "OnSeePlayer",
					activator);
		enemy->saw_target = true;
	} else if (state == SANDBOX_NPC_ENEMY_IDLE && enemy->saw_target) {
		(void)world_fire_output(enemy->prop.entity.world,
					&enemy->prop.entity, "OnLostPlayer",
					activator);
		enemy->saw_target = false;
	}
	enemy->state = state;
}

static void move_towards_target(sandbox_npc_enemy_t *enemy,
				const vec3_t destination,
				const float delta_time) {
	character_move_input_t input = {0};
	collision_filter_t filter;
	vec3_t direction;

	direction = vec3_subtract(destination, enemy->controller.position);
	direction.y = 0.0f;
	input.wish_direction = vec3_normalize(direction);
	input.look_direction = input.wish_direction;
	input.wish_speed = enemy->move_speed;
	filter.layer = COLLISION_LAYER_DYNAMIC;
	filter.mask = COLLISION_LAYER_WORLD_STATIC | COLLISION_LAYER_DYNAMIC |
		      COLLISION_LAYER_PLAYER;
	filter.ignored_entity_id = enemy->prop.entity.id;
	character_controller_move_filtered(
		&enemy->controller,
		world_get_const_collision_world(enemy->prop.entity.world),
		filter, &input, delta_time);
	enemy->prop.entity.transform.position = enemy->controller.position;
	enemy->prop.entity.transform.position.y += enemy->height * 0.5f;
}

static void attack_player(sandbox_npc_enemy_t *enemy, player_t *player) {
	damage_info_t damage = {0};
	vec3_t direction;

	if (enemy->attack_cooldown > 0.0f) { return; }
	direction = vec3_subtract(player_get_position(player),
				  enemy->controller.position);
	damage.amount = enemy->attack_damage;
	damage.type = DAMAGE_TYPE_CLUB;
	damage.attacker = &enemy->prop.entity;
	damage.inflictor = &enemy->prop.entity;
	damage.position = player_get_view_position(player);
	damage.direction = vec3_normalize(direction);
	if (entity_take_damage(player_get_entity(player), &damage)) {
		(void)world_fire_output(enemy->prop.entity.world,
					&enemy->prop.entity, "OnAttack",
					player_get_entity(player));
	}
	enemy->attack_cooldown = enemy->attack_interval;
}

static void update(entity_t *entity, const float delta_time) {
	sandbox_npc_enemy_t *enemy;
	player_t *player;
	float distance;
	bool visible;

	enemy = sandbox_npc_enemy_from_entity(entity);
	if (enemy == NULL || delta_time <= 0.0f) { return; }
	enemy->attack_cooldown =
		fmaxf(0.0f, enemy->attack_cooldown - delta_time);
	player = find_player(enemy);
	if (player == NULL) {
		set_state(enemy, SANDBOX_NPC_ENEMY_IDLE, NULL);
		return;
	}
	visible = can_see_player(enemy, player, &distance);
	if (visible) {
		enemy->last_known_position = player_get_position(player);
		enemy->memory_remaining = enemy->target_memory;
	} else {
		enemy->memory_remaining =
			fmaxf(0.0f, enemy->memory_remaining - delta_time);
	}

	if (visible && distance <= enemy->attack_range) {
		set_state(enemy, SANDBOX_NPC_ENEMY_ATTACK,
			  player_get_entity(player));
		attack_player(enemy, player);
		return;
	}
	if (visible || enemy->memory_remaining > 0.0f) {
		set_state(enemy, SANDBOX_NPC_ENEMY_CHASE,
			  player_get_entity(player));
		move_towards_target(enemy, enemy->last_known_position,
				    delta_time);
		return;
	}
	set_state(enemy, SANDBOX_NPC_ENEMY_IDLE, player_get_entity(player));
}

static bool take_damage(entity_t *entity, const damage_info_t *damage) {
	sandbox_npc_enemy_t *enemy;
	float applied;

	enemy = sandbox_npc_enemy_from_entity(entity);
	if (enemy == NULL || damage == NULL) { return false; }
	applied = health_apply_damage(&enemy->health, damage->amount);
	if (applied <= 0.0f) { return false; }
	if (entity->world != NULL) {
		(void)world_fire_output(entity->world, entity, "OnDamaged",
					damage->attacker);
	}
	if (damage->attacker != NULL &&
	    player_from_entity(damage->attacker) != NULL) {
		enemy->target = damage->attacker;
		enemy->last_known_position = player_get_position(
			player_from_entity(damage->attacker));
		enemy->memory_remaining = enemy->target_memory;
	}
	if (!health_is_alive(&enemy->health)) {
		if (entity->world != NULL) {
			(void)world_fire_output(entity->world, entity,
						"OnDeath", damage->attacker);
		}
		entity->pending_destroy = true;
	}
	return true;
}

bool sandbox_npc_enemy_register(void) {
	return entity_register_class(&npc_enemy_class);
}

sandbox_npc_enemy_t *sandbox_npc_enemy_from_entity(entity_t *entity) {
	if (entity == NULL || entity->class != &npc_enemy_class) {
		return NULL;
	}
	return (sandbox_npc_enemy_t *)entity;
}

sandbox_npc_enemy_state_t
sandbox_npc_enemy_get_state(const sandbox_npc_enemy_t *enemy) {
	return enemy == NULL ? SANDBOX_NPC_ENEMY_IDLE : enemy->state;
}

float sandbox_npc_enemy_get_health(const sandbox_npc_enemy_t *enemy) {
	return enemy == NULL ? 0.0f : health_get_current(&enemy->health);
}

static void
set_error(const entity_spawn_context_t *context, const char *format, ...) {
	va_list arguments;

	if (context == NULL || context->error == NULL ||
	    context->error_size == 0) {
		return;
	}
	va_start(arguments, format);
	vsnprintf(context->error, context->error_size, format, arguments);
	va_end(arguments);
}
