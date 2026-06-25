#ifndef WORM_GAME_H
#define WORM_GAME_H

#include "worm_types.h"

extern worm_state_t worm_game;
extern worm_border_t worm_border;

void worm_init(void);
void worm_set_direction(worm_dir_t d);
worm_dir_t worm_get_direction(void);
uint8_t worm_advance(void);
void worm_grow(void);
worm_dir_t worm_turn_left(worm_dir_t dir);
worm_dir_t worm_turn_right(worm_dir_t dir);

void border_init(void);

#endif
