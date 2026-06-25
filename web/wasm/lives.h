#ifndef WORM_LIVES_H
#define WORM_LIVES_H

#include "worm_types.h"

extern worm_lives_t game_lives;

void lives_init(void);
void lives_reset(void);
void lives_dec(void);
uint32_t lives_get(void);

#endif
