#ifndef __SCR_WORM_H__
#define __SCR_WORM_H__

//includes
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

#include <math.h>
#include <vector>

#include "screens_bitmap.h"

#include "worm_game_border.h"
#include "worm_game_eating.h"
#include "worm_game_lives.h"
#include "worm_game_score.h"
#include "worm_game_worm.h"
#include "worm_game_apple.h"

#define SCR_WIDTH 128
#define SCR_HEIGHT 64

//codes

extern view_dynamic_t dyn_view_item_worm;
extern view_screen_t scr_worm;
extern void scr_worm_handle(ak_msg_t* msg);
extern void game_gamer_handler(ak_msg_t* msg);
extern uint8_t worm_game_is_finished(void);
extern uint8_t worm_game_is_won(void);
extern void worm_game_finish(uint8_t won);
extern void worm_game_reset(void);

#endif //__SCR_WORM_H__