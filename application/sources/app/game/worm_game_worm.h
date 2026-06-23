#ifndef __WORM_GAME_WORM_H__
#define __WORM_GAME_WORM_H__

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
#include "worm_game_lives.h"
#include "scr_worm.h"
#include "worm_game_score.h"
#include "worm_game_worm.h"

//codes
#define WORM_START_POSITION_X  (51)
#define WORM_START_POSITION_Y  (36)
#define WORM_START_WIDTH       (20)
#define WORM_START_HEIGHT      (5)

#ifndef WORM_MOVE_STEP
#define WORM_MOVE_STEP         (5)
#endif

#define WORM_MAX_TRAIL         (384)
#define WORM_INITIAL_LENGTH    (4)

typedef struct {
    uint8_t x;
    uint8_t y;
} worm_game_point_t;

typedef struct {
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
    uint8_t worm_image;         
    uint8_t dir;                /* worm_game_dir_t value */
    uint16_t length;            /* number of segments in the worm trail (including head) */ 
    uint16_t grow_pending;      /* number of segments to grow (not yet applied to length) */
    worm_game_point_t trail[WORM_MAX_TRAIL];
} worm_game_t;

typedef enum {
    WORM_DIR_RIGHT = 0,
    WORM_DIR_DOWN,
    WORM_DIR_LEFT,
    WORM_DIR_UP,
} worm_game_dir_t;

extern worm_game_t worm_game;

/* Object-only (movement and input handled by screen code) */
void worm_init(void);
void worm_set_direction(worm_game_dir_t d);
worm_game_dir_t worm_get_direction(void);
uint8_t worm_advance(void);
void worm_grow(void);

/* message signals for `worm_game_worm_handler` */
enum {
    AC_WORM_INIT = AK_USER_DEFINE_SIG,
    AC_WORM_TICK,
    AC_WORM_SET_DIR_RIGHT,
    AC_WORM_SET_DIR_DOWN,
    AC_WORM_SET_DIR_LEFT,
    AC_WORM_SET_DIR_UP,
    AC_WORM_GROW,
};


#endif //__WORM_GAME_WORM_H__