#include "lives.h"

worm_lives_t game_lives;

void lives_init(void)
{
	lives_reset();
}

void lives_reset(void)
{
	game_lives.amount_lives = START_LIVES;
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
