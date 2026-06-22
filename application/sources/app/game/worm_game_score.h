#ifndef __WORM_GAME_SCORE_H__
#define __WORM_GAME_SCORE_H__

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
#include "app_eeprom.h"
#include "eeprom.h"

#include "buzzer.h"

#include <math.h>
#include <vector>

#include "screens_bitmap.h"

//codes

#define START_SCORE     (0)
#define INCREMENT_SCORE (1)

typedef struct {
    uint32_t x;
    uint32_t y;
    uint32_t score;
} worm_game_score_display_t;

void score_init(void);
void score_reset(void);
void score_inc(void);
uint32_t score_get(void);
void score_commit_current(void);
uint32_t score_top_get(uint8_t index);

void worm_game_score_handler(ak_msg_t* msg);

/* message signals for `worm_game_score_handler` */
enum {
    AC_SCORE_INIT = AK_USER_DEFINE_SIG + 10,
    AC_SCORE_INC,
    AC_SCORE_RESET,
    AC_SCORE_COMMIT,
    AC_SCORE_TOP_GET,
};

#endif //__WORM_GAME_SCORE_H__