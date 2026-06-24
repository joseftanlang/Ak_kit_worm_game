// If you want to see many cool animation do uncomments all the codes. and comment line 44 to 45

#include "scr_startup.h"

static void view_scr_startup();
static uint8_t startup_anim_index = 0;

// #define STARTUP_ANIM_FRAMES 20
view_dynamic_t dyn_view_startup = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_startup};

view_screen_t scr_startup = {
	&dyn_view_startup,
	ITEM_NULL,
	ITEM_NULL,

	.focus_item = 0,
};

void view_scr_startup()
{
#define AK_LOGO_AXIS_X (23)
#define AK_LOGO_TEXT (AK_LOGO_AXIS_X + 4)
	/* nicer startup: slide-in bitmap logo + floating apples */
	// const int LOGO_W = 119;
	// const int LOGO_H = 62;

	// int target_x = (SCR_W - LOGO_W) / 2; // typically 4
	// int start_x = -LOGO_W;
	// int delta = target_x - start_x; // positive
	// // Calculate the current frame of the animation based on the startup_anim_index, ensuring it does not exceed the total number of frames.
	// int frame = (startup_anim_index > STARTUP_ANIM_FRAMES - 1) ? (STARTUP_ANIM_FRAMES - 1) : startup_anim_index;
	// int logo_x = start_x + (frame * delta) / (STARTUP_ANIM_FRAMES - 1);
	// int logo_y = (SCR_H - LOGO_H) / 2;

	view_render.clear();
	view_render.setTextSize(1);
	view_render.setTextColor(WHITE);

	// Drawing the 2 penguins
	view_render.drawBitmap(0, 15, image_WarningDolphin_1_bits, 45, 42, WHITE);
	view_render.drawBitmap(80, 15, image_WarningDolphinFlip_bits, 45, 42, WHITE);

	// Title text
	view_render.setTextSize(2);
	view_render.setCursor(10, 0);
	view_render.print("WORM GAME");

	// /* floating apples along the bottom as particles */
	// const int NUM_APPLES = 5;
	// for (int i = 0; i < NUM_APPLES; i++)
	// {
	// 	int phase = (startup_anim_index + i * 3) % (STARTUP_ANIM_FRAMES * 2);
	// 	int ax = 8 + (i * 22) + ((phase < STARTUP_ANIM_FRAMES) ? (phase) : (STARTUP_ANIM_FRAMES * 2 - phase));
	// 	int ay = SCR_H - 12 - ((phase % 6));
	// 	view_render.drawBitmap(ax, ay, bitmap_apple, 10, 10, WHITE);
	// }

	// /* progress bar under logo */
	// const int BAR_X = 8;
	// const int BAR_Y = SCR_H - 8;
	// const int BAR_W = SCR_W - 16;
	// const int BAR_H = 5;
	// int filled = (frame * BAR_W) / (STARTUP_ANIM_FRAMES - 1);
	// view_render.drawRect(BAR_X, BAR_Y, BAR_W, BAR_H, WHITE);
	// if (filled > 0)
	// {
	// 	view_render.fillRect(BAR_X + 1, BAR_Y + 1, filled - 2, BAR_H - 2, WHITE);
	// }
}

void scr_startup_handle(ak_msg_t *msg)
{
	switch (msg->sig)
	{
	case SCREEN_ENTRY:
	{
		APP_DBG_SIG("AC_DISPLAY_INITIAL\n");
		view_render.initialize();
		view_render_display_on();
		view_render_screen(&scr_startup);
		// g_controller_mode = 1;
		/* start periodic animation ticks and a one-shot to finish */
		timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_STARTUP_ANIM_TICK, AC_DISPLAY_STARTUP_ANIM_TICK_INTERVAL, TIMER_PERIODIC);
		timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_LOGO, AC_DISPLAY_STARTUP_INTERVAL, TIMER_ONE_SHOT);
	}
	break;

	case AC_DISPLAY_BUTON_MODE_PRESSED:
	case AC_DISPLAY_BUTON_UP_PRESSED:
	case AC_DISPLAY_BUTON_DOWN_PRESSED:
	{
		APP_DBG_SIG("AC_DISPLAY_MENU_SCREEN\n");
		/* user pressed a button - cancel animation and go to menu */
		timer_remove_attr(AC_TASK_DISPLAY_ID, AC_DISPLAY_STARTUP_ANIM_TICK);
		SCREEN_TRAN(scr_worm_menu_game_handle, &scr_menu_game);
	}
	break;

	case AC_DISPLAY_STARTUP_ANIM_TICK:
	{
		startup_anim_index++;
		view_render_screen(&scr_startup);
	}
	break;

	case AC_DISPLAY_SHOW_LOGO:
	{
		APP_DBG_SIG("AC_DISPLAY_MENU_SCREEN\n");
		timer_remove_attr(AC_TASK_DISPLAY_ID, AC_DISPLAY_STARTUP_ANIM_TICK);
		SCREEN_TRAN(scr_worm_menu_game_handle, &scr_menu_game);
	}
	break;

	case AC_DISPLAY_SHOW_IDLE:
	{
		APP_DBG_SIG("AC_DISPLAY_SHOW_IDLE\n");
		SCREEN_TRAN(scr_idle_handle, &scr_idle);
	}
	break;

	default:
		break;
	}
}
