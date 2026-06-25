#ifndef WORM_SETTINGS_H
#define WORM_SETTINGS_H

#include <stdint.h>

void settings_init(void);
void settings_import(uint8_t speed, uint8_t apples, uint8_t song, uint8_t buzzer);
void settings_export(uint8_t *speed, uint8_t *apples, uint8_t *song, uint8_t *buzzer);
uint16_t settings_get_worm_tick_ms(void);
uint8_t settings_get_apple_count(void);
uint8_t settings_get_song_index(void);
uint8_t settings_is_buzzer_enabled(void);
void settings_cycle_speed(void);
void settings_cycle_apples(void);
void settings_cycle_song(void);
void settings_toggle_buzzer(void);
const char *settings_speed_label(void);
const char *settings_apple_label(void);
const char *settings_song_label(void);
const char *settings_buzzer_label(void);

#endif
