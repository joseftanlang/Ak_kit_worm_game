#ifndef WORM_FB_H
#define WORM_FB_H

#include "config.h"
#include <stdint.h>

void fb_clear(void);
void fb_draw_pixel(int16_t x, int16_t y, uint8_t color);
const uint8_t *fb_get_buffer(void);

#endif
