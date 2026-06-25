#ifndef WORM_GFX_H
#define WORM_GFX_H

#include "config.h"
#include <stdint.h>

void gfx_draw_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color);
void gfx_draw_fast_hline(int16_t x, int16_t y, int16_t w, uint8_t color);
void gfx_fill_circle(int16_t x0, int16_t y0, int16_t r, uint8_t color);
void gfx_draw_pixel(int16_t x, int16_t y, uint8_t color);
void gfx_fill_round_rect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint8_t color);
void gfx_draw_bitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint8_t color);
void gfx_set_cursor(int16_t x, int16_t y);
void gfx_set_text_size(uint8_t size);
void gfx_set_text_color(uint8_t color);
void gfx_print(const char *str);
void gfx_print_int(int32_t value);

#endif
