#include "apple.h"
#include "worm_game.h"
#include <stdlib.h>

worm_apple_t apples[MAX_APPLES];

static uint32_t apple_random_u32(uint32_t min_value, uint32_t max_value)
{
	if (max_value <= min_value)
	{
		return min_value;
	}
	return min_value + (uint32_t)(rand() % (max_value - min_value + 1));
}

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

static uint8_t apple_position_is_valid(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t ignore_index)
{
	for (uint8_t i = 0; i < MAX_APPLES; i++)
	{
		if (i == ignore_index)
		{
			continue;
		}

		if (apples[i].is_active &&
			apple_rect_overlap(x, y, w, h, apples[i].x, apples[i].y, apples[i].width, apples[i].height))
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

static void apple_reset_slot(uint8_t index)
{
	apples[index].x = 0;
	apples[index].y = 0;
	apples[index].width = APPLE_WIDTH;
	apples[index].height = APPLE_HEIGHT;
	apples[index].is_active = 0;
	apples[index].life_seconds = 0;
	apples[index].respawn_seconds = 0;
	apples[index].apple_image = 0;
}

static uint8_t apple_spawn_slot(uint8_t index)
{
	uint32_t bx = (worm_border.width != 0) ? worm_border.x : 0;
	uint32_t by = (worm_border.height != 0) ? worm_border.y : 0;
	uint32_t bw = (worm_border.width != 0) ? worm_border.width : SCR_WIDTH;
	uint32_t bh = (worm_border.height != 0) ? worm_border.height : SCR_HEIGHT;

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

		apples[index].x = x;
		apples[index].y = y;
		apples[index].width = APPLE_WIDTH;
		apples[index].height = APPLE_HEIGHT;
		apples[index].is_active = 1;
		apples[index].life_seconds = APPLE_LIFE_SECONDS;
		apples[index].respawn_seconds = 0;
		apples[index].apple_image = (uint8_t)(rand() & 1);
		return 1;
	}

	apples[index].respawn_seconds = 1;
	return 0;
}

void apple_init(uint8_t apple_limit)
{
	if (apple_limit < 1)
	{
		apple_limit = 1;
	}
	if (apple_limit > MAX_APPLES)
	{
		apple_limit = MAX_APPLES;
	}

	for (uint8_t i = 0; i < MAX_APPLES; i++)
	{
		apple_reset_slot(i);
		if (i < apple_limit)
		{
			apples[i].respawn_seconds = (uint8_t)(i * APPLE_RESPAWN_SECONDS);
			apple_spawn_slot(i);
		}
	}
}

void apple_collision_check(uint8_t apple_limit, uint8_t *ate_apple)
{
	*ate_apple = 0;

	if (apple_limit > MAX_APPLES)
	{
		apple_limit = MAX_APPLES;
	}

	for (uint8_t i = 0; i < apple_limit; i++)
	{
		if (apples[i].is_active)
		{
			if (apple_rect_overlap(
					worm_game.x, worm_game.y, worm_game.width, worm_game.height,
					apples[i].x, apples[i].y, apples[i].width, apples[i].height))
			{
				*ate_apple = 1;
				apples[i].is_active = 0;
				apples[i].respawn_seconds = APPLE_RESPAWN_SECONDS;
			}
		}
	}
}

void apple_timer_tick(uint8_t apple_limit)
{
	if (apple_limit > MAX_APPLES)
	{
		apple_limit = MAX_APPLES;
	}

	for (uint8_t i = apple_limit; i < MAX_APPLES; i++)
	{
		apple_reset_slot(i);
	}

	for (uint8_t i = 0; i < apple_limit; i++)
	{
		if (apples[i].is_active)
		{
			if (apples[i].life_seconds > 0)
			{
				apples[i].life_seconds--;
			}

			if (apples[i].life_seconds == 0)
			{
				apples[i].is_active = 0;
				apples[i].respawn_seconds = APPLE_RESPAWN_SECONDS;
			}
		}
		else
		{
			if (apples[i].respawn_seconds > 0)
			{
				apples[i].respawn_seconds--;
			}

			if (apples[i].respawn_seconds == 0)
			{
				apple_spawn_slot(i);
			}
		}
	}
}
