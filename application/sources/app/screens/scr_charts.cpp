#include "scr_charts.h"

#include "worm_game_score.h"
#include <string.h>

#define CHART_WIDTH                 (128)
#define CHART_HEIGHT                (64)
#define CHART_ANIM_INTERVAL_MS      (70)
#define CHART_WORM_STEP_PX          (3)
#define CHART_WORM_SEGMENTS         (7)
#define CHART_WORM_SEGMENT_GAP      (5)
#define CHART_WORM_Y                (24)
#define CHART_WORM_HEAD_RADIUS      (3)
#define AC_CHARTS_ANIM_TICK         (AK_USER_DEFINE_SIG + 180)
#define CHART_MODE_SIG              (11)
#define CHART_STAR_COUNT            (10)

static void view_scr_charts();
static void chart_screen();
static void chart_worm_tick();
static void chart_draw_worm();
static void chart_draw_stars();
static const char* chart_rank_name(uint8_t idx);
static const char* chart_rank_short(uint8_t idx);
static void chart_format_score(uint32_t score, char* out, uint8_t out_len);

typedef struct {
	int16_t x;
	uint8_t y;
	uint8_t speed;
} chart_star_t;

view_dynamic_t dyn_view_charts = {
    {
        .item_type = ITEM_TYPE_DYNAMIC,
    },
    view_scr_charts
};

view_screen_t scr_charts = {
    &dyn_view_charts,
    ITEM_NULL,
    ITEM_NULL,

    .focus_item = 0,
};

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

static const char* chart_rank_name(uint8_t idx) {
	switch (idx) {
	case 0:
		return "FIRST";
	case 1:
		return "SECOND";
	case 2:
	default:
		return "THIRD";
	}
}

static const char* chart_rank_short(uint8_t idx) {
	switch (idx) {
	case 0:
		return "1ST";
	case 1:
		return "2ND";
	case 2:
	default:
		return "3RD";
	}
}

static void chart_format_score(uint32_t score, char* out, uint8_t out_len) {
	if (score == 0) {
		snprintf(out, out_len, "-");
	}
	else {
		snprintf(out, out_len, "%lu", (unsigned long)score);
	}
}

static void chart_worm_tick() {
	int16_t tail_width = (CHART_WORM_SEGMENTS - 1) * CHART_WORM_SEGMENT_GAP + (CHART_WORM_HEAD_RADIUS * 2);

	chart_worm_x -= CHART_WORM_STEP_PX;
	chart_anim_frame++;

	if (chart_worm_x < -tail_width) {
		chart_worm_x = CHART_WIDTH;
		chart_rank_index = (chart_rank_index + 1) % 3;
	}

	for (uint8_t i = 0; i < CHART_STAR_COUNT; i++) {
		if (chart_stars[i].x <= chart_stars[i].speed) {
			chart_stars[i].x = CHART_WIDTH - 1;
			chart_stars[i].y = (uint8_t)((3 + (i * 5) + chart_anim_frame) % 20);
		}
		else {
			chart_stars[i].x -= chart_stars[i].speed;
		}
	}
}

static void chart_draw_stars() {
	for (uint8_t i = 0; i < CHART_STAR_COUNT; i++) {
		uint8_t draw = (uint8_t)((chart_anim_frame + i) & 0x01);
		if (draw == 0 || chart_stars[i].speed > 1) {
			view_render.drawPixel(chart_stars[i].x, chart_stars[i].y, WHITE);
		}
	}
}

static void chart_draw_worm() {
	int16_t y_base = CHART_WORM_Y + ((chart_anim_frame >> 1) & 0x01);

	for (uint8_t i = 1; i < CHART_WORM_SEGMENTS; i++) {
		int16_t segment_x = chart_worm_x + (i * CHART_WORM_SEGMENT_GAP);
		int16_t segment_y = y_base + (((i + chart_anim_frame) & 0x01) ? 1 : -1);
		view_render.fillCircle(segment_x, segment_y, 2, WHITE);
	}

	view_render.fillCircle(chart_worm_x, y_base, CHART_WORM_HEAD_RADIUS, WHITE);
	view_render.drawPixel(chart_worm_x - 1, y_base + 1, 0);
	view_render.drawPixel(chart_worm_x - 1, y_base - 1, 0);
}

static void chart_screen() {
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

	if (current == 0) {
		snprintf(bubble_text, sizeof(bubble_text), "%s -", chart_rank_short(chart_rank_index));
	}
	else {
		snprintf(bubble_text, sizeof(bubble_text), "%s %lu", chart_rank_short(chart_rank_index), (unsigned long)current);
	}

	bubble_len = (uint8_t)strlen(bubble_text);
	bubble_w = (int16_t)(bubble_len * 6 + 4);
	bubble_x = chart_worm_x - (bubble_w / 2);
	if (bubble_x < 2) {
		bubble_x = 2;
	}
	if ((bubble_x + bubble_w) > (CHART_WIDTH - 2)) {
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

static void view_scr_charts() {
    chart_screen();
}

void scr_charts_handle(ak_msg_t* msg)
{
    switch (msg->sig) {
    case SCREEN_ENTRY:
        chart_worm_x = CHART_WIDTH;
        chart_anim_frame = 0;
        chart_rank_index = 0;
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
        SCREEN_BACK();
        break;

    default:
        break;
    }
}