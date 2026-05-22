#include "worm_game_border.h"

#define BORDER_MARGIN 1

worm_game_border_t game_border = {0};

void border_settings(void) {
	game_border.x = BORDER_MARGIN;
	game_border.y = BORDER_MARGIN;
	game_border.width = SCR_WIDTH - (BORDER_MARGIN * 2);
	game_border.height = SCR_HEIGHT - (BORDER_MARGIN * 2);

	view_render.drawRect(game_border.x, game_border.y, game_border.width, game_border.height, WHITE);
}


void game_border_handler(ak_msg_t* msg) {
	(void)msg;
}