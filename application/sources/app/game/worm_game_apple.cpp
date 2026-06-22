#include "worm_game_apple.h"

// The apple must have a random position on the screen at all time, no duplicate apple overlapping anything
// The apple must be eaten by the worm when the front of the worm touches the apple.
// When the apple is eaten, it must disappear and a new apple must appear randomly on the screen after 2 seconds until it reaches the maximum number of apples.
// If there is still an apple on the screen, it must not be eaten by the worm until it is eaten or disappears after 3 seconds.
// The apple will either spawn when there is no apple left or after 2 seconds of the previous apple being spawn.
// The max number of the apple on the screen should be less than 5 at all times.

//  The worm_max_length is the maximum length the worm can grow, which is determined by the size of the game area and the size of the worm segments.
static uint16_t worm_max_length(void)
{
	uint32_t border_w = (worm_game_border.width != 0) ? worm_game_border.width : SCR_WIDTH;
	uint32_t border_h = (worm_game_border.height != 0) ? worm_game_border.height : SCR_HEIGHT;
	uint32_t max_cols = border_w / WORM_MOVE_STEP;
	uint32_t max_rows = border_h / WORM_MOVE_STEP;
	uint32_t max_length = max_cols * max_rows;

	if (max_length == 0)
	{
		return 1;
	}

	return (max_length > WORM_MAX_TRAIL) ? WORM_MAX_TRAIL : (uint16_t)max_length;
}

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
static uint8_t apple_rect_overlap(uint32_t x0, uint32_t y0, uint32_t w0, uint32_t h0,
								  uint32_t x1, uint32_t y1, uint32_t w1, uint32_t h1)
{
	return !((x0 + w0) <= x1 || (x1 + w1) <= x0 || (y0 + h0) <= y1 || (y1 + h1) <= y0);
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

// Calculate the spawn bounds for the apple based on the game border and screen dimensions, ensuring it does not spawn too close to the edges.
static void apple_get_spawn_bounds(uint32_t *min_x, uint32_t *min_y, uint32_t *max_x, uint32_t *max_y)
{
	uint32_t border_x = BORDER_MARGIN;
	uint32_t border_y = BORDER_MARGIN;
	uint32_t border_w = SCR_WIDTH - (BORDER_MARGIN * 2);
	uint32_t border_h = SCR_HEIGHT - (BORDER_MARGIN * 2);

	if (worm_game_border.width != 0 && worm_game_border.height != 0)
	{
		border_x = worm_game_border.x;
		border_y = worm_game_border.y;
		border_w = worm_game_border.width;
		border_h = worm_game_border.height;
	}

	*min_x = border_x + APPLE_BORDER_PADDING;
	*min_y = border_y + APPLE_BORDER_PADDING;
	*max_x = border_x + border_w - APPLE_BORDER_PADDING - APPLE_WIDTH;
	*max_y = border_y + border_h - APPLE_BORDER_PADDING - APPLE_HEIGHT;
}

// Try to spawn an apple in a valid random position within the spawn bounds, retrying up to APPLE_RANDOM_TRIES times. If successful, initialize the apple slot with the new position and properties. If it fails after all attempts, set the respawn timer for the slot.
static uint8_t apple_spawn_slot(uint8_t index)
{
	uint32_t min_x = 0;
	uint32_t min_y = 0;
	uint32_t max_x = 0;
	uint32_t max_y = 0;

	apple_get_spawn_bounds(&min_x, &min_y, &max_x, &max_y);

	for (uint8_t attempt = 0; attempt < APPLE_RANDOM_TRIES; attempt++)
	{
		uint32_t x = apple_random_u32(min_x, max_x);
		uint32_t y = apple_random_u32(min_y, max_y);

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

// counting the number of apples
void counting_apples(void)
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
			if (apple_rect_overlap(apples_no[i].x, apples_no[i].y, apples_no[i].width, apples_no[i].height,
								   worm_game.x, worm_game.y, worm_game.width, worm_game.height))
			{
				/* worm ate this apple */
				score_inc();
				worm_grow();
				if (worm_game.length >= worm_max_length())
				{
					worm_game_finish(1);
				}
				apples_no[i].is_active = 0;
				apples_no[i].life_seconds = 0;
				apples_no[i].respawn_seconds = APPLE_RESPAWN_SECONDS;
				continue;
			}

			if (apples_no[i].life_seconds > 0)
			{
				apples_no[i].life_seconds--;
			}

			if (apples_no[i].life_seconds == 0)
			{
				apples_no[i].is_active = 0;
				apples_no[i].respawn_seconds = APPLE_RESPAWN_SECONDS;
			}
			continue;
		}

		// If the apple is not active, count down the respawn timer if it's greater than 0.
		if (apples_no[i].respawn_seconds > 0)
		{
			apples_no[i].respawn_seconds--;
		}

		// Try to spawn a new apple in this slot if the respawn timer has reached 0.
		if (apples_no[i].respawn_seconds == 0)
		{
			apple_spawn_slot(i);
		}
	}
}

// Handle incoming messages for the apple system, responding to initialization and tick signals to manage apple state and behavior.
void worm_game_apple_handler(ak_msg_t *msg)
{
	switch (msg->sig)
	{
	case AC_APPLE_INIT:
		apple_init();
		break;

	case AC_APPLE_TICK:
		counting_apples();
		break;

	default:
		break;
	}
}