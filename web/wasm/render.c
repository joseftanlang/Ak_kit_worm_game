#include "render.h"
#include "apple.h"
#include "gfx.h"
#include "lives.h"
#include "score.h"
#include "settings.h"
#include "worm_game.h"

static const uint8_t bitmap_apple[] = {0x1C, 0x3E, 0x7F, 0x7F, 0x7F, 0x3E, 0x1C};

static void uint_to_str(uint32_t v, char *out, uint8_t out_len)
{
	uint8_t i = 0;
	char tmp[12];

	if (v == 0)
	{
		if (out_len > 1)
		{
			out[0] = '-';
			out[1] = 0;
		}
		return;
	}

	while (v > 0 && i < 11)
	{
		tmp[i++] = (char)('0' + (v % 10));
		v /= 10;
	}

	uint8_t j = 0;
	while (i > 0 && j + 1 < out_len)
	{
		out[j++] = tmp[i - 1];
		i--;
	}
	out[j] = 0;
}

static void format_score_slot(char *out, uint8_t out_len, uint8_t rank, uint32_t score)
{
	char num[12];
	uint8_t pos = 0;
	out[pos++] = (char)('0' + rank);
	out[pos++] = ':';
	if (score == 0)
	{
		out[pos++] = '-';
		out[pos] = 0;
	}
	else
	{
		uint_to_str(score, num, sizeof(num));
		uint8_t k = 0;
		while (num[k] && pos + 1 < out_len)
		{
			out[pos++] = num[k++];
		}
		out[pos] = 0;
	}
}

static void format_bubble(char *out, uint8_t out_len, const char *rank, uint32_t score)
{
	uint8_t pos = 0;
	while (rank[pos] && pos + 1 < out_len)
	{
		out[pos] = rank[pos];
		pos++;
	}
	if (score == 0)
	{
		if (pos + 3 < out_len)
		{
			out[pos++] = ' ';
			out[pos++] = '-';
			out[pos] = 0;
		}
	}
	else
	{
		char num[12];
		out[pos++] = ' ';
		uint_to_str(score, num, sizeof(num));
		uint8_t k = 0;
		while (num[k] && pos + 1 < out_len)
		{
			out[pos++] = num[k++];
		}
		out[pos] = 0;
	}
}

static void render_draw_worm(void)
{
	uint16_t trail_length = (worm_game.length > WORM_MAX_TRAIL) ? WORM_MAX_TRAIL : worm_game.length;
	int16_t center_x;
	int16_t center_y;
	int16_t eye_dx1 = 0;
	int16_t eye_dy1 = 0;
	int16_t eye_dx2 = 0;
	int16_t eye_dy2 = 0;

	for (uint16_t i = trail_length; i > 1; i--)
	{
		const worm_point_t *segment = &worm_game.trail[i - 1];
		gfx_fill_round_rect((int16_t)segment->x, (int16_t)segment->y,
							WORM_MOVE_STEP, WORM_MOVE_STEP, 1, 1);
	}

	center_x = (int16_t)worm_game.trail[0].x + (WORM_MOVE_STEP / 2);
	center_y = (int16_t)worm_game.trail[0].y + (WORM_MOVE_STEP / 2);

	gfx_fill_circle(center_x, center_y, (WORM_MOVE_STEP / 2) + 1, 1);
	gfx_fill_circle(center_x, center_y, WORM_MOVE_STEP / 2, 1);

	switch (worm_get_direction())
	{
	case WORM_DIR_RIGHT:
		eye_dx1 = 1;
		eye_dy1 = -1;
		eye_dx2 = 1;
		eye_dy2 = 1;
		break;
	case WORM_DIR_LEFT:
		eye_dx1 = -1;
		eye_dy1 = -1;
		eye_dx2 = -1;
		eye_dy2 = 1;
		break;
	case WORM_DIR_DOWN:
		eye_dx1 = -1;
		eye_dy1 = 1;
		eye_dx2 = 1;
		eye_dy2 = 1;
		break;
	case WORM_DIR_UP:
	default:
		eye_dx1 = -1;
		eye_dy1 = -1;
		eye_dx2 = 1;
		eye_dy2 = -1;
		break;
	}

	gfx_draw_pixel(center_x + eye_dx1, center_y + eye_dy1, 0);
	gfx_draw_pixel(center_x + eye_dx2, center_y + eye_dy2, 0);
}

void render_worm_screen(uint8_t finished, uint8_t won)
{
	fb_clear();

	if (finished)
	{
		const char *title = won ? "YOU WIN" : "GAME OVER";
		char score_buf[20];
		uint8_t pos = 0;

		score_buf[pos++] = 'S';
		score_buf[pos++] = 'C';
		score_buf[pos++] = 'O';
		score_buf[pos++] = 'R';
		score_buf[pos++] = 'E';
		score_buf[pos++] = ':';
		score_buf[pos++] = ' ';
		{
			char num[12];
			uint_to_str(score_get(), num, sizeof(num));
			uint8_t k = 0;
			while (num[k] && pos + 1 < sizeof(score_buf))
			{
				score_buf[pos++] = num[k++];
			}
		}
		score_buf[pos] = 0;

		gfx_draw_rect(0, 0, SCR_WIDTH, SCR_HEIGHT, 1);
		gfx_set_text_color(1);
		gfx_set_text_size(2);
		gfx_set_cursor(8, 16);
		gfx_print(title);
		gfx_set_text_size(1);
		gfx_set_cursor(33, 34);
		gfx_print(score_buf);
		gfx_set_cursor(10, 46);
		gfx_print("MODE TO VIEW TOP 3");
		return;
	}

	gfx_draw_rect(BORDER_MARGIN, BORDER_MARGIN,
				  SCR_WIDTH - (BORDER_MARGIN * 2),
				  SCR_HEIGHT - (BORDER_MARGIN * 2), 1);

	for (uint8_t i = 0; i < MAX_APPLES; i++)
	{
		if (apples[i].is_active)
		{
			gfx_draw_bitmap((int16_t)apples[i].x, (int16_t)apples[i].y,
							bitmap_apple, APPLE_WIDTH, APPLE_HEIGHT, 1);
		}
	}

	render_draw_worm();

	gfx_set_text_size(1);
	gfx_set_text_color(1);
	gfx_set_cursor(4, 2);
	gfx_print("Score:");
	gfx_print_int((int32_t)score_get());
}

void render_menu(int menu_index)
{
	static const char *menu_items[] = {"Worm", "Charts", "Settings"};
	static const int menu_count = 3;

	fb_clear();
	gfx_set_text_size(1);
	gfx_set_text_color(1);
	gfx_set_cursor(35, 3);
	gfx_print("WORM GAME");
	gfx_draw_fast_hline(0, 14, SCR_WIDTH, 1);

	for (int row = 0; row < 3; row++)
	{
		int i = row;
		if (i >= menu_count)
		{
			break;
		}
		int y = 20 + row * 10;
		if (i == menu_index)
		{
			gfx_set_cursor(8, y + 5);
			gfx_print("> ");
			gfx_print(menu_items[i]);
		}
		else
		{
			gfx_set_text_color(1);
			gfx_set_cursor(8, y + 5);
			gfx_print("  ");
			gfx_print(menu_items[i]);
		}
	}
}

void render_settings(int selected_item)
{
	fb_clear();
	gfx_set_text_color(1);
	gfx_set_text_size(1);
	gfx_set_cursor(30, 3);
	gfx_print("SETTINGS");

	static const char *labels[] = {"SPEED", "APPLE", "SONG", "BUZZER"};
	static const char *values[] = {
		settings_speed_label(),
		settings_apple_label(),
		settings_song_label(),
		settings_buzzer_label()};

	for (int i = 0; i < 4; i++)
	{
		int y = 18 + i * 10;
		if (i == selected_item)
		{
			gfx_set_cursor(8, y + 2);
			gfx_print("> ");
		}
		else
		{
			gfx_set_text_color(1);
			gfx_set_cursor(8, y + 2);
			gfx_print("  ");
		}
		gfx_print(labels[i]);
		gfx_set_cursor(82, y + 2);
		gfx_print(values[i]);
	}
}

typedef struct
{
	int16_t x;
	uint8_t y;
	uint8_t speed;
} chart_star_t;

static chart_star_t chart_stars[CHART_STAR_COUNT] = {
	{8, 8, 1}, {22, 15, 2}, {33, 4, 1}, {48, 12, 2}, {61, 18, 1},
	{77, 10, 2}, {89, 6, 1}, {101, 14, 2}, {114, 3, 1}, {126, 16, 2}};

static int16_t chart_worm_x = SCR_WIDTH;
static uint8_t chart_anim_frame = 0;
static uint8_t chart_rank_index = 0;

static const char *chart_rank_name(uint8_t idx)
{
	switch (idx)
	{
	case 0:
		return "FIRST";
	case 1:
		return "SECOND";
	case 2:
	default:
		return "THIRD";
	}
}

static const char *chart_rank_short(uint8_t idx)
{
	switch (idx)
	{
	case 0:
		return "1ST";
	case 1:
		return "2ND";
	case 2:
	default:
		return "3RD";
	}
}

static void chart_worm_tick(void)
{
	int16_t tail_width = (CHART_WORM_SEGMENTS - 1) * CHART_WORM_SEGMENT_GAP +
						 (CHART_WORM_HEAD_RADIUS * 2);

	chart_worm_x -= CHART_WORM_STEP_PX;
	chart_anim_frame++;

	if (chart_worm_x < -tail_width)
	{
		chart_worm_x = SCR_WIDTH;
		chart_rank_index = (chart_rank_index + 1) % 3;
	}

	for (uint8_t i = 0; i < CHART_STAR_COUNT; i++)
	{
		if (chart_stars[i].x <= chart_stars[i].speed)
		{
			chart_stars[i].x = SCR_WIDTH - 1;
			chart_stars[i].y = (uint8_t)((3 + (i * 5) + chart_anim_frame) % 20);
		}
		else
		{
			chart_stars[i].x -= chart_stars[i].speed;
		}
	}
}

static void chart_draw_stars(void)
{
	for (uint8_t i = 0; i < CHART_STAR_COUNT; i++)
	{
		uint8_t draw = (uint8_t)((chart_anim_frame + i) & 0x01);
		if (draw == 0 || chart_stars[i].speed > 1)
		{
			gfx_draw_pixel(chart_stars[i].x, chart_stars[i].y, 1);
		}
	}
}

static void chart_draw_worm(void)
{
	int16_t y_base = CHART_WORM_Y + ((chart_anim_frame >> 1) & 0x01);

	for (uint8_t i = 1; i < CHART_WORM_SEGMENTS; i++)
	{
		int16_t segment_x = chart_worm_x + (i * CHART_WORM_SEGMENT_GAP);
		int16_t segment_y = y_base + (((i + chart_anim_frame) & 0x01) ? 1 : -1);
		gfx_fill_circle(segment_x, segment_y, 2, 1);
	}

	gfx_fill_circle(chart_worm_x, y_base, CHART_WORM_HEAD_RADIUS, 1);
	gfx_draw_pixel(chart_worm_x - 1, y_base + 1, 0);
	gfx_draw_pixel(chart_worm_x - 1, y_base - 1, 0);
}

void render_charts_tick(void)
{
	chart_worm_tick();
}

void render_charts_reset(void)
{
	chart_worm_x = SCR_WIDTH;
	chart_anim_frame = 0;
	chart_rank_index = 0;
}

void render_charts_screen(void)
{
	uint32_t top1 = score_top_get(0);
	uint32_t top2 = score_top_get(1);
	uint32_t top3 = score_top_get(2);
	uint32_t current = score_top_get(chart_rank_index);
	char score_1[8];
	char score_2[8];
	char score_3[8];
	char slot_1[12];
	char slot_2[12];
	char slot_3[12];
	char bubble_text[20];
	uint8_t bubble_len;
	int16_t bubble_w;
	int16_t bubble_x;
	int16_t highlight_x;
	const int16_t rank_slot_x[3] = {4, 45, 86};

	if (top1 == 0)
	{
		score_1[0] = '-';
		score_1[1] = 0;
	}
	else
	{
		uint_to_str(top1, score_1, sizeof(score_1));
	}
	if (top2 == 0)
	{
		score_2[0] = '-';
		score_2[1] = 0;
	}
	else
	{
		uint_to_str(top2, score_2, sizeof(score_2));
	}
	if (top3 == 0)
	{
		score_3[0] = '-';
		score_3[1] = 0;
	}
	else
	{
		uint_to_str(top3, score_3, sizeof(score_3));
	}

	format_score_slot(slot_1, sizeof(slot_1), 1, top1);
	format_score_slot(slot_2, sizeof(slot_2), 2, top2);
	format_score_slot(slot_3, sizeof(slot_3), 3, top3);

	format_bubble(bubble_text, sizeof(bubble_text),
				  chart_rank_short(chart_rank_index), current);

	bubble_len = 0;
	while (bubble_text[bubble_len])
	{
		bubble_len++;
	}
	bubble_w = (int16_t)(bubble_len * 6 + 4);
	bubble_x = chart_worm_x - (bubble_w / 2);

	if (bubble_x < 2)
	{
		bubble_x = 2;
	}
	if ((bubble_x + bubble_w) > (SCR_WIDTH - 2))
	{
		bubble_x = SCR_WIDTH - bubble_w - 2;
	}

	highlight_x = rank_slot_x[chart_rank_index];

	fb_clear();
	gfx_draw_rect(0, 0, SCR_WIDTH, SCR_HEIGHT, 1);
	gfx_draw_fast_hline(1, 34, SCR_WIDTH - 2, 1);

	gfx_set_text_color(1);
	gfx_set_text_size(1);
	gfx_set_cursor(18, 2);
	gfx_print("WORM LEADER RUN");

	chart_draw_stars();
	chart_draw_worm();
	gfx_draw_rect(bubble_x, 10, bubble_w, 10, 1);
	gfx_set_cursor((uint8_t)(bubble_x + 2), 12);
	gfx_print(bubble_text);

	gfx_draw_rect(highlight_x, 41, 38, 11, 1);
	gfx_set_cursor(10, 44);
	gfx_print(slot_1);
	gfx_set_cursor(51, 44);
	gfx_print(slot_2);
	gfx_set_cursor(92, 44);
	gfx_print(slot_3);

	gfx_set_cursor(8, 56);
	gfx_print(chart_rank_name(chart_rank_index));
	gfx_set_cursor(70, 56);
	gfx_print("MODE:MENU");
}
