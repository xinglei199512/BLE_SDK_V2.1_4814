/*
 * app_pwm_test.c
 *
 *  Created on: 2018Äê6ÔÂ28ÈÕ
 *      Author: jiachuang
 */

#include "app_pwm.h"
#include "string.h"
#include "plf.h"

//#define PWM_SLEEP_TEST

//instance
app_pwm_inst_t bxpwm = PWM_INSTANCE();




static void app_pwm_delay_ms(uint32_t ms)
{
    while(ms --)
    {
        BX_DELAY_US(1000);
    }
}



void set_time_test(void)
{
    #ifndef PWM_SLEEP_TEST
    app_pwm_set_time(&bxpwm.inst , PWM_CHANNEL_0 ,   1 ,   1);      //high:0.0625us      low:0.0625us
    app_pwm_set_time(&bxpwm.inst , PWM_CHANNEL_1 , 160 , 160);      //high:10us          low:10us
    app_pwm_set_time(&bxpwm.inst , PWM_CHANNEL_2 , 160 , 320);      //high:10us          low:20us
    #endif
    app_pwm_set_time(&bxpwm.inst , PWM_CHANNEL_3 , 160 ,   0);      //still high
    app_pwm_set_time(&bxpwm.inst , PWM_CHANNEL_4 ,   0 , 160);      //still low
    app_pwm_delay_ms(100);

}
void set_duty_test(void)
{
    app_pwm_set_duty(&bxpwm.inst , PWM_CHANNEL_0 , 1000   ,   0);   //still low
    app_pwm_set_duty(&bxpwm.inst , PWM_CHANNEL_1 , 1000   , 100);   //still high
    app_pwm_set_duty(&bxpwm.inst , PWM_CHANNEL_2 , 1000   ,  10);   //high:100us         period:1000us
    app_pwm_set_duty(&bxpwm.inst , PWM_CHANNEL_3 , 10000  ,  20);   //high:20us          period:100us
    app_pwm_set_duty(&bxpwm.inst , PWM_CHANNEL_4 , 100000 ,  30);   //high:3us           period:10us
    app_pwm_delay_ms(100);

}

void app_pwm_output_test(void)
{
    bxpwm.channel[PWM_CHANNEL_0].pin_num = 8;
    bxpwm.channel[PWM_CHANNEL_1].pin_num = 9;
    bxpwm.channel[PWM_CHANNEL_2].pin_num = 10;
    bxpwm.channel[PWM_CHANNEL_3].pin_num = 11;
    bxpwm.channel[PWM_CHANNEL_4].pin_num = 12;

    #ifdef PWM_SLEEP_TEST
        app_pwm_init(&bxpwm.inst);
        set_time_test();
        return;
    #endif

    while(*(__IO uint32_t *)0 != 0)
    {
        app_pwm_init(&bxpwm.inst);
        app_pwm_delay_ms(100);
        
        set_time_test();
        set_time_test();
        set_duty_test();
        set_duty_test();
        
        app_pwm_uninit(&bxpwm.inst);
        app_pwm_delay_ms(100);
    }

}





