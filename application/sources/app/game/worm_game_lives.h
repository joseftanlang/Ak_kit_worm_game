#ifndef __OBJ_LIVES_H__
#define __OBJ_LIVES_H__

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

//codes
#define START_LIVES     (3)
#define MIN_LIVES       (0)

enum {
    AC_LIVES_INIT = AK_USER_DEFINE_SIG,
    AC_LIVES_RESET,
    AC_LIVES_INC,
    AC_LIVES_DEC,
};

typedef struct {
    uint32_t x;
    uint32_t y;
    uint32_t amount_lives;
} worm_game_lives_icon_t;

extern worm_game_lives_icon_t game_lives;

void lives_init(void);
void lives_reset(void);
void lives_inc(void);
void lives_dec(void);
uint32_t lives_get(void);

#endif //__OBJ_LIVES_H__