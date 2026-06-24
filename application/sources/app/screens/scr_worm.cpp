#include "scr_worm.h"
// This is the main game of the codes, where the worm will eat the apple until the worm is at max length (cover the entire the screen).
// The worm have 3 lives to win the game
// The apple will spawn
// Make sure that there is cool animation.

static void view_scr_worm();
static void view_scr_worm_overlay();

// The Worm game design and visual and sound states are managed through a combination of functions and data structures defined in this file, which interact with the rendering system and game logic to create an engaging gameplay experience. The worm's movement, growth, collision detection, and game state transitions are handled through these functions, while the rendering functions draw the worm, apples, score, and game over/win screens based on the current state of the game.
static void view_draw_worm();
static void worm_music_start(void);
static void worm_music_stop(void);

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
	if (worm_game_finished)
	{
		return;
	}

	worm_game_finished = 1;
	worm_game_won = won ? 1 : 0;
	worm_game_anim_tick = 0;
	worm_music_stop();
	timer_remove_attr(WORM_GAME_APPLE_ID, AC_APPLE_TICK);
	score_commit_current();
}

// worm_music_stop stops the background music for the worm game by removing the timer that loops the music and disabling the buzzer. It also sets the buzzer to silent mode if the buzzer is not enabled in the game settings.
static void worm_music_stop(void)
{
	timer_remove_attr(AC_TASK_DISPLAY_ID, WORM_MUSIC_LOOP_TICK_SIG);
	BUZZER_Disable();
	BUZZER_Sleep(scr_game_setting_is_buzzer_enabled() ? false : true);
}

// worm_music_start initializes and starts the background music for the worm game based on the user's settings. It retrieves the selected song and checks if the buzzer is enabled, then plays the sound and sets up a timer to loop the music at appropriate intervals.
static void worm_music_start(void)
{
	buzzer_sound_t song = scr_game_setting_get_song();
	uint32_t loop_interval_ms;

	timer_remove_attr(AC_TASK_DISPLAY_ID, WORM_MUSIC_LOOP_TICK_SIG);

	if (!scr_game_setting_is_buzzer_enabled())
	{
		BUZZER_Sleep(true);
		return;
	}

	BUZZER_Sleep(false);
	BUZZER_PlaySound(song);

	loop_interval_ms = 1000; // safe default loop delay
	timer_set(AC_TASK_DISPLAY_ID,
			  WORM_MUSIC_LOOP_TICK_SIG,
			  loop_interval_ms,
			  TIMER_PERIODIC);
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

	if (worm_game_finished)
	{
		view_scr_worm_overlay();
		return;
	}

	border_settings();

	for (uint8_t i = 0; i < MAX_APPLES; i++)
	{
		if (apples_no[i].is_active)
		{
			// draw apple image
			view_render.drawBitmap(
				(int16_t)apples_no[i].x,
				(int16_t)apples_no[i].y,
				bitmap_apple,
				APPLE_WIDTH,
				APPLE_HEIGHT,
				WHITE);

		}
	}

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

	view_render.setTextSize(1);
	view_render.setTextColor(WHITE);
	view_render.setCursor(4, 2);
	char buf_score[16];
	int sc = (int)score_get();
	snprintf(buf_score, sizeof(buf_score), "Score:%d", sc);
	view_render.print(buf_score);
}

// view_scr_worm_overlay draws the game over or win screen overlay when the worm game has finished.
// It displays a message indicating whether the player won or lost, shows the final score, and provides instructions for viewing the top scores.
// The overlay is drawn on top of the existing game screen, creating a clear and visually distinct end state for the game.
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
	// trail_length is the number of segments of the worm's trail that should be drawn, which is determined by the current length of the worm and the maximum allowed trail length. It ensures that only the valid segments of the worm's body are rendered on the screen.
	uint16_t trail_length = (worm_game.length > WORM_MAX_TRAIL) ? WORM_MAX_TRAIL : worm_game.length;

	// center_x and center_y represent the coordinates of the center of the worm's head, which is calculated based on the position of the head segment and the size of the worm. The eye_dx1, eye_dy1, eye_dx2, and eye_dy2 variables are used to determine the positions of the worm's eyes relative to its head, based on its current direction of movement. These variables are set in the switch statement that follows, which adjusts their values according to whether the worm is moving right, left, down, or up.
	int16_t center_x;
	int16_t center_y;

	// eye_dx1, eye_dy1, eye_dx2, and eye_dy2 are used to calculate the positions of the worm's eyes based on its current direction. They determine how far from the center of the head the eyes should be drawn in both the x and y directions, allowing the eyes to be positioned correctly for each movement direction (right, left, down, up).
	int16_t eye_dx1 = 0;
	int16_t eye_dy1 = 0;
	int16_t eye_dx2 = 0;
	int16_t eye_dy2 = 0;

	// The loop iterates through the segments of the worm's trail (starting from the second segment, since the head is drawn separately) and draws each segment as a filled rounded rectangle on the screen. The position of each segment is determined by the coordinates stored in the worm's trail array, and the size of each segment is based on the defined movement step. This loop effectively renders the body of the worm on the screen, with each segment following the previous one to create a continuous trail.
	for (uint16_t i = trail_length; i > 1; i--)
	{
		const worm_game_point_t *segment = &worm_game.trail[i - 1];
		view_render.fillRoundRect((int16_t)segment->x,
								  (int16_t)segment->y,
								  WORM_MOVE_STEP,
								  WORM_MOVE_STEP,
								  1,
								  WHITE);
		// worm body border
		// view_render.drawRect(
		// 	(int16_t)segment->x,
		// 	(int16_t)segment->y,
		// 	WORM_MOVE_STEP,
		// 	WORM_MOVE_STEP,
		// 	WHITE);
	}

	// Draw the head of the worm as a filled circle with a slightly larger radius to create a rounded appearance. The center of the head is calculated based on the position of the first segment in the trail (the head) and the movement step, which determines how far the center is from the top-left corner of the head segment. This creates a visually distinct head for the worm, differentiating it from the body segments.
	center_x = (int16_t)worm_game.trail[0].x + (WORM_MOVE_STEP / 2);
	center_y = (int16_t)worm_game.trail[0].y + (WORM_MOVE_STEP / 2);

	// The head is drawn as two filled circles to create a layered effect, with the outer circle slightly larger than the inner circle. This gives the head a more rounded and visually appealing appearance, making it stand out from the body segments. The eyes are then drawn as pixels on top of the head, positioned based on the current direction of movement to give the worm a sense of orientation.
	view_render.fillCircle(center_x, center_y, (WORM_MOVE_STEP / 2) + 1, WHITE);
	view_render.fillCircle(center_x, center_y, WORM_MOVE_STEP / 2, WHITE);
	// WORM HEAD BORDER
	// view_render.drawRect(
	// 	worm_game.trail[0].x,
	// 	worm_game.trail[0].y,
	// 	WORM_MOVE_STEP,
	// 	WORM_MOVE_STEP,
	// 	WHITE);

	// The switch statement sets the eye_dx1, eye_dy1, eye_dx2, and eye_dy2 variables based on the current direction of the worm. These variables determine the relative positions of the worm's eyes on its head, allowing them to be drawn in the correct location for each movement direction (right, left, down, up). This adds a visual cue to indicate the direction the worm is facing, enhancing the overall appearance of the game.
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

	view_render.drawPixel(center_x + eye_dx1, center_y + eye_dy1, 0);
	view_render.drawPixel(center_x + eye_dx2, center_y + eye_dy2, 0);
}

void worm_worm_gamer_handler(ak_msg_t *msg)
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
		if (worm_game_finished)
		{
			worm_game_anim_tick++;
			view_render_screen(&scr_worm);
			break;
		}

		task_post_pure_msg(WORM_GAME_WORM_ID, AC_WORM_TICK);
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
		timer_remove_attr(WORM_GAME_GAMER_ID, AC_WORM_TICK);
		worm_game_reset();
		score_init();
		worm_init();
		lives_init();
		apple_init();
		g_controller_mode = 1;
		worm_music_start();
		view_render_screen(&scr_worm);
		timer_set(WORM_GAME_GAMER_ID, AC_WORM_TICK, scr_game_setting_get_worm_tick_interval_ms(), TIMER_PERIODIC);
	}
	break;

	case WORM_MUSIC_LOOP_TICK_SIG:
	{
		if (!worm_game_finished && scr_game_setting_is_buzzer_enabled())
		{
			BUZZER_PlaySound(scr_game_setting_get_song());
		}
	}
	break;

	case SCREEN_EXIT:
	{
		timer_remove_attr(WORM_GAME_GAMER_ID, AC_WORM_TICK);
		worm_music_stop();
	}
	break;

	default:
		break;
	}

	if (msg->sig == 12)
	{ /* AC_DISPLAY_BUTON_UP_PRESSED */
		task_post_pure_msg(WORM_GAME_WORM_ID, AC_WORM_SET_DIR_UP);
	}
	else if (msg->sig == 13)
	{ /* AC_DISPLAY_BUTON_DOWN_PRESSED */
		task_post_pure_msg(WORM_GAME_WORM_ID, AC_WORM_SET_DIR_DOWN);
	}
	else if (msg->sig == 11)
	{ /* AC_DISPLAY_BUTON_MODE_PRESSED */
		timer_remove_attr(WORM_GAME_GAMER_ID, AC_WORM_TICK);
		worm_music_stop();
		if (worm_game_finished)
		{
			SCREEN_TRAN(scr_charts_handle, &scr_charts);
		}
		else
		{
			SCREEN_TRAN(scr_worm_menu_game_handle, &scr_menu_game);
		}
	}
}