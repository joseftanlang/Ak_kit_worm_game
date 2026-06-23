#include "worm_game_apple.h"

// The apple must have a random position on the screen at all time, no duplicate apple overlapping anything
// The apple must be eaten by the worm when the front of the worm touches the apple.
// When the apple is eaten, it must disappear and a new apple must appear randomly on the screen after 2 seconds until it reaches the maximum number of apples.
// If there is still an apple on the screen, it must not be eaten by the worm until it is eaten or disappears after 3 seconds.
// The apple will either spawn when there is no apple left or after 2 seconds of the previous apple being spawn.
// The max number of the apple on the screen should be less than 5 at all times.



worm_game_apple_t apples_no[MAX_APPLES];

// Generate a random number between min_value and max_value (inclusive) using the rand() function. If max_value is less than or equal to min_value, return min_value.

static uint32_t apple_random_u32(uint32_t min_value, uint32_t max_value)
{
	if (max_value <= min_value)
	{
		return min_value;
	}

	return min_value + (uint32_t)(rand() % (max_value - min_value + 1));
}

// Check if two rectangles overlap.
static uint8_t apple_rect_overlap(
	uint32_t x0, uint32_t y0, uint32_t w0, uint32_t h0,
	uint32_t x1, uint32_t y1, uint32_t w1, uint32_t h1)
{
	return !(
		(x0 >= x1 + w1) ||
		(x0 + w0 <= x1) ||
		(y0 >= y1 + h1) ||
		(y0 + h0 <= y1));
}

// Check if a position is valid for an apple (not overlapping with any other object).
static uint8_t apple_position_is_valid(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t ignore_index)
{
	for (uint8_t i = 0; i < MAX_APPLES; i++)
	{
		if (i == ignore_index)
		{
			continue;
		}

		if (apples_no[i].is_active && apple_rect_overlap(x, y, w, h, apples_no[i].x, apples_no[i].y, apples_no[i].width, apples_no[i].height))
		{
			return 0;
		}
	}

	if (apple_rect_overlap(x, y, w, h, worm_game.x, worm_game.y, worm_game.width, worm_game.height))
	{
		return 0;
	}

	return 1;
}

// Get the active apple limit from the game settings, ensuring it is between 1 and MAX_APPLES.
static uint8_t apple_get_active_limit(void)
{
	uint8_t limit = scr_game_setting_get_apple_count();

	if (limit < 1)
	{
		limit = 1;
	}

	if (limit > MAX_APPLES)
	{
		limit = MAX_APPLES;
	}

	return limit;
}

// Try to spawn an apple in a valid random position within the spawn bounds, retrying up to APPLE_RANDOM_TRIES times. If successful, initialize the apple slot with the new position and properties. If it fails after all attempts, set the respawn timer for the slot.
static uint8_t apple_spawn_slot(uint8_t index)
{
	uint32_t bx = (worm_game_border.width != 0) ? worm_game_border.x : 0;
	uint32_t by = (worm_game_border.height != 0) ? worm_game_border.y : 0;
	uint32_t bw = (worm_game_border.width != 0) ? worm_game_border.width : SCR_WIDTH;
	uint32_t bh = (worm_game_border.height != 0) ? worm_game_border.height : SCR_HEIGHT;

	uint32_t columns = bw / WORM_MOVE_STEP;
	uint32_t rows = bh / WORM_MOVE_STEP;

	if (columns < 3 || rows < 3)
	{
		return 0;
	}

	for (uint8_t attempt = 0; attempt < APPLE_RANDOM_TRIES; attempt++)
	{
		uint32_t x = bx + (apple_random_u32(1, columns - 2) * WORM_MOVE_STEP);
		uint32_t y = by + (apple_random_u32(1, rows - 2) * WORM_MOVE_STEP);

		if (!apple_position_is_valid(x, y, APPLE_WIDTH, APPLE_HEIGHT, index))
		{
			continue;
		}

		apples_no[index].x = x;
		apples_no[index].y = y;
		apples_no[index].width = APPLE_WIDTH;
		apples_no[index].height = APPLE_HEIGHT;
		apples_no[index].is_active = 1;
		apples_no[index].life_seconds = APPLE_LIFE_SECONDS;
		apples_no[index].respawn_seconds = 0;
		apples_no[index].apple_image = (uint8_t)(rand() & 1);
		return 1;
	}

	apples_no[index].respawn_seconds = 1;
	return 0;
}

// Reset the apple slot at the given index to its default inactive state, clearing its position and timers.
static void apple_reset_slot(uint8_t index)
{
	apples_no[index].x = 0;
	apples_no[index].y = 0;
	apples_no[index].width = APPLE_WIDTH;
	apples_no[index].height = APPLE_HEIGHT;
	apples_no[index].is_active = 0;
	apples_no[index].life_seconds = 0;
	apples_no[index].respawn_seconds = 0;
	apples_no[index].apple_image = 0;
}

// Initialize the apple system by resetting all apple slots and spawning the initial set of apples based on the active limit from the game settings. Also sets up a periodic timer to handle apple updates.
void apple_init(void)
{
	uint8_t active_limit = apple_get_active_limit();

	timer_remove_attr(WORM_GAME_APPLE_ID, AC_APPLE_TICK);
	timer_set(WORM_GAME_APPLE_ID, AC_APPLE_TICK, 1000, TIMER_PERIODIC);

	for (uint8_t i = 0; i < MAX_APPLES; i++)
	{
		apple_reset_slot(i);
		if (i < active_limit)
		{
			apples_no[i].respawn_seconds = (uint8_t)(i * APPLE_RESPAWN_SECONDS);
			apple_spawn_slot(i);
		}
	}
}

// Check for collision between worm head and active apples
static void apple_collision_check(void)
{
	if (worm_game_is_finished())
	{
		return;
	}

	uint8_t active_limit = apple_get_active_limit();

	for (uint8_t i = 0; i < active_limit; i++)
	{
		if (apples_no[i].is_active)
		{
			/* GRID SAFE COLLISION USING GLOBAL WORM COORDS */
			if (worm_game.x == apples_no[i].x && worm_game.y == apples_no[i].y)
			{
				score_inc();
				worm_grow();

				apples_no[i].is_active = 0;
				apples_no[i].respawn_seconds = APPLE_RESPAWN_SECONDS;
			}
		}
	}
}

// 1-second periodic timer tick to update apple lifespans and respawn timers
static void apple_timer_tick(void)
{
	if (worm_game_is_finished())
	{
		return;
	}

	uint8_t active_limit = apple_get_active_limit();

	for (uint8_t i = active_limit; i < MAX_APPLES; i++)
	{
		apple_reset_slot(i);
	}

	for (uint8_t i = 0; i < active_limit; i++)
	{
		if (apples_no[i].is_active)
		{
			if (apples_no[i].life_seconds > 0)
			{
				apples_no[i].life_seconds--;
			}

			if (apples_no[i].life_seconds == 0)
			{
				apples_no[i].is_active = 0;
				apples_no[i].respawn_seconds = APPLE_RESPAWN_SECONDS;
			}
		}
		else
		{
			if (apples_no[i].respawn_seconds > 0)
			{
				apples_no[i].respawn_seconds--;
			}

			if (apples_no[i].respawn_seconds == 0)
			{
				apple_spawn_slot(i);
			}
		}
	}
}

// Handle messages related to the eating effect
void worm_game_apple_handler(ak_msg_t *msg)
{
	switch (msg->sig)
	{
	case AC_APPLE_INIT:
		apple_init();
		break;

	case AC_APPLE_TICK:
		apple_timer_tick();
		break;

	case AC_APPLE_COLLISION_CHECK:
		apple_collision_check();
		break;

	default:
		break;
	}
}