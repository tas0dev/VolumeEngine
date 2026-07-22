/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "entity/builtin.h"
#include "entity/func_door.h"
#include "entity/light_environment.h"
#include "entity/logic_relay.h"
#include "entity/player.h"
#include "entity/prop_dynamic.h"
#include "entity/prop_static.h"

bool entity_register_builtin_classes(void) {
	if (!func_door_register()) { return false; }
	if (!prop_static_register()) { return false; }
	if (!prop_dynamic_register()) { return false; }
	if (!light_environment_register()) { return false; }
	if (!logic_relay_register()) { return false; }
	if (!player_register()) { return false; }

	return true;
}
