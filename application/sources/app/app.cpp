/**
 ******************************************************************************
 * Clean application core (refactored)
 ******************************************************************************
 **/

#include <malloc.h>
#include <stdlib.h>
#include <string.h>

#include <vector>
#include <deque>
#include <queue>
#include <array>
#include <map>

/* kernel */
#include "ak.h"
#include "message.h"
#include "timer.h"
#include "fsm.h"

/* drivers */
#include "led.h"
#include "button.h"
#include "flash.h"

/* app core */
#include "app.h"
#include "app_dbg.h"
#include "app_bsp.h"
#include "app_flash.h"
#include "app_non_clear_ram.h"

/* tasks */
#include "task_list.h"
#include "task_shell.h"
#include "task_life.h"
#include "task_if.h"
#include "task_rf24_if.h"
#include "task_uart_if.h"
#include "task_display.h"
#include "task_zigbee.h"

/* system */
#include "sys_boot.h"
#include "sys_irq.h"
#include "sys_io.h"
#include "sys_ctrl.h"
#include "sys_dbg.h"

/* hardware */
#include "SPI.h"
#include "HardwareSerial.h"

/* UI */
#include "screen_manager.h"

/* buzzer + settings */
#include "buzzer.h"
#include "scr_setting.h"

using namespace std;

/* =========================================================
 * Build / Version Info
 * ========================================================= */

#if defined(RELEASE)
const char *app_run_mode = "RELEASE";
#else
static const char *app_run_mode = "DEBUG";
#endif

const app_info_t app_info{
    APP_MAGIC_NUMBER,
    APP_VER,
};

/* =========================================================
 * Globals
 * ========================================================= */

static boot_app_share_data_t boot_app_share_data;
volatile bool g_controller_mode = false;

/* =========================================================
 * Forward declarations
 * ========================================================= */

static void app_power_on_reset();
static void app_start_timer();
static void app_init_state_machine();
static void app_task_init();

/* =========================================================
 * MAIN
 * ========================================================= */

int main_app()
{
    APP_PRINT("App run mode: %s, version: %d.%d.%d.%d\n",
              app_run_mode,
              app_info.version[0],
              app_info.version[1],
              app_info.version[2],
              app_info.version[3]);

    sys_soft_reboot_counter++;

    /* kernel init */
    ENTRY_CRITICAL();
    task_init();
    task_create((task_t *)app_task_table);
    task_polling_create((task_polling_t *)app_task_polling_table);
    EXIT_CRITICAL();

    /* hardware init */
    sys_ctrl_independent_watchdog_init();
    sys_ctrl_soft_watchdog_init(200);

    SPI.begin();

    io_cfg_adc1();
    adc_bat_io_cfg();

    flash_io_ctrl_init();

    /* system init */
    sys_boot_init();

    led_init(&led_life, led_life_init, led_life_on, led_life_off);

    ring_buffer_char_init(&ring_buffer_console_rev,
                          buffer_console_rev,
                          BUFFER_CONSOLE_REV_SIZE);

    /* buttons */
    button_init(&btn_mode, 10, BUTTON_MODE_ID,
                io_button_mode_init,
                io_button_mode_read,
                btn_mode_callback);

    button_init(&btn_up, 10, BUTTON_UP_ID,
                io_button_up_init,
                io_button_up_read,
                btn_up_callback);

    button_init(&btn_down, 10, BUTTON_DOWN_ID,
                io_button_down_init,
                io_button_down_read,
                btn_down_callback);

    button_enable(&btn_mode);
    button_enable(&btn_up);
    button_enable(&btn_down);

    /* buzzer */
    BUZZER_Init();

    if (scr_game_setting_is_buzzer_enabled())
        BUZZER_PlaySound(BUZZER_SOUND_STARTUP);
    else
        BUZZER_Sleep(true);

    /* boot data */
    flash_read(APP_FLASH_INTTERNAL_SHARE_DATA_SECTOR_1,
               (uint8_t *)&boot_app_share_data,
               sizeof(boot_app_share_data));

    if (boot_app_share_data.is_power_on_reset == SYS_POWER_ON_RESET)
    {
        app_power_on_reset();
    }

    /* fatal log */
    fatal_log_t log;
    flash_read(APP_FLASH_AK_DBG_FATAL_LOG_SECTOR,
               (uint8_t *)&log,
               sizeof(log));

    log.restart_times++;

    flash_erase_sector(APP_FLASH_AK_DBG_FATAL_LOG_SECTOR);
    flash_write(APP_FLASH_AK_DBG_FATAL_LOG_SECTOR,
                (uint8_t *)&log,
                sizeof(log));

    /* zigbee (optional) */
#if defined(TASK_ZIGBEE_EN)
    Serial2.begin();
    Serial2.setTimeout(100);
#endif

    EXIT_CRITICAL();

#if defined(TASK_ZIGBEE_EN)
    zigbee_network.start_coordinator(0);
#endif

    /* app start */
    app_init_state_machine();
    app_start_timer();
    app_task_init();

#if !defined(IF_LINK_UART_EN)
    sys_ctrl_shell_sw_to_nonblock();
#endif

    return task_run();
}

/* =========================================================
 * Polling tasks
 * ========================================================= */

void task_polling_zigbee()
{
#if defined(TASK_ZIGBEE_EN)
    zigbee_network.update();
#endif
}

/* console input + game control */
void task_polling_console()
{
    uint8_t c;

    while (!ring_buffer_char_is_empty(&ring_buffer_console_rev))
    {
        ENTRY_CRITICAL();
        c = ring_buffer_char_get(&ring_buffer_console_rev);
        EXIT_CRITICAL();

        if (g_controller_mode)
        {
            /* If the shell buffer is empty, a single game-key fires a game event directly.
             * If the user has already started typing a command (shell.index > 0), or
             * the key is not a recognised game key, fall through to normal shell handling
             * so that commands like "lcd d" can still be used. */
            if (shell.index == 0)
            {
                bool handled = true;
                switch (c)
                {
                case 'w': case 'W':
                case 0x41: /* Arrow Up (ESC [ A) - simplified: treat 'A' as up when in controller mode */
                    task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_BUTON_UP_PRESSED);
                    break;

                case 's': case 'S':
                case 0x42: /* Arrow Down (ESC [ B) */
                    task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_BUTON_DOWN_PRESSED);
                    break;

                case 'd': case 'D':
                case ' ':       /* Space = MODE / select / back */
                case '\r':      /* Enter = MODE / select / back */
                    task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_BUTON_MODE_PRESSED);
                    break;

                case 'a': case 'n': case 'N':
                    task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_BUTON_UP_MODE_PRESSED);
                    break;

                case 'q': case 'Q':
                    task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_BUTON_DOWN_MODE_PRESSED);
                    break;

                default:
                    handled = false;
                    break;
                }
                if (handled)
                    continue; /* consumed as game input, skip shell */
            }
            /* Fall through to shell handling below (partial command or non-game key) */
        }

        /* shell fallback */
        if (shell.index < SHELL_BUFFER_LENGHT - 1)
        {
            if (c == '\r' || c == '\n')
            {
                shell.data[shell.index] = 0;

                task_post_common_msg(
                    AC_TASK_SHELL_ID,
                    AC_SHELL_LOGIN_CMD,
                    (uint8_t *)shell.data,
                    shell.index + 1);

                shell.index = 0;
            }
            else
            {
                shell.data[shell.index++] = c;
            }
        }
        else
        {
            shell.index = 0;
        }
    }
}

/* =========================================================
 * Timer setup
 * ========================================================= */

static void app_start_timer()
{
    timer_set(AC_TASK_LIFE_ID, AC_LIFE_SYSTEM_CHECK,
              AC_LIFE_TASK_TIMER_LED_LIFE_INTERVAL,
              TIMER_PERIODIC);

    timer_set(AC_TASK_FW_ID, FW_CHECKING_REQ,
              FW_UPDATE_REQ_INTERVAL,
              TIMER_ONE_SHOT);

    timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_INITIAL,
              AC_DISPLAY_INITIAL_INTERVAL,
              TIMER_ONE_SHOT);
}

/* =========================================================
 * Init
 * ========================================================= */

static void app_init_state_machine()
{
    /* reserved */
}

static void app_task_init()
{
    SCREEN_CTOR(&scr_mng_app, scr_worm_menu_game_handle, &scr_menu_game);

    task_post_pure_msg(AC_TASK_RF24_IF_ID, AC_RF24_IF_INIT_NETWORK);
    task_post_pure_msg(AC_TASK_UART_IF_ID, AC_UART_IF_INIT);
}

/* =========================================================
 * IRQ
 * ========================================================= */

void sys_irq_timer_10ms()
{
    button_timer_polling(&btn_mode);
    button_timer_polling(&btn_up);
    button_timer_polling(&btn_down);
}

/* =========================================================
 * Boot helpers
 * ========================================================= */

static void app_power_on_reset()
{
    sys_soft_reboot_counter = 0;
}

void *app_get_boot_share_data()
{
    return &boot_app_share_data;
}