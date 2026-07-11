/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#ifndef VOLUME_SHADER_H
#define VOLUME_SHADER_H

#include "math/mat4.h"
#include "math/vec3.h"

typedef struct shader shader_t;

shader_t *shader_create(const char *vertex_path, const char *fragment_path);
void shader_destroy(shader_t *shader);
void shader_bind(const shader_t *shader);
void shader_unbind(void);
void shader_set_mat4(const shader_t *shader,
		     const char *name,
		     const mat4_t *matrix);
void shader_set_vec3(const shader_t *shader, const char *name, vec3_t value);
void shader_set_float(const shader_t *shader, const char *name, float value);

#endif // VOLUME_SHADER_H
