#ifndef __SCR_GAME_SETTING_H__
#define __SCR_GAME_SETTING_H__

#include "fsm.h"
#include "port.h"
#include "message.h"
#include "timer.h"

#include "sys_ctrl.h"
#include "sys_dbg.h"

#include "app.h"
#include "app_dbg.h"
#include "task_list.h"
#include "task_display.h"
#include "view_render.h"

#include "buzzer.h"

#include "eeprom.h"
#include "app_eeprom.h"

#include "screens.h"
#include "screens_bitmap.h"

extern view_screen_t scr_game_setting;
extern view_dynamic_t dyn_view_item_game_setting;
extern void scr_game_setting_handle(ak_msg_t* msg);
extern uint16_t scr_game_setting_get_worm_tick_interval_ms(void);
extern uint8_t scr_game_setting_is_buzzer_enabled(void);

#endif // __SCR_GAME_SETTING_H__