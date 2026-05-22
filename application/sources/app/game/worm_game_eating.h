#ifndef __OBJ_EATING_H__
#define __OBJ_EATING_H__

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

//codes
#define EATING_EFFECT_MAX_RADIUS    (7) //it needs to be slightly bigger than the worm head.

enum {
	AC_EATING_INIT = AK_USER_DEFINE_SIG,
	AC_EATING_START,
	AC_EATING_TICK,
};

typedef struct {
    uint8_t visible;
    uint32_t x;
    uint32_t y;
    uint32_t radius;
    uint8_t eating_image;
} workm_game_eating_effect_t;

extern workm_game_eating_effect_t game_eating;

void eating_effect_init(void);
void eating_effect_start(uint32_t x, uint32_t y);
void eating_effect_tick(void);
void eating_effect_stop(void);

extern void game_eating_handler(ak_msg_t* msg);


#endif //__OBJ_EATING_H__