/*
 * rc_calib.c
 *
 *  Created on: 2016Äê6ÔÂ21ÈÕ
 *      Author: mingzhou
 */

//#include "rc_calib.h"
#include <stdint.h>
#include <stdbool.h>
#include "reg_sysc_cpu.h"
#include "log.h"

uint32_t calib32k;
void rc_calib_start()
{
    sysc_cpu_calb32k_start_setf(1);
}

void rc_calib_end(bool wait)
{
    uint32_t i = 0;
    while(!(sysc_cpu_calb32k_start_getf() == 0))
    {
        if(wait == false) return;
        i++;
    }
    if(wait)
    {
        LOG(LOG_LVL_INFO, "rc_calib loop count i=%d\n", i);
    }
    calib32k = sysc_cpu_calb32k_rslt_getf();
    BX_ASSERT(calib32k);
}

uint32_t get_rc32k_calib_val()
{
    return calib32k;
}
