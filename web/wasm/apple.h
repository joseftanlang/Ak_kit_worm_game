#ifndef WORM_APPLE_H
#define WORM_APPLE_H

#include "worm_types.h"

extern worm_apple_t apples[MAX_APPLES];

void apple_init(uint8_t apple_limit);
void apple_timer_tick(uint8_t apple_limit);
void apple_collision_check(uint8_t apple_limit, uint8_t *ate_apple);

#endif
