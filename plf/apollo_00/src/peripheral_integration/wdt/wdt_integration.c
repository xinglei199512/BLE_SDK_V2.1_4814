/*
 * wdt_integration.c
 *
 *  Created on: 2018Äê6ÔÂ26ÈÕ
 *      Author: CBK
 */


#include "periph_common.h"
#include "app_wdt.h"
#include "periph_recovery.h"
#include "bx_config.h"
#include "apollo_00.h"
#include "bx_dbg.h"
#include "rst_gen.h"
#include "clk_gate.h"
#include "reg_sysc_per.h"
#include "pshare.h"
#include "log.h"
#include "sysctrl.h"
#include "ll.h"






static app_wdt_inst_t *wdt_inst_env = 0;

static void wdt_intr_op(app_wdt_inst_t * inst,intr_operation_t op)
{
    if(op == INTR_ENABLE)
    {
        wdt_inst_env = inst;
        NVIC_EnableIRQ(WDT_IRQn);
    }else if(op == INTR_DISABLE)
    {
        NVIC_DisableIRQ(WDT_IRQn);
    }else if(op == INTR_CLR)
    {
        NVIC_ClearPendingIRQ(WDT_IRQn);
    }else
    {
        BX_ASSERT(0);
    }
}

static void wdt_sw_rst(app_wdt_inst_t * inst)
{
    srst_cpu(WDT_SRST_CPU);
}

static void wdt_clk_gate(app_wdt_inst_t * inst,clk_gate_operation_t op)
{
    if(op == SET_CLK)
    {
        sysc_cpu_clkg1_set(CPU_CLKG_SET_WDT);
    }
    if(op == CLR_CLK)
    {
        sysc_cpu_clkg1_set(CPU_CLKG_CLR_WDT);
    }
    return;

}

static void wdt_pin_cfg(app_wdt_inst_t * inst,uint8_t pin_num,uint32_t pin_role,bool enable)
{
    //NOTING TO DO
    return;
}

static void wdt_sys_stat(app_wdt_inst_t *inst,uint32_t sys_stat)
{
    uint8_t idx = inst->idx;
    switch(sys_stat)
    {
    case WDT_INIT:
        recovery_list_add(cpu_domain_recovery_buf, CPU_DOMAIN_WDT,&inst->inst);
    break;
    case WDT_UNINIT:
        recovery_list_remove(cpu_domain_recovery_buf,CPU_DOMAIN_WDT);
    break;
    case WDT_START:
        if(idx)
        {
            //cpu_domain_stat.timer1 = 1;
        }else
        {
            //cpu_domain_stat.timer0 = 1;
        }
    break;
    case WDT_STOP:
        if(idx)
        {
            //cpu_domain_stat.timer1 = 0;
        }else
        {
            //cpu_domain_stat.timer0 = 0;
        }
    break;
    default:
        LOG(LOG_LVL_WARN,"unexpected sys stat: %d\n",sys_stat);
    break;
    }
}

const periph_universal_func_set_t wdt_universal_func = 
{
    .intr_op_func     = (intr_op_func_t    )wdt_intr_op,
    .sw_rst_func      = (sw_rst_func_t     )wdt_sw_rst,
    .clk_src_cfg_func = NULL,
    .clk_gate_func    = (clk_gate_func_t   )wdt_clk_gate,
    .pin_cfg_func     = (pin_cfg_func_t    )wdt_pin_cfg,
    .sys_stat_func    = (sys_stat_func_t   )wdt_sys_stat,
};


void WDT_IRQHandler(void)
{
    //reg_wdt_t *reg = (reg_wdt_t *)REG_WDT_BASE;
#if 0
    uint32_t timer_isr_status0 , timer_isr_status1;
    temp_val0 = reg->ch[0].TIMERCURRENTVALUE;
    temp_val1 = reg->ch[1].TIMERCURRENTVALUE;

    timer_isr_status0 = FIELD_RD((&reg->ch[0]), TIMERINTSTATUS, TIMER_TIMERINTERRUPTSTATUSREGISTER);
    timer_isr_status1 = FIELD_RD((&reg->ch[1]), TIMERINTSTATUS, TIMER_TIMERINTERRUPTSTATUSREGISTER);
    if(timer_isr_status0) app_wdt_isr(timer_inst_env[0]);
    if(timer_isr_status1) app_wdt_isr(timer_inst_env[1]);
#endif
    LOG(LOG_LVL_INFO,"WDT_IRQHandler\n");
    app_wdt_isr(wdt_inst_env);
}




