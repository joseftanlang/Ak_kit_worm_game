#include "worm_game_apple.h"
#include "worm_game_border.h"
#include "scr_worm.h"

#include <stdlib.h>

// The apple must have a random position on the screen at all time, no duplicate apple overlapping anything
// The apple must be eaten by the worm when the front of the worm touches the apple.
// When the apple is eaten, it must disappear and a new apple must appear randomly on the screen after 2 seconds until it reaches the maximum number of apples.
// If there is still an apple on the screen, it must not be eaten by the worm until it is eaten or disappears after 3 seconds.
// The apple will either spawn when there is no apple left or after 2 seconds of the previous apple being spawn. 
// The max number of the apple on the screen should be less than 5 at all times.

#define BORDER_MARGIN 1
#define APPLE_LIFE_SECONDS 3
#define APPLE_RESPAWN_SECONDS 2
#define APPLE_RANDOM_TRIES 32
#define APPLE_BORDER_PADDING 1

static uint16_t worm_max_length(void) {
	uint32_t border_w = (game_border.width != 0) ? game_border.width : SCR_WIDTH;
	uint32_t border_h = (game_border.height != 0) ? game_border.height : SCR_HEIGHT;
	uint32_t max_cols = border_w / WORM_MOVE_STEP;
	uint32_t max_rows = border_h / WORM_MOVE_STEP;
	uint32_t max_length = max_cols * max_rows;

	if (max_length == 0) {
		return 1;
	}

	return (max_length > WORM_MAX_TRAIL) ? WORM_MAX_TRAIL : (uint16_t)max_length;
}

worm_game_apple_t apples_no[MAX_APPLES];

static uint32_t apple_random_u32(uint32_t min_value, uint32_t max_value) {
	if (max_value <= min_value) {
		return min_value;
	}

	return min_value + (uint32_t)(rand() % (max_value - min_value + 1));
}

static uint8_t apple_rect_overlap(uint32_t x0, uint32_t y0, uint32_t w0, uint32_t h0,
								uint32_t x1, uint32_t y1, uint32_t w1, uint32_t h1) {
	return !((x0 + w0) <= x1 || (x1 + w1) <= x0 || (y0 + h0) <= y1 || (y1 + h1) <= y0);
}

static uint8_t apple_position_is_valid(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t ignore_index) {
	for (uint8_t i = 0; i < MAX_APPLES; i++) {
		if (i == ignore_index) {
			continue;
		}

		if (apples_no[i].is_active && apple_rect_overlap(x, y, w, h, apples_no[i].x, apples_no[i].y, apples_no[i].width, apples_no[i].height)) {
			return 0;
		}
	}

	if (apple_rect_overlap(x, y, w, h, game_worm.x, game_worm.y, game_worm.width, game_worm.height)) {
		return 0;
	}

	return 1;
}

static void apple_get_spawn_bounds(uint32_t* min_x, uint32_t* min_y, uint32_t* max_x, uint32_t* max_y) {
	uint32_t border_x = BORDER_MARGIN;
	uint32_t border_y = BORDER_MARGIN;
	uint32_t border_w = SCR_WIDTH - (BORDER_MARGIN * 2);
	uint32_t border_h = SCR_HEIGHT - (BORDER_MARGIN * 2);

	if (game_border.width != 0 && game_border.height != 0) {
		border_x = game_border.x;
		border_y = game_border.y;
		border_w = game_border.width;
		border_h = game_border.height;
	}

	*min_x = border_x + APPLE_BORDER_PADDING;
	*min_y = border_y + APPLE_BORDER_PADDING;
	*max_x = border_x + border_w - APPLE_BORDER_PADDING - APPLE_WIDTH;
	*max_y = border_y + border_h - APPLE_BORDER_PADDING - APPLE_HEIGHT;
}

static uint8_t apple_spawn_slot(uint8_t index) {
	uint32_t min_x = 0;
	uint32_t min_y = 0;
	uint32_t max_x = 0;
	uint32_t max_y = 0;

	apple_get_spawn_bounds(&min_x, &min_y, &max_x, &max_y);

	for (uint8_t attempt = 0; attempt < APPLE_RANDOM_TRIES; attempt++) {
		uint32_t x = apple_random_u32(min_x, max_x);
		uint32_t y = apple_random_u32(min_y, max_y);

		if (!apple_position_is_valid(x, y, APPLE_WIDTH, APPLE_HEIGHT, index)) {
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

static void apple_reset_slot(uint8_t index) {
	apples_no[index].x = 0;
	apples_no[index].y = 0;
	apples_no[index].width = APPLE_WIDTH;
	apples_no[index].height = APPLE_HEIGHT;
	apples_no[index].is_active = 0;
	apples_no[index].life_seconds = 0;
	apples_no[index].respawn_seconds = 0;
	apples_no[index].apple_image = 0;
}

void apple_init(void) {
	timer_remove_attr(GAME_APPLE_ID, AC_APPLE_TICK);
	timer_set(GAME_APPLE_ID, AC_APPLE_TICK, 1000, TIMER_PERIODIC);

	for (uint8_t i = 0; i < MAX_APPLES; i++) {
		apple_reset_slot(i);
		apples_no[i].respawn_seconds = (uint8_t)(i * APPLE_RESPAWN_SECONDS);
	}

	apple_spawn_slot(0);
}

// counting the number of apples
void counting_apples(void)
{
	if (worm_game_is_finished()) {
		return;
	}

	for (uint8_t i = 0; i < MAX_APPLES; i++) {
		if (apples_no[i].is_active) {
			if (apple_rect_overlap(apples_no[i].x, apples_no[i].y, apples_no[i].width, apples_no[i].height,
							   game_worm.x, game_worm.y, game_worm.width, game_worm.height)) {
				/* worm ate this apple */
				score_inc();
				worm_grow();
				if (game_worm.length >= worm_max_length()) {
					worm_game_finish(1);
				}
				apples_no[i].is_active = 0;
				apples_no[i].life_seconds = 0;
				apples_no[i].respawn_seconds = APPLE_RESPAWN_SECONDS;
				continue;
			}

			if (apples_no[i].life_seconds > 0) {
				apples_no[i].life_seconds--;
			}

			if (apples_no[i].life_seconds == 0) {
				apples_no[i].is_active = 0;
				apples_no[i].respawn_seconds = APPLE_RESPAWN_SECONDS;
			}
			continue;
		}

		if (apples_no[i].respawn_seconds > 0) {
			apples_no[i].respawn_seconds--;
		}

		if (apples_no[i].respawn_seconds == 0) {
			apple_spawn_slot(i);
		}
	}
}

void game_apple_handler(ak_msg_t *msg)
{
	switch (msg->sig) {
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