#ifndef WORM_RENDER_H
#define WORM_RENDER_H

#include <stdint.h>

void render_worm_screen(uint8_t finished, uint8_t won);
void render_menu(int menu_index);
void render_settings(int selected_item);
void render_charts_tick(void);
void render_charts_reset(void);
void render_charts_screen(void);

#endif
