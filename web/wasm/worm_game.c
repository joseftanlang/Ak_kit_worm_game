#include "worm_game.h"

worm_state_t worm_game;
worm_border_t worm_border;

static uint16_t worm_clamp_trail_length(uint16_t length)
{
	return (length > WORM_MAX_TRAIL) ? WORM_MAX_TRAIL : length;
}

static uint8_t worm_is_opposite_direction(worm_dir_t current_dir, worm_dir_t next_dir)
{
	return ((current_dir == WORM_DIR_RIGHT && next_dir == WORM_DIR_LEFT) ||
			(current_dir == WORM_DIR_LEFT && next_dir == WORM_DIR_RIGHT) ||
			(current_dir == WORM_DIR_UP && next_dir == WORM_DIR_DOWN) ||
			(current_dir == WORM_DIR_DOWN && next_dir == WORM_DIR_UP));
}

worm_dir_t worm_turn_left(worm_dir_t dir)
{
	switch (dir)
	{
	case WORM_DIR_RIGHT:
		return WORM_DIR_UP;
	case WORM_DIR_UP:
		return WORM_DIR_LEFT;
	case WORM_DIR_LEFT:
		return WORM_DIR_DOWN;
	case WORM_DIR_DOWN:
	default:
		return WORM_DIR_RIGHT;
	}
}

worm_dir_t worm_turn_right(worm_dir_t dir)
{
	switch (dir)
	{
	case WORM_DIR_RIGHT:
		return WORM_DIR_DOWN;
	case WORM_DIR_DOWN:
		return WORM_DIR_LEFT;
	case WORM_DIR_LEFT:
		return WORM_DIR_UP;
	case WORM_DIR_UP:
	default:
		return WORM_DIR_RIGHT;
	}
}

void border_init(void)
{
	worm_border.x = BORDER_MARGIN;
	worm_border.y = BORDER_MARGIN;
	worm_border.width = BORDER_WIDTH;
	worm_border.height = BORDER_HEIGHT;
}

void worm_init(void)
{
	worm_game.x = WORM_START_POSITION_X;
	worm_game.y = WORM_START_POSITION_Y;
	worm_game.width = WORM_MOVE_STEP;
	worm_game.height = WORM_MOVE_STEP;
	worm_game.worm_image = 0;
	worm_game.dir = (uint8_t)WORM_DIR_RIGHT;
	worm_game.length = WORM_INITIAL_LENGTH;
	worm_game.grow_pending = 0;

	for (uint16_t i = 0; i < WORM_MAX_TRAIL; i++)
	{
		worm_game.trail[i].x = 0;
		worm_game.trail[i].y = 0;
	}

	for (uint16_t i = 0; i < worm_game.length; i++)
	{
		uint32_t segment_x = (WORM_START_POSITION_X > (i * WORM_MOVE_STEP))
								 ? (WORM_START_POSITION_X - (i * WORM_MOVE_STEP))
								 : 0;
		worm_game.trail[i].x = (uint8_t)segment_x;
		worm_game.trail[i].y = (uint8_t)WORM_START_POSITION_Y;
	}
}

void worm_set_direction(worm_dir_t d)
{
	if (d == (worm_dir_t)worm_game.dir)
	{
		return;
	}

	if (worm_is_opposite_direction((worm_dir_t)worm_game.dir, d))
	{
		return;
	}

	worm_game.dir = (uint8_t)d;
}

worm_dir_t worm_get_direction(void)
{
	return (worm_dir_t)worm_game.dir;
}

uint8_t worm_advance(void)
{
	worm_point_t worm_next_head = worm_game.trail[0];
	uint8_t hit_self = 0;
	uint16_t current_length = worm_clamp_trail_length(worm_game.length);
	uint16_t occupied_length = current_length;

	if (worm_game.grow_pending == 0 && occupied_length > 1)
	{
		occupied_length--;
	}

	switch (worm_get_direction())
	{
	case WORM_DIR_RIGHT:
		worm_next_head.x = (uint8_t)(worm_next_head.x + WORM_MOVE_STEP);
		break;
	case WORM_DIR_LEFT:
		worm_next_head.x = (worm_next_head.x > WORM_MOVE_STEP) ? (uint8_t)(worm_next_head.x - WORM_MOVE_STEP) : 0;
		break;
	case WORM_DIR_DOWN:
		worm_next_head.y = (uint8_t)(worm_next_head.y + WORM_MOVE_STEP);
		break;
	case WORM_DIR_UP:
		worm_next_head.y = (worm_next_head.y > WORM_MOVE_STEP) ? (uint8_t)(worm_next_head.y - WORM_MOVE_STEP) : 0;
		break;
	}

	uint32_t bx = (worm_border.width != 0) ? worm_border.x : 0;
	uint32_t by = (worm_border.height != 0) ? worm_border.y : 0;
	uint32_t bw = (worm_border.width != 0) ? worm_border.width : SCR_WIDTH;
	uint32_t bh = (worm_border.height != 0) ? worm_border.height : SCR_HEIGHT;

	if ((uint32_t)worm_next_head.x + WORM_MOVE_STEP > bx + bw)
	{
		worm_next_head.x = (uint8_t)bx;
	}
	else if (worm_next_head.x < bx)
	{
		worm_next_head.x = (uint8_t)(bx + bw - WORM_MOVE_STEP);
	}

	if ((uint32_t)worm_next_head.y + WORM_MOVE_STEP > by + bh)
	{
		worm_next_head.y = (uint8_t)by;
	}
	else if (worm_next_head.y < by)
	{
		worm_next_head.y = (uint8_t)(by + bh - WORM_MOVE_STEP);
	}

	for (uint16_t i = 0; i < occupied_length; i++)
	{
		if (worm_game.trail[i].x == worm_next_head.x && worm_game.trail[i].y == worm_next_head.y)
		{
			hit_self = 1;
			break;
		}
	}

	if (hit_self)
	{
		return 1;
	}

	uint16_t next_length = current_length;
	if (worm_game.grow_pending > 0 && next_length < WORM_MAX_TRAIL)
	{
		next_length++;
		worm_game.grow_pending--;
	}

	for (uint16_t i = next_length; i > 1; i--)
	{
		worm_game.trail[i - 1] = worm_game.trail[i - 2];
	}

	worm_game.trail[0] = worm_next_head;
	worm_game.x = worm_next_head.x;
	worm_game.y = worm_next_head.y;
	worm_game.width = WORM_MOVE_STEP;
	worm_game.height = WORM_MOVE_STEP;
	worm_game.length = next_length;

	return 0;
}

void worm_grow(void)
{
	if (worm_game.length + worm_game.grow_pending < WORM_MAX_TRAIL)
	{
		worm_game.grow_pending++;
	}
}
