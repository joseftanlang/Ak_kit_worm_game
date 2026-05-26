#include "scr_setting.h"

static uint8_t selected_item = 0; // 0 = speed, 1 = apples, 2 = song, 3 = buzzer
static uint8_t setting_anim_tick = 0;
static uint8_t setting_worm_speed = 1;     /* 1..5 */
static uint8_t setting_apple_count = 1;    /* 1..8 */
static uint8_t setting_song_index = 0;     /* 0..4 */
static uint8_t setting_buzzer_enabled = 0; /* default: BUZZER OFF */

#define SETTING_ANIM_INTERVAL_MS (85)
#define SETTING_ANIM_TICK_SIG (AK_USER_DEFINE_SIG + 183)
#define SETTING_ROW_COUNT (4)
#define SETTING_ROW_TOP_Y (18)
#define SETTING_ROW_STEP_Y (10)
#define SETTING_ROW_HEIGHT (9)

#define SETTING_WORM_SPEED_MIN (1)
#define SETTING_WORM_SPEED_MAX (5)
#define SETTING_APPLE_COUNT_MIN (1)
#define SETTING_APPLE_COUNT_MAX (8)
#define SETTING_SONG_COUNT (5)

typedef struct
{
    uint32_t magic;
    uint8_t worm_speed;
    uint8_t apple_count;
    uint8_t song_index;
    uint8_t buzzer_enabled;
} setting_persist_t;

static const uint16_t setting_worm_tick_intervals_ms[SETTING_WORM_SPEED_MAX] = {
    300,
    240,
    180,
    130,
    90,
};
static const char *setting_speed_values[] = {"1", "2", "3", "4", "5"};
static const char *setting_apple_values[] = {"1", "2", "3", "4", "5", "6", "7", "8"};
static const char *setting_song_values[] = {"WELCOME", "MARIO", "HIGH", "LOW", "XMAS"};
static const buzzer_sound_t setting_song_sounds[] = {
    BUZZER_SOUND_WELCOME,
    BUZZER_SOUND_SUPER_MARIO,
    BUZZER_SOUND_HIGHSCORE,
    BUZZER_SOUND_LOWSCORE,
    BUZZER_SOUND_MERRY_CHRISTMAS,
};

static uint8_t setting_loaded = 0;

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
static void setting_draw_row(int index, int y, const char *label, const char *value);
static void setting_toggle_selected_item();
static void setting_load_if_needed(void);
static void setting_save(void);
static const char *setting_get_speed_value();
static const char *setting_get_apple_value();
static const char *setting_get_song_value();
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

static void setting_load_if_needed(void)
{
    setting_persist_t stored = {0};

    if (setting_loaded)
    {
        return;
    }

    if (eeprom_read(EEPROM_WORM_SETTING_MAGIC_ADDR, (uint8_t *)&stored, sizeof(stored)) == EEPROM_DRIVER_OK &&
        stored.magic == EEPROM_WORM_SETTING_MAGIC)
    {
        if (stored.worm_speed >= SETTING_WORM_SPEED_MIN && stored.worm_speed <= SETTING_WORM_SPEED_MAX)
        {
            setting_worm_speed = stored.worm_speed;
        }

        if (stored.apple_count >= SETTING_APPLE_COUNT_MIN && stored.apple_count <= SETTING_APPLE_COUNT_MAX)
        {
            setting_apple_count = stored.apple_count;
        }

        if (stored.song_index < SETTING_SONG_COUNT)
        {
            setting_song_index = stored.song_index;
        }

        setting_buzzer_enabled = stored.buzzer_enabled ? 1 : 0;
    }

    setting_loaded = 1;
}

static void setting_save(void)
{
    setting_persist_t stored;

    setting_load_if_needed();

    stored.magic = EEPROM_WORM_SETTING_MAGIC;
    stored.worm_speed = setting_worm_speed;
    stored.apple_count = setting_apple_count;
    stored.song_index = setting_song_index;
    stored.buzzer_enabled = setting_buzzer_enabled;

    eeprom_write(EEPROM_WORM_SETTING_MAGIC_ADDR, (uint8_t *)&stored, sizeof(stored));
}

uint16_t scr_game_setting_get_worm_tick_interval_ms(void)
{
    setting_load_if_needed();

    uint8_t speed = setting_worm_speed;

    if (speed < SETTING_WORM_SPEED_MIN)
    {
        speed = SETTING_WORM_SPEED_MIN;
    }
    else if (speed > SETTING_WORM_SPEED_MAX)
    {
        speed = SETTING_WORM_SPEED_MAX;
    }

    return setting_worm_tick_intervals_ms[speed - 1];
}

uint8_t scr_game_setting_get_apple_count(void)
{
    setting_load_if_needed();

    if (setting_apple_count < SETTING_APPLE_COUNT_MIN)
    {
        return SETTING_APPLE_COUNT_MIN;
    }

    if (setting_apple_count > SETTING_APPLE_COUNT_MAX)
    {
        return SETTING_APPLE_COUNT_MAX;
    }

    return setting_apple_count;
}

buzzer_sound_t scr_game_setting_get_song(void)
{
    setting_load_if_needed();

    uint8_t song_index = setting_song_index;

    if (song_index >= SETTING_SONG_COUNT)
    {
        song_index = 0;
    }

    return setting_song_sounds[song_index];
}

uint8_t scr_game_setting_is_buzzer_enabled(void)
{
    setting_load_if_needed();

    return setting_buzzer_enabled;
}

static const char *setting_get_speed_value()
{
    setting_load_if_needed();

    if (setting_worm_speed < SETTING_WORM_SPEED_MIN || setting_worm_speed > SETTING_WORM_SPEED_MAX)
    {
        return setting_speed_values[0];
    }

    return setting_speed_values[setting_worm_speed - 1];
}

static const char *setting_get_apple_value()
{
    setting_load_if_needed();

    uint8_t apple_count = scr_game_setting_get_apple_count();
    return setting_apple_values[apple_count - 1];
}

static const char *setting_get_song_value()
{
    setting_load_if_needed();

    uint8_t song_index = setting_song_index;

    if (song_index >= SETTING_SONG_COUNT)
    {
        song_index = 0;
    }

    return setting_song_values[song_index];
}

static const char *setting_get_buzzer_value()
{
    setting_load_if_needed();

    return setting_buzzer_enabled ? "ON" : "OFF";
}

static void setting_toggle_selected_item()
{
    switch (selected_item)
    {
    case 0:
        if (setting_worm_speed < SETTING_WORM_SPEED_MAX)
        {
            setting_worm_speed++;
        }
        else
        {
            setting_worm_speed = SETTING_WORM_SPEED_MIN;
        }
        setting_save();
        break;

    case 1:
        if (setting_apple_count < SETTING_APPLE_COUNT_MAX)
        {
            setting_apple_count++;
        }
        else
        {
            setting_apple_count = SETTING_APPLE_COUNT_MIN;
        }
        setting_save();
        break;

    case 2:
        setting_song_index = (uint8_t)((setting_song_index + 1) % SETTING_SONG_COUNT);
        setting_save();
        break;

    case 3:
    default:
        setting_buzzer_enabled = setting_buzzer_enabled ? 0 : 1;
        BUZZER_Silent(setting_buzzer_enabled ? false : true);
        setting_save();
        break;
    }
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

static void setting_draw_row(int index, int y, const char *label, const char *value)
{
    if (index == selected_item)
    {
        view_render.fillRoundRect(4, y, 120, SETTING_ROW_HEIGHT, 2, WHITE);
        view_render.setTextColor(BLACK);
        view_render.setCursor(8, y + 2);
        view_render.print("> ");
        view_render.print(label);
        view_render.setCursor(82, y + 2);
        view_render.print(value);
    }
    else
    {
        view_render.drawRoundRect(4, y, 120, SETTING_ROW_HEIGHT, 2, WHITE);
        view_render.setTextColor(WHITE);
        view_render.setCursor(8, y + 2);
        view_render.print("  ");
        view_render.print(label);
        view_render.setCursor(82, y + 2);
        view_render.print(value);
    }
}

void view_scr_game_setting()
{
    view_render.clear();

    setting_draw_background();
    setting_draw_title();

    setting_draw_row(0, SETTING_ROW_TOP_Y, "SPEED", setting_get_speed_value());
    setting_draw_row(1, SETTING_ROW_TOP_Y + SETTING_ROW_STEP_Y, "APPLE", setting_get_apple_value());
    setting_draw_row(2, SETTING_ROW_TOP_Y + (SETTING_ROW_STEP_Y * 2), "SONG", setting_get_song_value());
    setting_draw_row(3, SETTING_ROW_TOP_Y + (SETTING_ROW_STEP_Y * 3), "BUZZER", setting_get_buzzer_value());
}

void scr_game_setting_handle(ak_msg_t *msg)
{
    switch (msg->sig)
    {
    case SCREEN_ENTRY:
        setting_load_if_needed();
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
        if (selected_item > 0)
        {
            selected_item--;
        }
        else
        {
            selected_item = SETTING_ROW_COUNT - 1;
        }
        view_scr_game_setting();
        BUZZER_PlaySound(BUZZER_SOUND_CLICK);
    }
    break;

    case 13: /* AC_DISPLAY_BUTON_DOWN_PRESSED */
    {
        selected_item = (selected_item + 1) % SETTING_ROW_COUNT;
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
        setting_worm_speed = SETTING_WORM_SPEED_MAX;
        setting_apple_count = SETTING_APPLE_COUNT_MAX;
        setting_song_index = 1;
        setting_buzzer_enabled = 1;
        BUZZER_Silent(false);
        setting_save();
        view_scr_game_setting();
    }
    break;
    case 18: /* AC_DISPLAY_BUTON_DOWN_HOLD */
    {
        setting_worm_speed = SETTING_WORM_SPEED_MIN;
        setting_apple_count = SETTING_APPLE_COUNT_MIN;
        setting_song_index = SETTING_SONG_COUNT - 1;
        setting_buzzer_enabled = 0;
        BUZZER_Silent(true);
        setting_save();
        view_scr_game_setting();
    }
    break;

    default:
        break;
    }
}