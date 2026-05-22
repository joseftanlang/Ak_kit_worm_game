#include "scr_setting.h"

static uint8_t selected_item = 0; // 0 = speed, 1 = buzzer
static uint8_t setting_anim_tick = 0;
static uint8_t setting_worm_fast = 0; /* default: SLOW */
static uint8_t setting_buzzer_enabled = 0; /* default: BUZZER OFF */

#define SETTING_ANIM_INTERVAL_MS (85)
#define SETTING_ANIM_TICK_SIG (AK_USER_DEFINE_SIG + 183)
#define SETTING_WORM_TICK_FAST_MS (120)
#define SETTING_WORM_TICK_SLOW_MS (220)

typedef struct
{
    int16_t x;
    uint8_t y;
    uint8_t speed;
} setting_star_t;

static setting_star_t setting_stars[] = {
    {12, 8, 1},
    {28, 19, 2},
    {46, 6, 1},
    {67, 15, 2},
    {92, 10, 1},
    {111, 22, 2},
};
static const uint8_t setting_star_count = sizeof(setting_stars) / sizeof(setting_stars[0]);

static void view_scr_game_setting();
static void setting_tick();
static void setting_draw_background();
static void setting_draw_title();
static void setting_draw_row(int index, int y, const char *label, const char *value, const char *hint);
static void setting_toggle_selected_item();
static const char *setting_get_speed_value();
static const char *setting_get_buzzer_value();

view_dynamic_t dyn_view_item_game_setting = {
    {.item_type = ITEM_TYPE_DYNAMIC},
    view_scr_game_setting};

view_screen_t scr_game_setting = {
    &dyn_view_item_game_setting,
    ITEM_NULL,
    ITEM_NULL,
    .focus_item = 0,
};

uint16_t scr_game_setting_get_worm_tick_interval_ms(void)
{
    return setting_worm_fast ? SETTING_WORM_TICK_FAST_MS : SETTING_WORM_TICK_SLOW_MS;
}

uint8_t scr_game_setting_is_buzzer_enabled(void)
{
    return setting_buzzer_enabled;
}

static const char *setting_get_speed_value()
{
    return setting_worm_fast ? "FAST" : "SLOW";
}

static const char *setting_get_buzzer_value()
{
    return setting_buzzer_enabled ? "OFF" : "ON";
}

static void setting_toggle_selected_item()
{
    if (selected_item == 0)
    {
        setting_worm_fast = setting_worm_fast ? 0 : 1;
        return;
    }

    setting_buzzer_enabled = setting_buzzer_enabled ? 0 : 1;
    BUZZER_Silent(setting_buzzer_enabled ? false : true);
}

static void setting_tick()
{
    setting_anim_tick++;

    for (uint8_t i = 0; i < setting_star_count; i++)
    {
        if (setting_stars[i].x <= setting_stars[i].speed)
        {
            setting_stars[i].x = 127;
            setting_stars[i].y = (uint8_t)((setting_anim_tick + (i * 13)) % 64);
        }
        else
        {
            setting_stars[i].x -= setting_stars[i].speed;
        }
    }
}

static void setting_draw_background()
{
    for (uint8_t i = 0; i < setting_star_count; i++)
    {
        if (((setting_anim_tick + i) & 0x01) == 0 || setting_stars[i].speed > 1)
        {
            view_render.drawPixel(setting_stars[i].x, setting_stars[i].y, WHITE);
        }
    }

    view_render.drawRect(0, 0, 128, 64, WHITE);
    view_render.drawFastHLine(0, 14, 128, WHITE);
}

static void setting_draw_title()
{
    view_render.setTextColor(WHITE);
    view_render.setTextSize(1);
    view_render.setCursor(30, 3);
    view_render.print("SETTINGS");
}

static void setting_draw_row(int index, int y, const char *label, const char *value, const char *hint)
{
    if (index == selected_item)
    {
        view_render.fillRoundRect(6, y, 116, 16, 3, WHITE);
        view_render.setTextColor(BLACK);
        view_render.setCursor(12, y + 4);
        view_render.print("> ");
        view_render.print(label);
        view_render.setCursor(78, y + 4);
        view_render.print(value);
        view_render.setTextColor(WHITE);
        view_render.setCursor(12, y + 12);
        view_render.print(hint);
    }
    else
    {
        view_render.drawRoundRect(6, y, 116, 16, 3, WHITE);
        view_render.setTextColor(WHITE);
        view_render.setCursor(12, y + 4);
        view_render.print("  ");
        view_render.print(label);
        view_render.setCursor(78, y + 4);
        view_render.print(value);
        view_render.setCursor(12, y + 12);
        view_render.print(hint);
    }
}

void view_scr_game_setting()
{
    view_render.clear();

    setting_draw_background();
    setting_draw_title();

    setting_draw_row(0, 22, "SPEED", setting_get_speed_value(), "");
    setting_draw_row(1, 42, "BUZZER", setting_get_buzzer_value(), "");
}

void scr_game_setting_handle(ak_msg_t *msg)
{
    switch (msg->sig)
    {
    case SCREEN_ENTRY:
        selected_item = 0;
        setting_anim_tick = 0;
        timer_set(AC_TASK_DISPLAY_ID, SETTING_ANIM_TICK_SIG, SETTING_ANIM_INTERVAL_MS, TIMER_PERIODIC);
        view_scr_game_setting();
        break;

    case SCREEN_EXIT:
        timer_remove_attr(AC_TASK_DISPLAY_ID, SETTING_ANIM_TICK_SIG);
        break;

    case SETTING_ANIM_TICK_SIG:
        setting_tick();
        view_scr_game_setting();
        break;

    case 12: /* AC_DISPLAY_BUTON_UP_PRESSED */
    {
        if(selected_item > 0)
        {
                selected_item--;
        }
        else
        {
            selected_item = 1;
        }
        view_scr_game_setting();
        BUZZER_PlaySound(BUZZER_SOUND_CLICK);
    }
    break;

    case 13: /* AC_DISPLAY_BUTON_DOWN_PRESSED */
    {
        selected_item = (selected_item + 1) % 2;
        view_scr_game_setting();
        BUZZER_PlaySound(BUZZER_SOUND_CLICK);
    }
    break;

    case 11: /* AC_DISPLAY_BUTON_MODE_PRESSED */
    {
        BUZZER_PlaySound(BUZZER_SOUND_CLICK);
        setting_toggle_selected_item();
        view_scr_game_setting();
    }
    break;

    case 19: /* AC_DISPLAY_BUTON_MODE_HOLD */
    {
        BUZZER_PlaySound(BUZZER_SOUND_CLICK);
        timer_remove_attr(AC_TASK_DISPLAY_ID, SETTING_ANIM_TICK_SIG);
        SCREEN_TRAN(scr_menu_game_handle, &scr_menu_game);
    }
    break;

    case 17: /* AC_DISPLAY_BUTON_UP_HOLD */
    {
        setting_worm_fast = 1;
        setting_buzzer_enabled = 1;
    }
    break;
    case 18: /* AC_DISPLAY_BUTON_DOWN_HOLD */
    {
        setting_worm_fast = 0;
        setting_buzzer_enabled = 0;
    }
    break;

    default:
        break;
    }
}