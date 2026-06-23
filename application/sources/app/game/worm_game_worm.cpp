#include "worm_game_worm.h"

worm_game_t worm_game = {0};

static inline void get_worm_head(uint32_t *x, uint32_t *y)
{
    *x = worm_game.trail[0].x;
    *y = worm_game.trail[0].y;
}

// worm_clamp_trail_length ensures that the worm's trail length does not exceed the maximum allowed length (WORM_MAX_TRAIL).
static uint16_t worm_clamp_trail_length(uint16_t length)
{
	return (length > WORM_MAX_TRAIL) ? WORM_MAX_TRAIL : length;
}
// worm_is_opposite_direction checks if the next direction is directly opposite to the current direction, which is not allowed in the game.
static uint8_t worm_is_opposite_direction(worm_game_dir_t current_dir, worm_game_dir_t next_dir)
{
	return ((current_dir == WORM_DIR_RIGHT && next_dir == WORM_DIR_LEFT) ||
			(current_dir == WORM_DIR_LEFT && next_dir == WORM_DIR_RIGHT) ||
			(current_dir == WORM_DIR_UP && next_dir == WORM_DIR_DOWN) ||
			(current_dir == WORM_DIR_DOWN && next_dir == WORM_DIR_UP));
}
// worm_turn_left returns the new direction when the worm turns left from its current direction.
static worm_game_dir_t worm_turn_left(worm_game_dir_t dir)
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
// worm_turn_right returns the new direction when the worm turns right from its current direction.
static worm_game_dir_t worm_turn_right(worm_game_dir_t dir)
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
// worm_init initializes the worm's position, direction, length, and trail segments to their starting values at the beginning of the game.
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
	// Initialize the worm's trail segments to the starting position, with the head at the front and the body segments trailing behind it.
	for (uint16_t i = 0; i < WORM_MAX_TRAIL; i++)
	{
		view_render.drawRect(worm_game.trail[i].x, worm_game.trail[i].y, WORM_MOVE_STEP, WORM_MOVE_STEP, WHITE);
		worm_game.trail[i].x = 0;
		worm_game.trail[i].y = 0;
	}
	// Set the initial trail segments based on the starting position and initial length of the worm, with each segment positioned behind the head.
	for (uint16_t i = 0; i < worm_game.length; i++)
	{
		uint32_t segment_x = (WORM_START_POSITION_X > (i * WORM_MOVE_STEP)) ? (WORM_START_POSITION_X - (i * WORM_MOVE_STEP)) : 0;
		view_render.drawRect((int16_t)segment_x, (int16_t)WORM_START_POSITION_Y, WORM_MOVE_STEP, WORM_MOVE_STEP, WHITE);
		worm_game.trail[i].x = (uint8_t)segment_x;
		worm_game.trail[i].y = (uint8_t)WORM_START_POSITION_Y;
	}
}

// worm_set_direction changes the worm's direction to the specified direction, ensuring that it does not reverse directly into itself.
// It checks if the new direction is the same as the current direction or if it is opposite, and only updates the direction if it is valid.
void worm_set_direction(worm_game_dir_t d)
{
	if (d == (worm_game_dir_t)worm_game.dir)
	{
		return;
	}

	if (worm_is_opposite_direction((worm_game_dir_t)worm_game.dir, d))
	{
		return;
	}

	worm_game.dir = (uint8_t)d;
}

// worm_get_direction returns the current direction of the worm.
worm_game_dir_t worm_get_direction(void)
{
	return (worm_game_dir_t)worm_game.dir;
}

// worm_advance moves the worm forward by one step in its current direction, updating its position and trail segments.
uint8_t worm_advance(void)
{
	worm_game_point_t worm_next_head = worm_game.trail[0];
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

	/* wrap around inside the game border (or screen if border not set) */
	uint32_t bx = (worm_game_border.width != 0) ? worm_game_border.x : 0;
	uint32_t by = (worm_game_border.height != 0) ? worm_game_border.y : 0;
	uint32_t bw = (worm_game_border.width != 0) ? worm_game_border.width : SCR_WIDTH;
	uint32_t bh = (worm_game_border.height != 0) ? worm_game_border.height : SCR_HEIGHT;

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

// worm_grow increases the worm's length by one segment, which will be added to the trail in the next update cycle.
void worm_grow(void)
{
	if (worm_game.length + worm_game.grow_pending < WORM_MAX_TRAIL)
	{
		worm_game.grow_pending++;
	}
}

// worm_game_worm_handler processes messages related to the worm's behavior and updates its state accordingly.
void worm_game_worm_handler(ak_msg_t *msg)
{
	switch (msg->sig)
	{
	case AC_WORM_INIT:
		worm_init();
		break;

	case AC_WORM_TICK:
		if (!worm_game_is_finished())
		{
			if (worm_advance())
			{
				lives_dec();
				if (lives_get() == MIN_LIVES)
				{
					worm_game_finish(0);
				}
			}
			task_post_pure_msg(WORM_GAME_APPLE_ID, AC_APPLE_COLLISION_CHECK);

			view_render.drawRect(
				worm_game.trail[0].x,
				worm_game.trail[0].y,
				WORM_MOVE_STEP,
				WORM_MOVE_STEP,
				WHITE);
		}
		break;

	case AC_WORM_SET_DIR_RIGHT:
		worm_set_direction(WORM_DIR_RIGHT);
		break;

	case AC_WORM_SET_DIR_DOWN:
		worm_set_direction(worm_turn_right(worm_get_direction()));
		break;

	case AC_WORM_SET_DIR_LEFT:
		worm_set_direction(WORM_DIR_LEFT);
		break;

	case AC_WORM_SET_DIR_UP:
		worm_set_direction(worm_turn_left(worm_get_direction()));
		break;

	case AC_WORM_GROW:
		worm_grow();
		score_commit_current();
		break;

	default:
		break;
	}
}