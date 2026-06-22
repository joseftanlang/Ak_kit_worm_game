#include "scr_charts.h"



typedef struct
{
	int16_t x;
	uint8_t y;
	uint8_t speed;
} chart_star_t;

static void view_scr_charts();
static void chart_screen();
static void chart_worm_tick();
static void chart_draw_worm();
static void chart_draw_stars();
static const char *chart_rank_name(uint8_t idx);
static const char *chart_rank_short(uint8_t idx);
static void chart_format_score(uint32_t score, char *out, uint8_t out_len);

// Main message handler for the charts screen, handling entry, exit, animation ticks, and mode button presses
void scr_charts_handle(ak_msg_t *msg);

view_dynamic_t dyn_view_charts = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_charts};

view_screen_t scr_charts = {
	&dyn_view_charts,
	ITEM_NULL,
	ITEM_NULL,

	.focus_item = 0,
};

// Chart animation state variables
static int16_t chart_worm_x = CHART_WIDTH;
static uint8_t chart_anim_frame = 0;
static uint8_t chart_rank_index = 0;
static chart_star_t chart_stars[CHART_STAR_COUNT] = {
	{8, 8, 1},
	{22, 15, 2},
	{33, 4, 1},
	{48, 12, 2},
	{61, 18, 1},
	{77, 10, 2},
	{89, 6, 1},
	{101, 14, 2},
	{114, 3, 1},
	{126, 16, 2},
};

// Draw the entire charts screen, including background, stars, worm animation, and score bubbles
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

// Return a short label for the rank index, used in the score bubble
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

// Format the score for display, returning "-" if the score is zero, or the numeric value otherwise
static void chart_format_score(uint32_t score, char *out, uint8_t out_len)
{
	if (score == 0)
	{
		snprintf(out, out_len, "-");
	}
	else
	{
		snprintf(out, out_len, "%lu", (unsigned long)score);
	}
}

// Update the worm's position and animate the stars for the background, creating a dynamic effect on the charts screen
static void chart_worm_tick()
{
	// Calculate the total width of the worm's tail to determine when it has fully exited the screen
	int16_t tail_width = (CHART_WORM_SEGMENTS - 1) * CHART_WORM_SEGMENT_GAP + (CHART_WORM_HEAD_RADIUS * 2);

	// Move the worm to the left by a fixed step size, and advance the animation frame counter
	chart_worm_x -= CHART_WORM_STEP_PX;
	chart_anim_frame++;

	// If the worm has completely moved off the left edge of the screen, reset its position to the right and update the rank index
	if (chart_worm_x < -tail_width)
	{
		chart_worm_x = CHART_WIDTH;
		chart_rank_index = (chart_rank_index + 1) % 3;
	}

	// Animate the stars in the background to create a sense of motion, with some stars blinking for visual interest
	for (uint8_t i = 0; i < CHART_STAR_COUNT; i++)
	{
		if (chart_stars[i].x <= chart_stars[i].speed)
		{
			chart_stars[i].x = CHART_WIDTH - 1;
			// Use a combination of the star index and animation frame to create a varied vertical movement pattern for the stars and 20 pixel vertical range
			chart_stars[i].y = (uint8_t)((3 + (i * 5) + chart_anim_frame) % 20);
		}
		else
		{
			// Move the star to the left by its speed, creating a parallax effect where faster stars appear closer and slower stars appear farther away
			chart_stars[i].x -= chart_stars[i].speed;
		}
	}
}

// Update the worm's position and animate the stars for the background, creating a dynamic effect on the charts screen
static void chart_draw_stars()
{
	for (uint8_t i = 0; i < CHART_STAR_COUNT; i++)
	{
		uint8_t draw = (uint8_t)((chart_anim_frame + i) & 0x01);
		if (draw == 0 || chart_stars[i].speed > 1)
		{
			view_render.drawPixel(chart_stars[i].x, chart_stars[i].y, WHITE);
		}
	}
}

// Draw the worm on the screen, with its head and segments, and add a simple animation by
// alternating the vertical position of the segments based on the animation frame counter
static void chart_draw_worm()
{
	// Calculate the base vertical position for the worm's head and segments, creating a simple up-and-down animation effect
	int16_t y_base = CHART_WORM_Y + ((chart_anim_frame >> 1) & 0x01);

	// Draw the worm's segments, with a gap between each segment,
	// and alternate their vertical position to create a wiggling animation effect
	for (uint8_t i = 1; i < CHART_WORM_SEGMENTS; i++)
	{
		int16_t segment_x = chart_worm_x + (i * CHART_WORM_SEGMENT_GAP);
		int16_t segment_y = y_base + (((i + chart_anim_frame) & 0x01) ? 1 : -1);
		view_render.fillCircle(segment_x, segment_y, 2, WHITE);
	}

	// Draw the worm's head as a larger circle, and add a simple animation by alternating the vertical position of the head based on the animation frame counter
	view_render.fillCircle(chart_worm_x, y_base, CHART_WORM_HEAD_RADIUS, WHITE);
	view_render.drawPixel(chart_worm_x - 1, y_base + 1, 0);
	view_render.drawPixel(chart_worm_x - 1, y_base - 1, 0);
}

// Draw the entire charts screen, including background, stars, worm animation, and score bubbles
static void chart_screen()
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

	chart_format_score(top1, score_1, sizeof(score_1));
	chart_format_score(top2, score_2, sizeof(score_2));
	chart_format_score(top3, score_3, sizeof(score_3));

	snprintf(slot_1, sizeof(slot_1), "1:%s", score_1);
	snprintf(slot_2, sizeof(slot_2), "2:%s", score_2);
	snprintf(slot_3, sizeof(slot_3), "3:%s", score_3);

	if (current == 0)
	{
		snprintf(bubble_text, sizeof(bubble_text), "%s -", chart_rank_short(chart_rank_index));
	}
	else
	{
		snprintf(bubble_text, sizeof(bubble_text), "%s %lu", chart_rank_short(chart_rank_index), (unsigned long)current);
	}

	bubble_len = (uint8_t)strlen(bubble_text); // Calculate the width of the score bubble based on the length of the text, with a fixed width per character and some padding
	bubble_w = (int16_t)(bubble_len * 6 + 4);  // 6 pixels per character + 4 pixels padding
	bubble_x = chart_worm_x - (bubble_w / 2);  // Center the bubble horizontally on the worm's head

	if (bubble_x < 2)
	{
		bubble_x = 2;
	}
	if ((bubble_x + bubble_w) > (CHART_WIDTH - 2))
	{
		bubble_x = CHART_WIDTH - bubble_w - 2;
	}

	highlight_x = rank_slot_x[chart_rank_index];

	view_render.clear();
	view_render.drawRect(0, 0, CHART_WIDTH, CHART_HEIGHT, WHITE);
	view_render.drawFastHLine(1, 34, CHART_WIDTH - 2, WHITE);

	view_render.setTextColor(WHITE);
	view_render.setTextSize(1);
	view_render.setCursor(18, 2);
	view_render.print("WORM LEADER RUN");

	chart_draw_stars();
	chart_draw_worm();
	view_render.drawRect(bubble_x, 10, bubble_w, 10, WHITE);
	view_render.setCursor((uint8_t)(bubble_x + 2), 12);
	view_render.print(bubble_text);

	view_render.drawRect(highlight_x, 41, 38, 11, WHITE);
	view_render.setCursor(10, 44);
	view_render.print(slot_1);
	view_render.setCursor(51, 44);
	view_render.print(slot_2);
	view_render.setCursor(92, 44);
	view_render.print(slot_3);

	view_render.setCursor(8, 56);
	view_render.print(chart_rank_name(chart_rank_index));
	view_render.setCursor(55, 56);
	view_render.print("MODE:BACK");
}

static void view_scr_charts()
{
	chart_screen();
}

// Main message handler for the charts screen, handling entry, exit, animation ticks, and mode button presses
void scr_charts_handle(ak_msg_t *msg)
{
	switch (msg->sig)
	{
	case SCREEN_ENTRY:
		chart_worm_x = CHART_WIDTH;
		chart_anim_frame = 0;
		chart_rank_index = 0;
		g_controller_mode = 1;
		timer_set(AC_TASK_DISPLAY_ID, AC_CHARTS_ANIM_TICK, CHART_ANIM_INTERVAL_MS, TIMER_PERIODIC);
		view_render_screen(&scr_charts);
		break;

	case AC_CHARTS_ANIM_TICK:
		chart_worm_tick();
		view_render_screen(&scr_charts);
		break;

	case SCREEN_EXIT:
		timer_remove_attr(AC_TASK_DISPLAY_ID, AC_CHARTS_ANIM_TICK);
		break;

	case CHART_MODE_SIG:
		timer_remove_attr(AC_TASK_DISPLAY_ID, AC_CHARTS_ANIM_TICK);
		SCREEN_TRAN(scr_worm_menu_game_handle, &scr_menu_game);
		break;

	default:
		break;
	}
}