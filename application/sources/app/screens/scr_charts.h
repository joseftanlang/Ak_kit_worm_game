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

//codes

extern view_dynamic_t dyn_view_charts;
extern view_screen_t scr_charts;
extern void scr_charts_handle(ak_msg_t* msg);


#endif //__SCR_CHARTS_H__