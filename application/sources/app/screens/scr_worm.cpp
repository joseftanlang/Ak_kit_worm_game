#include "scr_worm.h"
#include "scr_charts.h"
#include "scr_setting.h"
#include "app.h"
#include "app_dbg.h"

// This is the main game of the codes, where the worm will eat the apple until the worm is at max length (cover the entire the screen). 
// The worm have 3 lives to win the game
// The apple will spawn
// Make sure that there is cool animation.

static void view_scr_worm();

static void view_scr_worm_overlay();

static void view_draw_worm();

#define HEART_SIZE (7)
#define HEART_SPACING (1)
#define WORM_GAME_TEXT_X (20)
#define WORM_GAME_TEXT_Y (18)
#define WORM_GAME_HELP_Y (44)
#define WORM_GAME_SPARKLES (8)

static uint8_t worm_game_finished = 0;
static uint8_t worm_game_won = 0;
static uint8_t worm_game_anim_tick = 0;

uint8_t worm_game_is_finished(void)
{
	return worm_game_finished;
}

uint8_t worm_game_is_won(void)
{
	return worm_game_won;
}

void worm_game_reset(void)
{
	worm_game_finished = 0;
	worm_game_won = 0;
	worm_game_anim_tick = 0;
}

void worm_game_finish(uint8_t won)
{
	if (worm_game_finished) {
		return;
	}

	worm_game_finished = 1;
	worm_game_won = won ? 1 : 0;
	worm_game_anim_tick = 0;
	timer_remove_attr(GAME_APPLE_ID, AC_APPLE_TICK);
	score_commit_current();
}

view_dynamic_t dyn_view_item_worm = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_worm};

view_screen_t scr_worm = {
	&dyn_view_item_worm,
	ITEM_NULL,
	ITEM_NULL,

	.focus_item = 0,
};

void view_scr_worm()
{
	view_render.clear();

	if (worm_game_finished) {
		view_scr_worm_overlay();
		return;
	}

	/* draw border (and set game_border) */
	border_settings();

	/* draw apples using bitmap_apple */
	for (uint8_t i = 0; i < MAX_APPLES; i++)
	{
		if (apples_no[i].is_active)
		{
			view_render.drawBitmap((int16_t)apples_no[i].x,
								   (int16_t)apples_no[i].y,
								   bitmap_apple,
								   APPLE_WIDTH,
								   APPLE_HEIGHT,
								   WHITE);
		}
	}

	/* draw worm */
	view_draw_worm();

	// /* draw lives at top right */ --> use only if needed to.
	// for (uint32_t i = 0; i < lives_get(); i++)
	// {
	// 	int16_t heart_x = (int16_t)(SCR_WIDTH - HEART_SIZE - (i * (HEART_SIZE + HEART_SPACING)));
	// 	view_render.drawBitmap(heart_x,
	// 						   0,
	// 						   bitmap_heart_w,
	// 						   HEART_SIZE,
	// 						   HEART_SIZE,
	// 						   WHITE);
	// }

	/* draw score at top-left */
	view_render.setTextSize(1);
	view_render.setTextColor(WHITE);
	view_render.setCursor(4, 2);
	char buf_score[16];
	int sc = (int)score_get();
	snprintf(buf_score, sizeof(buf_score), "Score:%d", sc);
	view_render.print(buf_score);

}

static void view_scr_worm_overlay()
{
	const char *title = worm_game_won ? "YOU WIN" : "GAME OVER";
	const char *subtitle = "MODE TO VIEW TOP 3";
	char score_buf[20];

	snprintf(score_buf, sizeof(score_buf), "SCORE: %d", (int)score_get());

	view_render.drawRect(0, 0, SCR_WIDTH, SCR_HEIGHT, WHITE);

	view_render.setTextColor(WHITE);
	view_render.setTextSize(2);
	view_render.setCursor(8, 16);
	view_render.print(title);

	view_render.setTextSize(1);
	view_render.setCursor(33, 34);
	view_render.print(score_buf);
	view_render.setCursor(10, 46);
	view_render.print(subtitle);
}

static void view_draw_worm()
{
	uint16_t trail_length = (game_worm.length > WORM_MAX_TRAIL) ? WORM_MAX_TRAIL : game_worm.length;
	int16_t center_x;
	int16_t center_y;
	int16_t eye_dx1 = 0;
	int16_t eye_dy1 = 0;
	int16_t eye_dx2 = 0;
	int16_t eye_dy2 = 0;

	for (uint16_t i = trail_length; i > 1; i--) {
		const worm_game_point_t *segment = &game_worm.trail[i - 1];
		view_render.fillRoundRect((int16_t)segment->x,
								  (int16_t)segment->y,
								  WORM_MOVE_STEP,
								  WORM_MOVE_STEP,
								  1,
								  WHITE);
	}

	center_x = (int16_t)game_worm.trail[0].x + (WORM_MOVE_STEP / 2);
	center_y = (int16_t)game_worm.trail[0].y + (WORM_MOVE_STEP / 2);

	view_render.fillCircle(center_x, center_y, (WORM_MOVE_STEP / 2) + 1, WHITE);
	view_render.fillCircle(center_x, center_y, WORM_MOVE_STEP / 2, WHITE);

	switch (worm_get_direction()) {
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

	view_render.drawPixel(center_x + eye_dx1, center_y + eye_dy1, 0);
	view_render.drawPixel(center_x + eye_dx2, center_y + eye_dy2, 0);
}

void game_gamer_handler(ak_msg_t *msg)
{
	switch (msg->sig)
	{
	case AC_WORM_INIT:
		worm_game_reset();
		score_init();
		worm_init();
		lives_init();
		apple_init();
		break;

	case AC_WORM_TICK:
		if (worm_game_finished) {
			worm_game_anim_tick++;
			view_render_screen(&scr_worm);
			break;
		}

		task_post_pure_msg(GAME_WORM_ID, AC_WORM_TICK);
		view_render_screen(&scr_worm);
	break;

	default:
		break;
	}
}

void scr_worm_handle(ak_msg_t *msg)
{
	switch (msg->sig)
	{
	case SCREEN_ENTRY:
	{
		timer_remove_attr(GAME_GAMER_ID, AC_WORM_TICK);
		worm_game_reset();
		score_init();
		/* keep buzzer aligned with the settings screen */
		BUZZER_Silent(scr_game_setting_is_buzzer_enabled() ? false : true);
		worm_init();
		lives_init();
		apple_init();
		view_render_screen(&scr_worm);
		timer_set(GAME_GAMER_ID, AC_WORM_TICK, scr_game_setting_get_worm_tick_interval_ms(), TIMER_PERIODIC);
	}
	break;

	default:
		break;
	}

	if (msg->sig == 12) { /* AC_DISPLAY_BUTON_UP_PRESSED */
		task_post_pure_msg(GAME_WORM_ID, AC_WORM_SET_DIR_UP);
	}
    else if (msg->sig == 13) { /* AC_DISPLAY_BUTON_DOWN_PRESSED */
		task_post_pure_msg(GAME_WORM_ID, AC_WORM_SET_DIR_DOWN);
	}
	else if (msg->sig == 11) { /* AC_DISPLAY_BUTON_MODE_PRESSED */
		timer_remove_attr(GAME_GAMER_ID, AC_WORM_TICK);
		if (worm_game_finished) {
			SCREEN_TRAN(scr_charts_handle, &scr_charts);
		}
		else {
			/* keep buzzer state aligned with the settings screen */
			BUZZER_Silent(scr_game_setting_is_buzzer_enabled() ? false : true);
			SCREEN_BACK();
		}
	}
}