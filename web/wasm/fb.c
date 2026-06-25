#include "fb.h"

static uint8_t framebuffer[FB_SIZE];

void fb_clear(void)
{
	for (int i = 0; i < FB_SIZE; i++)
	{
		framebuffer[i] = 0;
	}
}

void fb_draw_pixel(int16_t x, int16_t y, uint8_t color)
{
	if (x < 0 || x >= SCR_WIDTH || y < 0 || y >= SCR_HEIGHT)
	{
		return;
	}

	uint8_t row = (uint8_t)(y / 8);
	uint8_t offset = (uint8_t)(y % 8);
	int16_t index = (int16_t)(row * SCR_WIDTH + x);
	uint8_t val = (uint8_t)(1 << offset);

	if (color)
	{
		framebuffer[index] |= val;
	}
	else
	{
		framebuffer[index] &= (uint8_t)(~val);
	}
}

const uint8_t *fb_get_buffer(void)
{
	return framebuffer;
}
