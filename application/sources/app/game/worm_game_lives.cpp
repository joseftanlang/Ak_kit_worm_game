#include "worm_game_lives.h"

/* Object-only lives state. Rendering and restart flow are handled by the screen. */
// The lives icon will be displayed at the top right corner of the screen. The amount of lives will decrease by 1 when the worm crashes into itself or the border. The amount of lives will increase by 1 when the worm grows and eats an apple. The game will end when the amount of lives reaches 0. The game will restart when the player presses the button after the game ends.
//  The lives icon will be represented by a heart bitmap. The amount of lives will be represented by the number of heart bitmaps displayed on the screen.
//  Everytime when someone reenter the game the heart wil be restore to 3 heart, the game will end if the game has 0 heart left

worm_game_lives_icon_t game_lives = {0};

void lives_init(void)
{
	game_lives.x = 0;
	game_lives.y = 0;
	lives_reset();
}

void lives_reset(void)
{
	game_lives.amount_lives = START_LIVES;
}

void lives_inc(void)
{
	game_lives.amount_lives++;
}

void lives_dec(void)
{
	if (game_lives.amount_lives > MIN_LIVES)
	{
		game_lives.amount_lives--;
	}
}

uint32_t lives_get(void)
{
	return game_lives.amount_lives;
}

void game_lives_handler(ak_msg_t *msg)
{
	switch (msg->sig)
	{
	case AC_LIVES_INIT:
		lives_init();
		break;

	case AC_LIVES_RESET:
		lives_reset();
		break;

	case AC_LIVES_INC:
		lives_inc();
		break;

	case AC_LIVES_DEC:
		lives_dec();
		break;

	default:
		break;
	}
}