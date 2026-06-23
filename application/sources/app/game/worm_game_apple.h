#ifndef __WORM_GAME_APPLE_H__
#define __WORM_GAME_APPLE_H__

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
#include <stdlib.h>

#include "screens_bitmap.h"
#include "worm_game_border.h"
#include "scr_worm.h"
#include "scr_setting.h"


//codes
#define MAX_APPLES      (8)
#define APPLE_WIDTH     (7)
#define APPLE_HEIGHT    (7)
#define BORDER_MARGIN 1
#define APPLE_LIFE_SECONDS 3
#define APPLE_RESPAWN_SECONDS 2
#define APPLE_RANDOM_TRIES 32
#define APPLE_BORDER_PADDING 1

enum {
	AC_APPLE_INIT = AK_USER_DEFINE_SIG,
	AC_APPLE_TICK,
};

typedef struct {
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
	uint8_t is_active;
	uint8_t life_seconds;
	uint8_t respawn_seconds;
    uint8_t apple_image;
} worm_game_apple_t;

extern worm_game_apple_t apples_no[MAX_APPLES];

void counting_apples(void);
void apple_init(void);

#endif //__WORM_GAME_APPLE_H__