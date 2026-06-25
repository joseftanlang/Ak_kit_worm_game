#include "app.h"
#include "apple.h"
#include "lives.h"
#include "render.h"
#include "score.h"
#include "settings.h"
#include "worm_game.h"
#include <emscripten.h>
#include <stdlib.h>
#include <string.h>

typedef enum
{
	SCREEN_MENU = 0,
	SCREEN_WORM = 1,
	SCREEN_CHARTS = 2,
	SCREEN_SETTINGS = 3
} screen_t;

static screen_t current_screen = SCREEN_MENU;
static int menu_index = 0;
static int settings_selected = 0;

static uint8_t worm_finished = 0;
static uint8_t worm_won = 0;

static uint32_t worm_tick_accum = 0;
static uint32_t apple_tick_accum = 0;
static uint32_t chart_tick_accum = 0;
static uint32_t music_tick_accum = 0;

static int pending_event = EVENT_NONE;

static void push_event(int event)
{
	if (pending_event == EVENT_NONE)
	{
		pending_event = event;
	}
}

static void worm_game_reset(void)
{
	worm_finished = 0;
	worm_won = 0;
	worm_tick_accum = 0;
	apple_tick_accum = 0;
}

static void worm_game_finish(uint8_t won)
{
	if (worm_finished)
	{
		return;
	}
	worm_finished = 1;
	worm_won = won ? 1 : 0;
	score_commit_current();
	push_event(EVENT_GAME_OVER);
}

static void worm_start_new_game(void)
{
	worm_game_reset();
	border_init();
	score_init();
	worm_init();
	lives_init();
	apple_init(settings_get_apple_count());
	worm_tick_accum = 0;
	apple_tick_accum = 0;
	if (settings_is_buzzer_enabled())
	{
		push_event(EVENT_MUSIC_START);
	}
	render_worm_screen(0, 0);
}

static void enter_screen(screen_t screen)
{
	current_screen = screen;

	switch (screen)
	{
	case SCREEN_MENU:
		menu_index = 0;
		render_menu(menu_index);
		break;
	case SCREEN_WORM:
		worm_start_new_game();
		break;
	case SCREEN_CHARTS:
		render_charts_reset();
		chart_tick_accum = 0;
		render_charts_screen();
		break;
	case SCREEN_SETTINGS:
		settings_selected = 0;
		render_settings(settings_selected);
		break;
	}
}

EMSCRIPTEN_KEEPALIVE
void game_init(uint32_t top1, uint32_t top2, uint32_t top3,
			   uint8_t speed, uint8_t apples, uint8_t song, uint8_t buzzer)
{
	settings_init();
	settings_import(speed, apples, song, buzzer);
	score_import_top(top1, top2, top3);
	enter_screen(SCREEN_MENU);
}

EMSCRIPTEN_KEEPALIVE
uint8_t *game_framebuffer(void)
{
	return (uint8_t *)fb_get_buffer();
}

EMSCRIPTEN_KEEPALIVE
int game_get_event(void)
{
	int e = pending_event;
	pending_event = EVENT_NONE;
	return e;
}

EMSCRIPTEN_KEEPALIVE
void game_get_settings(uint8_t *speed, uint8_t *apples, uint8_t *song, uint8_t *buzzer)
{
	settings_export(speed, apples, song, buzzer);
}

EMSCRIPTEN_KEEPALIVE
void game_get_top_scores(uint32_t *top1, uint32_t *top2, uint32_t *top3)
{
	*top1 = score_top_get(0);
	*top2 = score_top_get(1);
	*top3 = score_top_get(2);
}

static void worm_do_tick(void)
{
	if (worm_finished)
	{
		render_worm_screen(1, worm_won);
		return;
	}

	if (worm_advance())
	{
		lives_dec();
		if (lives_get() == MIN_LIVES)
		{
			worm_game_finish(0);
			render_worm_screen(1, 0);
			return;
		}
	}

	uint8_t ate = 0;
	apple_collision_check(settings_get_apple_count(), &ate);
	if (ate)
	{
		score_inc();
		worm_grow();
		score_commit_current();
		push_event(EVENT_EAT);
	}

	render_worm_screen(0, 0);
}

EMSCRIPTEN_KEEPALIVE
void game_update(uint32_t dt_ms)
{
	if (current_screen == SCREEN_WORM && !worm_finished)
	{
		worm_tick_accum += dt_ms;
		apple_tick_accum += dt_ms;
		music_tick_accum += dt_ms;

		uint16_t worm_interval = settings_get_worm_tick_ms();
		while (worm_tick_accum >= worm_interval)
		{
			worm_tick_accum -= worm_interval;
			worm_do_tick();
			if (worm_finished)
			{
				break;
			}
		}

		if (!worm_finished)
		{
			while (apple_tick_accum >= 1000)
			{
				apple_tick_accum -= 1000;
				apple_timer_tick(settings_get_apple_count());
			}

			if (settings_is_buzzer_enabled())
			{
				while (music_tick_accum >= 1000)
				{
					music_tick_accum -= 1000;
					push_event(EVENT_MUSIC_LOOP);
				}
			}
		}
	}
	else if (current_screen == SCREEN_CHARTS)
	{
		chart_tick_accum += dt_ms;
		while (chart_tick_accum >= CHART_ANIM_INTERVAL_MS)
		{
			chart_tick_accum -= CHART_ANIM_INTERVAL_MS;
			render_charts_tick();
			render_charts_screen();
		}
	}
}

EMSCRIPTEN_KEEPALIVE
void game_input(int key)
{
	switch (current_screen)
	{
	case SCREEN_MENU:
		if (key == KEY_UP)
		{
			menu_index = (menu_index > 0) ? menu_index - 1 : 2;
			push_event(EVENT_CLICK);
			render_menu(menu_index);
		}
		else if (key == KEY_DOWN)
		{
			menu_index = (menu_index + 1) % 3;
			push_event(EVENT_CLICK);
			render_menu(menu_index);
		}
		else if (key == KEY_MODE)
		{
			push_event(EVENT_CLICK);
			if (menu_index == 0)
			{
				enter_screen(SCREEN_WORM);
			}
			else if (menu_index == 1)
			{
				enter_screen(SCREEN_CHARTS);
			}
			else
			{
				enter_screen(SCREEN_SETTINGS);
			}
		}
		break;

	case SCREEN_WORM:
		if (worm_finished)
		{
			if (key == KEY_UP || key == KEY_DOWN)
			{
				push_event(EVENT_CLICK);
				enter_screen(SCREEN_MENU);
			}
			else if (key == KEY_MODE)
			{
				push_event(EVENT_CLICK);
				enter_screen(SCREEN_CHARTS);
			}
		}
		else
		{
			if (key == KEY_UP)
			{
				worm_set_direction(worm_turn_left(worm_get_direction()));
				push_event(EVENT_CLICK);
			}
			else if (key == KEY_DOWN)
			{
				worm_set_direction(worm_turn_right(worm_get_direction()));
				push_event(EVENT_CLICK);
			}
			else if (key == KEY_MODE)
			{
				push_event(EVENT_CLICK);
				enter_screen(SCREEN_MENU);
			}
		}
		break;

	case SCREEN_CHARTS:
		push_event(EVENT_CLICK);
		enter_screen(SCREEN_MENU);
		break;

	case SCREEN_SETTINGS:
		if (key == KEY_UP)
		{
			settings_selected = (settings_selected == 0) ? 3 : settings_selected - 1;
			push_event(EVENT_CLICK);
			render_settings(settings_selected);
		}
		else if (key == KEY_DOWN)
		{
			settings_selected = (settings_selected + 1) % 4;
			push_event(EVENT_CLICK);
			render_settings(settings_selected);
		}
		else if (key == KEY_MODE)
		{
			push_event(EVENT_CLICK);
			switch (settings_selected)
			{
			case 0:
				settings_cycle_speed();
				break;
			case 1:
				settings_cycle_apples();
				break;
			case 2:
				settings_cycle_song();
				break;
			case 3:
				settings_toggle_buzzer();
				break;
			}
			push_event(EVENT_SETTINGS_CHANGED);
			render_settings(settings_selected);
		}
		else if (key == KEY_LONG_MODE)
		{
			push_event(EVENT_CLICK);
			enter_screen(SCREEN_MENU);
		}
		break;
	}
}
