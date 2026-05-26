#include "worm_game_eating.h"

workm_game_eating_effect_t game_eating = {0};

// Initialize the eating effect state
void eating_effect_init(void)
{
	game_eating.visible = 0;
	game_eating.x = 0;
	game_eating.y = 0;
	game_eating.radius = 0;
	game_eating.eating_image = 0;
}

// Start the eating effect at the specified coordinates
void eating_effect_start(uint32_t x, uint32_t y)
{
	game_eating.x = x;
	game_eating.y = y;
	game_eating.radius = EATING_EFFECT_MAX_RADIUS;
	game_eating.visible = 1;
	game_eating.eating_image = 0;
}

// Update the eating effect state on each tick
void eating_effect_tick(void)
{
	if (!game_eating.visible)
	{
		return;
	}

	if (game_eating.radius > 0)
	{
		game_eating.radius--;
	}

	if (game_eating.radius == 0)
	{
		game_eating.visible = 0;
	}
}

// Stop the eating effect and reset its state
void eating_effect_stop(void)
{
	game_eating.visible = 0;
	game_eating.radius = 0;
}

// Handle messages related to the eating effect
void game_eating_handler(ak_msg_t *msg)
{
	switch (msg->sig)
	{
	case AC_EATING_INIT:
		eating_effect_init();
		break;

	case AC_EATING_START:
		eating_effect_start(msg->if_sig, msg->ref_count);
		break;

	case AC_EATING_TICK:
		eating_effect_tick();
		if (game_eating.visible)
		{
			view_render.fillCircle((int16_t)game_eating.x,
								   (int16_t)game_eating.y,
								   (int16_t)game_eating.radius,
								   WHITE);
		}
		break;

	default:
		break;
	}
}