#ifndef __WORM_GAME_BORDER_H__
#define __WORM_GAME_BORDER_H__

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
#include "scr_worm.h"

typedef struct {
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
} worm_game_border_t;

//codes

void border_settings(void);
void worm_game_border_handler(ak_msg_t* msg);

extern worm_game_border_t worm_game_border;

#endif //__WORM_GAME_BORDER_H__