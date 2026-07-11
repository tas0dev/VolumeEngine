/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 *
 */

#ifndef VOLUME_SHADER_H
#define VOLUME_SHADER_H

typedef struct shader shader_t;

shader_t *shader_create(const char *vertex_path, const char *fragment_path);
void shader_destroy(shader_t *shader);
void shader_bind(const shader_t *shader);
void shader_unbind(void);

#endif // VOLUME_SHADER_H
