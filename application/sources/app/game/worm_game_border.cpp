#include "worm_game_border.h"

#define BORDER_MARGIN 1

worm_game_border_t worm_game_border = {
	.x = BORDER_MARGIN,
	.y = BORDER_MARGIN,
	.width = 125,
	.height = 60};

// Initialize the game border settings and draw the border rectangle on the screen.
void border_settings(void)
{
	worm_game_border.x = BORDER_MARGIN;
	worm_game_border.y = BORDER_MARGIN;
	worm_game_border.width = 125;
	worm_game_border.height = 60;

	// Draw the border rectangle on the screen with a white color.
	view_render.drawRect(
		(int16_t)BORDER_MARGIN,
		(int16_t)BORDER_MARGIN,
		(int16_t)(SCR_WIDTH - (BORDER_MARGIN * 2)),
		(int16_t)(SCR_HEIGHT - (BORDER_MARGIN * 2)),
		WHITE);
}

void worm_game_border_handler(ak_msg_t *msg)
{
	(void)msg;
}