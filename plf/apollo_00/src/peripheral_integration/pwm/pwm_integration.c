/*
 * pwm_integration.c
 *
 *  Created on: 2018-6-26
 *      Author: jiachuang
 */



#include "periph_common.h"
#include "app_pwm.h"
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
#include "io_ctrl.h"

static void pwm_intr_op(app_pwm_inst_t * inst,intr_operation_t op)
{
    //NOTHING TO DO
    return;
}
static void pwm_sw_rst(app_pwm_inst_t * inst)
{
    srst_per(PWM_SRST_PER);
    return;
}
static void pwm_clk_src_cfg(app_pwm_inst_t * inst,uint32_t clk_cfg)
{
    ATOMIC_OP(
    sysc_per_clkg1_set(1<<1);
    sysc_per_clk_div_pwm_para_m1_setf(clk_cfg);
    sysc_per_clkg1_set(1<<0);
    );
    return;
}
static void pwm_clk_gate(app_pwm_inst_t * inst,clk_gate_operation_t op)
{
    if(op == SET_CLK)
    {
        sysc_per_clkg1_set(1<<2); 
        sysc_per_clkg1_set(1<<4); 
        sysc_per_clkg1_set(1<<6); 
        sysc_per_clkg1_set(1<<8); 
        sysc_per_clkg1_set(1<<10);
    }
    if(op == CLR_CLK)
    {
        sysc_per_clkg1_set(1<<3); 
        sysc_per_clkg1_set(1<<5); 
        sysc_per_clkg1_set(1<<7); 
        sysc_per_clkg1_set(1<<9); 
        sysc_per_clkg1_set(1<<11);
    }
    return;
}
static void pwm_pin_cfg(app_pwm_inst_t * inst,uint8_t pin_num,uint32_t pin_role,bool enable)
{
    bool pwm_pshare_en = false;
    if((pin_num < 2) || (pin_num > 23))
    {
        return ;
    }
    else
    {
        if(enable)
        {
            if((inst->inter_stat[pin_role] == PWM_INTER_INVALID) || (inst->inter_stat[pin_role] == PWM_INTER_STILL_LOW))
            {
                io_pin_clear (pin_num);
                io_cfg_output(pin_num);
            }
            else if(inst->inter_stat[pin_role] == PWM_INTER_STILL_HIGH)
            {
                io_pin_set   (pin_num);
                io_cfg_output(pin_num);
            }
            else
            {
                pwm_pshare_en = true;
            }
        }
        else
        {
            io_cfg_input(pin_num);
            sysctrl_io_config(pin_num, GPIO_DISABLED);
        }
        pshare_funcio_set(pin_num - 2,IO_PWM_0 + pin_role,pwm_pshare_en);
        
    }
    return;
}

static void pwm_sys_stat(app_pwm_inst_t *inst,uint32_t sys_stat)
{
    switch(sys_stat)
    {
    case PWM_INIT:
        recovery_list_add(periph_domain_recovery_buf, PERIPH_DOMAIN_PWM,&inst->inst);
        break;
    case PWM_UNINIT:
        recovery_list_remove(periph_domain_recovery_buf,PERIPH_DOMAIN_PWM);
        break;
    case PWM_OUTPUT_START:
        periph_domain_stat.pwm = 1;
        break;
    case PWM_OUTPUT_STOP:
        periph_domain_stat.pwm = 0;
        break;
    default:
        LOG(LOG_LVL_WARN,"unexpected sys stat: %d\n",sys_stat);
        break;
    }
}

const periph_universal_func_set_t pwm_universal_func =
{
    .intr_op_func     = (intr_op_func_t    )pwm_intr_op,
    .sw_rst_func      = (sw_rst_func_t     )pwm_sw_rst,
    .clk_src_cfg_func = (clk_src_cfg_func_t)pwm_clk_src_cfg,
    .clk_gate_func    = (clk_gate_func_t   )pwm_clk_gate,
    .pin_cfg_func     = (pin_cfg_func_t    )pwm_pin_cfg,
    .sys_stat_func    = (sys_stat_func_t   )pwm_sys_stat,
};

