#include "worm_game_eating.h"

worm_game_eating_effect_t worm_eating = {0};

// Initialize the eating effect state
void eating_effect_init(void)
{
	worm_eating.visible = 0;
	worm_eating.x = 0;
	worm_eating.y = 0;
	worm_eating.radius = 0;
	worm_eating.eating_image = 0;
}

// Start the eating effect at the specified coordinates
void eating_effect_start(uint32_t x, uint32_t y)
{
	worm_eating.x = x;
	worm_eating.y = y;
	worm_eating.radius = EATING_EFFECT_MAX_RADIUS;
	worm_eating.visible = 1;
	worm_eating.eating_image = 0;
}

// Update the eating effect state on each tick
void eating_effect_tick(void)
{
	if (!worm_eating.visible)
	{
		return;
	}

	if (worm_eating.radius > 0)
	{
		worm_eating.radius--;
	}

	if (worm_eating.radius == 0)
	{
		worm_eating.visible = 0;
	}
}

// Stop the eating effect and reset its state
void eating_effect_stop(void)
{
	worm_eating.visible = 0;
	worm_eating.radius = 0;
}

// Handle messages related to the eating effect
void worm_game_eating_handler(ak_msg_t *msg)
{
	switch (msg->sig)
	{
	case AC_WORM_EATING_INIT:
		eating_effect_init();
		break;

	case AC_WORM_EATING_START:
		eating_effect_start(msg->if_sig, msg->ref_count);
		break;

	case AC_WORM_EATING_TICK:
		eating_effect_tick();
		if (worm_eating.visible)
		{
			view_render.fillCircle((int16_t)worm_eating.x,
								   (int16_t)worm_eating.y,
								   (int16_t)worm_eating.radius,
								   WHITE);
		}
		break;

	default:
		break;
	}
}