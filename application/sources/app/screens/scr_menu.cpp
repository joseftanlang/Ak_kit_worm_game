#include "scr_menu.h"
#include "screens.h"
#include "scr_setting.h"

static const char *menu_items[] = {"Idle", "Worm", "Charts", "Settings", "QR Code"};
static int menu_count = sizeof(menu_items) / sizeof(menu_items[0]);
static int menu_index = 0;
static int menu_view_offset = 0;
static uint8_t menu_anim_tick = 0;
static uint32_t menu_last_input_ms = 0;

#define MENU_ANIM_INTERVAL_MS (70)
#define MENU_ANIM_PAUSE_AFTER_INPUT_MS (300)
#define MENU_ANIM_TICK_SIG (AK_USER_DEFINE_SIG + 181)
#define MENU_VISIBLE_COUNT (2)
#define MENU_TITLE_Y (3)
#define MENU_DIVIDER_Y (14)
#define MENU_ITEMS_TOP_Y (20)
#define MENU_ITEM_STEP_Y (20)
#define MENU_ITEM_HEIGHT (18)

// Simple starfield animation for menu background
typedef struct
{
    int16_t x;
    uint8_t y;
    uint8_t speed;
} menu_star_t;

// Predefined stars with different speeds
static menu_star_t menu_stars[] = {
    {16, 5, 1},
    {34, 11, 2},
    {52, 7, 1},
    {71, 14, 2},
    {86, 4, 1},
    {104, 13, 2},
    {118, 9, 1},
};

static const uint8_t menu_star_count = sizeof(menu_stars) / sizeof(menu_stars[0]);

static void view_scr_menu();
static void menu_tick();
static void menu_draw_background();
static void menu_draw_item(int i, int y);
static void menu_draw_title();

view_dynamic_t dyn_view_item_menu_game = {
    {.item_type = ITEM_TYPE_DYNAMIC},
    view_scr_menu};

view_screen_t scr_menu_game = {
    &dyn_view_item_menu_game,
    ITEM_NULL,
    ITEM_NULL,
    .focus_item = 0,
};

// Menu animation and rendering functions
static void menu_tick()
{
    menu_anim_tick++;

    for (uint8_t i = 0; i < menu_star_count; i++)
    {
        if (menu_stars[i].x <= menu_stars[i].speed)
        {
            menu_stars[i].x = 127;
            menu_stars[i].y = (uint8_t)((menu_anim_tick + (i * 20)) % 64);
        }
        else
        {
            menu_stars[i].x -= menu_stars[i].speed;
        }
    }
}

// Draw the animated starfield background and static elements
static void menu_draw_background()
{
    for (uint8_t i = 0; i < menu_star_count; i++)
    {
        if (((menu_anim_tick + i) & 0x01) == 0 || menu_stars[i].speed > 1)
        {
            view_render.drawPixel(menu_stars[i].x, menu_stars[i].y, WHITE);
        }
    }

    view_render.drawRect(0, 0, 128, 64, WHITE);
}

// Draw the menu title and divider line
static void menu_draw_title()
{
    view_render.setTextSize(1);
    view_render.setTextColor(WHITE);
    view_render.setCursor(20, MENU_TITLE_Y);
    view_render.print("WORM GAME MENU");

    view_render.drawFastHLine(0, MENU_DIVIDER_Y, 128, WHITE); // Divider line
}

// Draw a single menu item, highlighting the selected one
static void menu_draw_item(int i, int y)
{
    const char *label = menu_items[i];

    // Highlight the selected item with a filled background and inverted text color
    if (i == menu_index)
    {
        view_render.fillRoundRect(2, y - 1, 124, MENU_ITEM_HEIGHT, 2, WHITE);
        view_render.setTextColor(BLACK);
        view_render.setCursor(8, y + 5);
        view_render.print("> ");
        view_render.print(label);
    }
    // Non-selected items are drawn with normal text color and no background
    else
    {
        view_render.drawRoundRect(2, y - 1, 124, MENU_ITEM_HEIGHT, 2, WHITE);
        view_render.setTextColor(WHITE);
        view_render.setCursor(8, y + 5);
        view_render.print("  ");
        view_render.print(label);
    }
}

// Update the entire menu screen, including background and all visible items
static void view_scr_menu()
{
    view_render.clear();
    menu_draw_background();
    menu_draw_title();

    for (int row = 0; row < MENU_VISIBLE_COUNT; row++)
    {
        int i = menu_view_offset + row;
        if (i >= menu_count)
            break;

        int y = MENU_ITEMS_TOP_Y + row * MENU_ITEM_STEP_Y;
        menu_draw_item(i, y);
    }
}

// Main message handler for the menu screen, handling entry, exit, animation ticks, and button presses
void scr_menu_game_handle(ak_msg_t *msg)
{
    switch (msg->sig)
    {
    case SCREEN_ENTRY:
    {
        view_render.initialize();
        view_render_display_on();
        menu_index = 0;
        menu_view_offset = 0;
        menu_anim_tick = 0;
        menu_last_input_ms = 0;
        timer_set(AC_TASK_DISPLAY_ID, MENU_ANIM_TICK_SIG, MENU_ANIM_INTERVAL_MS, TIMER_PERIODIC);
        view_scr_menu();
    }
    break;

    case SCREEN_EXIT:
    {
        timer_remove_attr(AC_TASK_DISPLAY_ID, MENU_ANIM_TICK_SIG);
    }
    break;

    case MENU_ANIM_TICK_SIG:
    {
        uint32_t current_ms = sys_ctrl_millis();
        if ((current_ms - menu_last_input_ms) < MENU_ANIM_PAUSE_AFTER_INPUT_MS)
        {
            break;
        }

        menu_tick();
        view_scr_menu();
    }
    break;

    case AC_DISPLAY_BUTON_UP_PRESSED:
    {
        menu_last_input_ms = sys_ctrl_millis();

        if (menu_index > 0)
            menu_index--;
        else
            menu_index = menu_count - 1;

        if (menu_index < menu_view_offset)
            menu_view_offset = menu_index;
        else if (menu_index >= menu_view_offset + MENU_VISIBLE_COUNT)
            menu_view_offset = menu_index - MENU_VISIBLE_COUNT + 1;

        BUZZER_PlaySound(BUZZER_SOUND_CLICK);
    }
    break;

    case AC_DISPLAY_BUTON_DOWN_PRESSED:
    {
        menu_last_input_ms = sys_ctrl_millis();

        menu_index = (menu_index + 1) % menu_count;

        if (menu_index < menu_view_offset)
            menu_view_offset = menu_index;
        else if (menu_index >= menu_view_offset + MENU_VISIBLE_COUNT)
            menu_view_offset = menu_index - MENU_VISIBLE_COUNT + 1;

        BUZZER_PlaySound(BUZZER_SOUND_CLICK);
    }
    break;

    case AC_DISPLAY_BUTON_MODE_PRESSED:
    {
        menu_last_input_ms = sys_ctrl_millis();

        BUZZER_PlaySound(BUZZER_SOUND_CLICK);
        timer_remove_attr(AC_TASK_DISPLAY_ID, MENU_ANIM_TICK_SIG);
        switch (menu_index)
        {
        case 0:
            SCREEN_TRAN(scr_idle_handle, &scr_idle);
            break;
        case 1:
            SCREEN_TRAN(scr_worm_handle, &scr_worm);
            break;
        case 2:
            SCREEN_TRAN(scr_charts_handle, &scr_charts);
            break;
        case 3:
            SCREEN_TRAN(scr_game_setting_handle, &scr_game_setting);
            break;
        case 4:
            SCREEN_TRAN(scr_qrcode_handle, &scr_qrcode);
            break;
        default:
            break;
        }
    }
    break;

    case AC_DISPLAY_BUTON_DOWN_MODE_PRESSED:
    {
        // Off the sound
        BUZZER_Silent(1);
    }
    break;

    case AC_DISPLAY_BUTON_UP_MODE_PRESSED:
    {
        // On The sound
        BUZZER_Silent(0);
    }
    break;

    default:
        break;
    }
}