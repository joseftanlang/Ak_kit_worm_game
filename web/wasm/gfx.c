#include "gfx.h"
#include "fb.h"
#include "font5x7.h"
#include <string.h>

static int16_t cursor_x = 0;
static int16_t cursor_y = 0;
static uint8_t text_size = 1;
static uint8_t text_color = 1;

void gfx_draw_pixel(int16_t x, int16_t y, uint8_t color)
{
	fb_draw_pixel(x, y, color);
}

void gfx_draw_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color)
{
	gfx_draw_fast_hline(x, y, w, color);
	gfx_draw_fast_hline(x, y + h - 1, w, color);
	for (int16_t i = y; i < y + h; i++)
	{
		gfx_draw_pixel(x, i, color);
		gfx_draw_pixel(x + w - 1, i, color);
	}
}

void gfx_draw_fast_hline(int16_t x, int16_t y, int16_t w, uint8_t color)
{
	for (int16_t i = 0; i < w; i++)
	{
		gfx_draw_pixel(x + i, y, color);
	}
}

void gfx_fill_circle(int16_t x0, int16_t y0, int16_t r, uint8_t color)
{
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	gfx_draw_pixel(x0, y0 + r, color);
	gfx_draw_pixel(x0, y0 - r, color);
	gfx_draw_pixel(x0 + r, y0, color);
	gfx_draw_pixel(x0 - r, y0, color);

	while (x < y)
	{
		if (f >= 0)
		{
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;

		gfx_draw_pixel(x0 + x, y0 + y, color);
		gfx_draw_pixel(x0 - x, y0 + y, color);
		gfx_draw_pixel(x0 + x, y0 - y, color);
		gfx_draw_pixel(x0 - x, y0 - y, color);
		gfx_draw_pixel(x0 + y, y0 + x, color);
		gfx_draw_pixel(x0 - y, y0 + x, color);
		gfx_draw_pixel(x0 + y, y0 - x, color);
		gfx_draw_pixel(x0 - y, y0 - x, color);
	}
}

void gfx_fill_round_rect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint8_t color)
{
	if (r > w / 2)
	{
		r = w / 2;
	}
	if (r > h / 2)
	{
		r = h / 2;
	}

	for (int16_t i = x + r; i < x + w - r; i++)
	{
		for (int16_t j = y; j < y + h; j++)
		{
			gfx_draw_pixel(i, j, color);
		}
	}
	for (int16_t i = y + r; i < y + h - r; i++)
	{
		for (int16_t j = x; j < x + w; j++)
		{
			gfx_draw_pixel(j, i, color);
		}
	}

	gfx_fill_circle(x + r, y + r, r, color);
	gfx_fill_circle(x + w - r - 1, y + r, r, color);
	gfx_fill_circle(x + r, y + h - r - 1, r, color);
	gfx_fill_circle(x + w - r - 1, y + h - r - 1, r, color);
}

void gfx_draw_bitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint8_t color)
{
	for (int16_t j = 0; j < h; j++)
	{
		uint8_t line = bitmap[j];
		for (int16_t i = 0; i < w; i++)
		{
			if (line & (1 << (w - 1 - i)))
			{
				gfx_draw_pixel(x + i, y + j, color);
			}
		}
	}
}

void gfx_set_cursor(int16_t x, int16_t y)
{
	cursor_x = x;
	cursor_y = y;
}

void gfx_set_text_size(uint8_t size)
{
	if (size < 1)
	{
		size = 1;
	}
	text_size = size;
}

void gfx_set_text_color(uint8_t color)
{
	text_color = color;
}

static void gfx_draw_char(int16_t x, int16_t y, char c, uint8_t color, uint8_t size)
{
	if (c < ' ' || c > 127)
	{
		return;
	}

	const unsigned char *glyph = &font[(c - ' ') * 5];

	for (int8_t i = 0; i < 5; i++)
	{
		uint8_t line = glyph[i];
		for (int8_t j = 0; j < 8; j++)
		{
			if (line & 0x1)
			{
				if (size == 1)
				{
					gfx_draw_pixel(x + i, y + j, color);
				}
				else
				{
					for (uint8_t sy = 0; sy < size; sy++)
					{
						for (uint8_t sx = 0; sx < size; sx++)
						{
							gfx_draw_pixel(x + i * size + sx, y + j * size + sy, color);
						}
					}
				}
			}
			line >>= 1;
		}
	}
}

void gfx_print(const char *str)
{
	while (*str)
	{
		if (*str == '\n')
		{
			cursor_y += text_size * 8;
			cursor_x = 0;
		}
		else
		{
			gfx_draw_char(cursor_x, cursor_y, *str, text_color, text_size);
			cursor_x += text_size * 6;
		}
		str++;
	}
}

void gfx_print_int(int32_t value)
{
	char buf[12];
	int i = 0;
	uint32_t u;

	if (value < 0)
	{
		gfx_print("-");
		u = (uint32_t)(-(value + 1)) + 1;
	}
	else
	{
		u = (uint32_t)value;
	}

	if (u == 0)
	{
		gfx_print("0");
		return;
	}

	while (u > 0)
	{
		buf[i++] = (char)('0' + (u % 10));
		u /= 10;
	}

	while (i > 0)
	{
		char digit[2] = {buf[i - 1], 0};
		gfx_print(digit);
		i--;
	}
}
