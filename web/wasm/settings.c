#include "settings.h"
#include "config.h"

static uint8_t setting_worm_speed = 3;
static uint8_t setting_apple_count = 1;
static uint8_t setting_song_index = 0;
static uint8_t setting_buzzer_enabled = 0;

static const uint16_t setting_worm_tick_intervals_ms[SETTING_WORM_SPEED_MAX] = {
	180, 150, 110, 90, 50};

static const char *setting_speed_values[] = {"1", "2", "3", "4", "5"};
static const char *setting_apple_values[] = {"1", "2", "3", "4", "5", "6", "7", "8"};
static const char *setting_song_values[] = {"ROCK", "MARIO", "PIRATES", "BUS", "XMAS"};

void settings_init(void)
{
}

void settings_import(uint8_t speed, uint8_t apples, uint8_t song, uint8_t buzzer)
{
	if (speed >= SETTING_WORM_SPEED_MIN && speed <= SETTING_WORM_SPEED_MAX)
	{
		setting_worm_speed = speed;
	}
	if (apples >= SETTING_APPLE_COUNT_MIN && apples <= SETTING_APPLE_COUNT_MAX)
	{
		setting_apple_count = apples;
	}
	if (song < SETTING_SONG_COUNT)
	{
		setting_song_index = song;
	}
	setting_buzzer_enabled = buzzer ? 1 : 0;
}

void settings_export(uint8_t *speed, uint8_t *apples, uint8_t *song, uint8_t *buzzer)
{
	*speed = setting_worm_speed;
	*apples = setting_apple_count;
	*song = setting_song_index;
	*buzzer = setting_buzzer_enabled;
}

uint16_t settings_get_worm_tick_ms(void)
{
	uint8_t speed = setting_worm_speed;
	if (speed < SETTING_WORM_SPEED_MIN)
	{
		speed = SETTING_WORM_SPEED_MIN;
	}
	if (speed > SETTING_WORM_SPEED_MAX)
	{
		speed = SETTING_WORM_SPEED_MAX;
	}
	return setting_worm_tick_intervals_ms[speed - 1];
}

uint8_t settings_get_apple_count(void)
{
	if (setting_apple_count < SETTING_APPLE_COUNT_MIN)
	{
		return SETTING_APPLE_COUNT_MIN;
	}
	if (setting_apple_count > SETTING_APPLE_COUNT_MAX)
	{
		return SETTING_APPLE_COUNT_MAX;
	}
	return setting_apple_count;
}

uint8_t settings_get_song_index(void)
{
	if (setting_song_index >= SETTING_SONG_COUNT)
	{
		return 0;
	}
	return setting_song_index;
}

uint8_t settings_is_buzzer_enabled(void)
{
	return setting_buzzer_enabled;
}

void settings_cycle_speed(void)
{
	setting_worm_speed = (setting_worm_speed < SETTING_WORM_SPEED_MAX)
							 ? setting_worm_speed + 1
							 : SETTING_WORM_SPEED_MIN;
}

void settings_cycle_apples(void)
{
	setting_apple_count = (setting_apple_count < SETTING_APPLE_COUNT_MAX)
							  ? setting_apple_count + 1
							  : SETTING_APPLE_COUNT_MIN;
}

void settings_cycle_song(void)
{
	setting_song_index = (setting_song_index + 1) % SETTING_SONG_COUNT;
}

void settings_toggle_buzzer(void)
{
	setting_buzzer_enabled = !setting_buzzer_enabled;
}

const char *settings_speed_label(void)
{
	return setting_speed_values[setting_worm_speed - 1];
}

const char *settings_apple_label(void)
{
	return setting_apple_values[settings_get_apple_count() - 1];
}

const char *settings_song_label(void)
{
	uint8_t idx = settings_get_song_index();
	return setting_song_values[idx];
}

const char *settings_buzzer_label(void)
{
	return setting_buzzer_enabled ? "ON" : "OFF";
}
