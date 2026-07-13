/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#include "entity/builtin.h"
#include "entity/prop_static.h"

bool entity_register_builtin_classes(void) { return prop_static_register(); }