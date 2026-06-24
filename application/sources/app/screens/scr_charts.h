    #ifndef __SCR_CHARTS_H__
    #define __SCR_CHARTS_H__

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

    #include "buzzer.h"

    #include <math.h>
    #include <vector>

    #include "screens_bitmap.h"
    #include "worm_game_score.h"
    #include <string.h>

    #define CHART_WIDTH (128)
    #define CHART_HEIGHT (64)
    #define CHART_ANIM_INTERVAL_MS (110) // Original is 100 Animation tick interval in milliseconds, controlling the speed of the worm and star animations on the charts screen
    #define CHART_WORM_STEP_PX (3)
    #define CHART_WORM_SEGMENTS (7)
    #define CHART_WORM_SEGMENT_GAP (5)
    #define CHART_WORM_Y (24)
    #define CHART_WORM_HEAD_RADIUS (3)
    #define AC_CHARTS_ANIM_TICK (30) // = 190, unique timer signal for charts animation
    // #define CHART_MODE_SIG (AC_DISPLAY_BUTON_MODE_PRESSED) // = 11, maps to MODE button press to exit charts
    #define CHART_STAR_COUNT (10)

    //codes
    extern view_dynamic_t dyn_view_charts;
    extern view_screen_t scr_charts;
    extern void scr_charts_handle(ak_msg_t* msg);


    #endif //__SCR_CHARTS_H__