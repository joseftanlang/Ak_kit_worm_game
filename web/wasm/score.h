#ifndef WORM_SCORE_H
#define WORM_SCORE_H

#include "worm_types.h"
#include <stdint.h>

extern worm_score_t game_score;

void score_init(void);
void score_reset(void);
void score_inc(void);
uint32_t score_get(void);
void score_commit_current(void);
uint32_t score_top_get(uint8_t index);
void score_import_top(uint32_t top1, uint32_t top2, uint32_t top3);

#endif
