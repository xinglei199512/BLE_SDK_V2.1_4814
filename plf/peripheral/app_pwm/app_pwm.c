/*
 * app_pwm.c
 *
 *  Created on: 2018��6��26��
 *      Author: jiachuang
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "reg_pwm.h"
#include "field_manipulate.h"
#include "app_pwm.h"
#include "bx_dbg.h"
#include "ll.h"
#include "rwip.h"
#include "log.h"
#include "co_utils.h"
#include "app_dmac_wrapper.h"
#include "string.h"
#include "plf.h"
#include "rf_reg_typedef.h"
#include "reg_sysc_cpu.h"
#include "io_ctrl.h"
#include "periph_mngt.h"


extern periph_universal_func_set_t pwm_universal_func;



periph_err_t app_pwm_reinit(periph_inst_handle_t hdl)
{
    app_pwm_inst_t *inst = CONTAINER_OF(hdl, app_pwm_inst_t, inst);

    for(uint8_t i=0;i<PWM_CHANNEL_SUM;i++)
    {
        if(0x00 != inst->channel[i].pin_num)
        {
            pwm_universal_func.pin_cfg_func(inst,inst->channel[i].pin_num,i,true);
        }
    }
    pwm_universal_func.clk_src_cfg_func(inst,ALL_CHANNEL_PWM_CLK_DIV);
    
    //STAT
    pwm_universal_func.sys_stat_func(inst,PWM_INIT);
    pwm_universal_func.clk_gate_func(inst,CLR_CLK);
    return PERIPH_NO_ERROR;
}



periph_err_t app_pwm_init(periph_inst_handle_t hdl)
{
    app_pwm_inst_t *inst = CONTAINER_OF(hdl, app_pwm_inst_t, inst);

    if(periph_lock(&inst->init_lock)==false)//init uninit lock
    {
        return PERIPH_STATE_ERROR;
    }

    return app_pwm_reinit(hdl);
}

periph_err_t app_pwm_uninit(periph_inst_handle_t hdl)
{
    app_pwm_inst_t *inst = CONTAINER_OF(hdl, app_pwm_inst_t, inst);
    reg_pwm_t *reg = inst->reg;
    
    if(periph_unlock(&inst->init_lock)==false)//init uninit lock
    {
        return PERIPH_STATE_ERROR;
    }

    for(uint8_t i=0;i<PWM_CHANNEL_SUM;i++)
    {
        FIELD_WR((&reg->ch[i]), PWM_EN, PWM_PWM_EN_R, 0);
        inst->inter_stat[i]=PWM_INTER_INVALID;
        if(0x00 != inst->channel[i].pin_num)
        {
            pwm_universal_func.pin_cfg_func(inst,inst->channel[i].pin_num,i,false);
        }
    }
    pwm_universal_func.clk_gate_func(inst,CLR_CLK);
    pwm_universal_func.intr_op_func(inst,INTR_DISABLE);

    //STAT
    pwm_universal_func.sys_stat_func(inst,PWM_UNINIT);
    return PERIPH_NO_ERROR;
}

static void pwm_set_sys_stat(periph_inst_handle_t hdl , pwm_channel_t channel , pwm_inter_stat_t stat)
{
    app_pwm_inst_t *inst = CONTAINER_OF(hdl, app_pwm_inst_t, inst);
    uint8_t busy = 0;

    inst->inter_stat[channel] = stat;

    ATOMIC_OP(
    for(uint8_t i=0;i<PWM_CHANNEL_SUM;i++)
    {
        if(inst->inter_stat[i] == PWM_INTER_NORMAL_PWM)
        {
            busy = 1;
        }
    }
    pwm_universal_func.sys_stat_func(inst , busy ? PWM_OUTPUT_START : PWM_OUTPUT_STOP);
    );
}

static void pwm_set_constant(periph_inst_handle_t hdl , pwm_channel_t channel , uint8_t value) 
{
    app_pwm_inst_t *inst = CONTAINER_OF(hdl, app_pwm_inst_t, inst);

    if(0 == value)
    {
        pwm_set_sys_stat(hdl,channel,PWM_INTER_STILL_LOW);
        pwm_universal_func.pin_cfg_func(inst,inst->channel[channel].pin_num,channel,true);//must set after state changed
        io_pin_clear(inst->channel[channel].pin_num);//must set io after pin_cfg_func
    }
    else
    {
        pwm_set_sys_stat(hdl,channel,PWM_INTER_STILL_HIGH);
        pwm_universal_func.pin_cfg_func(inst,inst->channel[channel].pin_num,channel,true);
        io_pin_set(inst->channel[channel].pin_num);
    }
    io_cfg_output(inst->channel[channel].pin_num);
    
}

static void pwm_set_start(periph_inst_handle_t hdl , pwm_channel_t channel , uint16_t high_time, uint16_t low_time)
{
    app_pwm_inst_t *inst = CONTAINER_OF(hdl, app_pwm_inst_t, inst);
    reg_pwm_t *reg = inst->reg;
    //STAT
    pwm_set_sys_stat(hdl,channel,PWM_INTER_NORMAL_PWM);   
    //clk,pin,enable
    pwm_universal_func.pin_cfg_func(inst,inst->channel[channel].pin_num,channel,true);
    pwm_universal_func.clk_gate_func(inst,SET_CLK);
    //set parameter
    if(high_time > 0)   high_time -= 1;
    if(low_time  > 0)   low_time  -= 1;
    FIELD_WR((&reg->ch[channel]), PWM_SETTING, PWM_PWM_HIGH, high_time);
    FIELD_WR((&reg->ch[channel]), PWM_SETTING, PWM_PWM_LOW,  low_time );
    FIELD_WR((&reg->ch[channel]), PWM_EN, PWM_PWM_EN_R, 1);
}

/**
 * @brief app_pwm_set_time APP_PWM module
 * @param[in] 		hdl 		the peripheral common instance handler(pointer) for the PWM instance
 * @param[in]       channel     the channel switched.
 * @param[in] 		high_time 	set pwm high time , uint:ticks
 * @param[in]       low_time    set pwm low time  , uint:ticks
 */
void app_pwm_set_time(periph_inst_handle_t hdl , pwm_channel_t channel , uint16_t high_time, uint16_t low_time)
{
    BX_ASSERT(!((high_time == 0) && (low_time == 0)));
    if((high_time == 0) || (low_time == 0))
    {
        pwm_set_constant(hdl , channel , high_time ? 1 : 0);
    }
    else
    {
        pwm_set_start(hdl , channel , high_time , low_time);
    }
}



/**
 * @brief app_pwm_set_duty APP_PWM module
 * @param[in] 		hdl 		the peripheral common instance handler(pointer) for the PWM instance
 * @param[in]       channel     the channel switched.
 * @param[in] 		frequency 	set pwm frequency unit  hz   (max 160k hz)
 * @param[in]       percent     set pwm percent (0~100)
 */
void app_pwm_set_duty(periph_inst_handle_t hdl , pwm_channel_t channel,uint32_t frequency,uint8_t percent)
{
   	BX_ASSERT((hdl)&&(frequency)&&(frequency<=160000)&&(percent<=100));
	
	if((0==percent) || (100 == percent))
	{	
		pwm_set_constant(hdl , channel , percent ? 1 : 0);
	}
	else
	{
		uint32_t high_time = ((MAX_PWM_CLK_FREQUENCY)*percent)/(frequency*100);
		uint32_t low_time  = ((MAX_PWM_CLK_FREQUENCY)*(100-percent))/(frequency*100);
		BX_ASSERT(high_time);//high time must >=1
		BX_ASSERT(low_time );//low  time must >=1
		BX_ASSERT((high_time+low_time) >= 100);
		app_pwm_set_time(hdl , channel , high_time , low_time);
	}
}




