#ifndef VOLUME_RENDERER_RENDERER_H
#define VOLUME_RENDERER_RENDERER_H

#include "platform/platform.h"

typedef struct renderer renderer_t;

renderer_t *renderer_create(platform_t *platform);
void renderer_destroy(renderer_t *renderer);
void renderer_begin_frame(const renderer_t *renderer);
void renderer_end_frame(const renderer_t *renderer);

#endif