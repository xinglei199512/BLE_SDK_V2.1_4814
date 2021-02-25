/*
 * app_wdt_test.c
 *
 *  Created on: 2018Äê6ÔÂ28ÈÕ
 *      Author: cbk
 */

#include "app_wdt.h"
#include "string.h"
#include "plf.h"
#include "log.h"
#include "clk_gate.h"


//instance
app_wdt_inst_t wdt_inst = WDT_INSTANCE(0);

void app_wdt_test(void)
{
    wdt_inst.param.mode = wdt_Irq_Reset;
    wdt_inst.param.plck = wdt_2_pclk_cycles;
    wdt_inst.param.timeout_period = 0x0f;
    app_wdt_init(&wdt_inst.inst);
    app_wdt_start(&wdt_inst.inst);
    while(1)
    {
        app_wdt_feed_dog(&wdt_inst.inst);
        LOG(LOG_LVL_INFO,"wdt_feed_dog\n");
        BX_DELAY_US(1000000);
    }
}

