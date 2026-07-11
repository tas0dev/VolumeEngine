#ifndef VOLUME_PLATFORM_H
#define VOLUME_PLATFORM_H

#include <stdbool.h>

typedef struct platform_config {
	const char *title;
	int width;
	int height;
} platform_config_t;

typedef struct platform platform_t;

platform_t *platform_create(const platform_config_t *config);
void platform_destroy(platform_t *platform);
bool platform_poll_events(platform_t *platform);
double platform_get_time(void);
void platform_sleep(double seconds);

#endif