#include "worm_game_border.h"

#define BORDER_MARGIN 1

worm_game_border_t worm_game_border = {0};

// Initialize the game border settings and draw the border rectangle on the screen.
void border_settings(void)
{
	worm_game_border.x = BORDER_MARGIN;
	worm_game_border.y = BORDER_MARGIN;
	worm_game_border.width = SCR_WIDTH - (BORDER_MARGIN * 2);
	worm_game_border.height = SCR_HEIGHT - (BORDER_MARGIN * 2);

	view_render.drawRect(worm_game_border.x, worm_game_border.y, worm_game_border.width, worm_game_border.height, WHITE);
}

void worm_game_border_handler(ak_msg_t *msg)
{
	(void)msg;
}