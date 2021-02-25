/*
 * app_wdt.c
 *
 *  Created on: 2018Äê6ÔÂ26ÈÕ
 *      Author: CBK
 */



#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "reg_wdt.h"
#include "field_manipulate.h"
#include "app_wdt.h"
#include "bx_dbg.h"
#include "ll.h"
#include "rwip.h"
#include "log.h"
#include "co_utils.h"
#include "string.h"
#include "plf.h"
#include "rf_reg_typedef.h"
#include "reg_sysc_cpu.h"


extern periph_universal_func_set_t wdt_universal_func;



void app_wdt_isr(app_wdt_inst_t *inst)
{
    reg_wdt_t *reg = inst->reg;

    FIELD_RD(reg, WDT_EOI, WDT_INTERRUPTCLEARREGISTER);
    inst->param.callback(inst->param.callback_param);    
}



void app_wdt_init(periph_inst_handle_t hdl)
{
    app_wdt_inst_t *inst = CONTAINER_OF(hdl, app_wdt_inst_t, inst);
    reg_wdt_t *reg = inst->reg;

    //CANNOT SOFT RESET
    wdt_universal_func.sw_rst_func(inst); 
    wdt_universal_func.pin_cfg_func(inst,0,0,true);   
    wdt_universal_func.clk_gate_func(inst,SET_CLK);

    //set parameter
    FIELD_WR(reg, WDT_CR , WDT_RMOD, inst->param.mode);
    FIELD_WR(reg, WDT_TORR, WDT_TOP , inst->param.timeout_period);
    FIELD_WR(reg, WDT_CR, WDT_RPL , inst->param.plck);
    
    //STAT
    wdt_universal_func.sys_stat_func(inst,WDT_INIT);
    
    //CANNOT CLEAR CLOCK GATE
}

void app_wdt_uninit(periph_inst_handle_t hdl)
{
    app_wdt_inst_t *inst = CONTAINER_OF(hdl, app_wdt_inst_t, inst);

    wdt_universal_func.clk_gate_func(inst,CLR_CLK);
    wdt_universal_func.intr_op_func(inst,INTR_DISABLE);
    //STAT
    wdt_universal_func.sys_stat_func(inst,WDT_UNINIT);
}



void app_wdt_start(periph_inst_handle_t hdl)
{
    app_wdt_inst_t *inst = CONTAINER_OF(hdl, app_wdt_inst_t, inst);
    reg_wdt_t *reg = inst->reg;

    FIELD_WR(reg, WDT_CR, WDT_VAL, 1);
    //STAT
    wdt_universal_func.intr_op_func(inst,INTR_CLR);
    wdt_universal_func.intr_op_func(inst,INTR_ENABLE);
    wdt_universal_func.sys_stat_func(inst,WDT_START);   
}


void app_wdt_stop(periph_inst_handle_t hdl)
{
    app_wdt_inst_t *inst = CONTAINER_OF(hdl, app_wdt_inst_t, inst);
    reg_wdt_t *reg = inst->reg;

    FIELD_WR(reg, WDT_CR, WDT_VAL, 0);
    //STAT
    wdt_universal_func.sys_stat_func(inst,WDT_STOP);   
}

void app_wdt_feed_dog(periph_inst_handle_t hdl)
{
    app_wdt_inst_t *inst = CONTAINER_OF(hdl, app_wdt_inst_t, inst);
    reg_wdt_t *reg = inst->reg;

    __asm("nop");
    FIELD_WR(reg, WDT_CRR, WDT_COUNT_RESTART_REGISTER, RESTART_COUNT);
}


