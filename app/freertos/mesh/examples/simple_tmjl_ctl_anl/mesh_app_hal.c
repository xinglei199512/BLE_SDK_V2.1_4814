/**
 ****************************************************************************************
 *
 * @file   mesh_app_hal.c
 *
 * @brief  .
 *
 * @author  ZHAOYUNLIU
 * @date    2018-11-27 13:48
 * @version <0.0.0.1>
 *
 * @license 
 *              Copyright (C) Apollo 2018
 *                         ALL Right Reserved.
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "osapp_config.h"

#include "node_setup.h"
#include "mesh_app_hal.h"
#include "io_ctrl.h"
#include "plf.h"
#include "mesh_core_api.h"
#include "app_pwm.h"
#include "proxy_s.h"
#include "generic_common.h"
#include "timer_wrapper.h"

/*
 * MACROS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */
#define GATT_BEACON_ON  1
#define GATT_BEACON_OFF 0

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

typedef enum
{
    RESET_CHECK = 0,
    RESET_DELAY = 1,
    RESET_DONE = 2,
}RESET_PHASE_T;

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */




/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
 
mesh_timer_t gal_heartbeat_Timer;
static app_pwm_inst_t bxpwm = PWM_INSTANCE();
/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

void hal_set_lightness_led(uint8_t io_num, uint16_t lightness)
{
#if 0
    lightness = 0xffff - lightness;
#endif
    if(io_num == BX_DONGLE_LED1_B) {
        app_pwm_set_time(&bxpwm.inst, PWM_CHANNEL_0, lightness, 0xffff - lightness);
        //LOG(3,"hal_set_lightness_led B lightness:%x\n", lightness);
    }else if(io_num == BX_DONGLE_LED1_G) {
        app_pwm_set_time(&bxpwm.inst, PWM_CHANNEL_3, lightness, 0xffff - lightness);
        //LOG(3,"hal_set_lightness_led G lightness:%x\n", lightness);
    }else if(io_num == BX_DONGLE_LED1_R) {
        app_pwm_set_time(&bxpwm.inst, PWM_CHANNEL_4, lightness, 0xffff - lightness);
        //LOG(3,"hal_set_lightness_led R lightness:%x\n", lightness);
    }else if(io_num == BX_DONGLE_LED2_R) {
        app_pwm_set_time(&bxpwm.inst, PWM_CHANNEL_1, lightness, 0xffff - lightness);
        //LOG(3,"hal_set_lightness_led2 lightness:%x \n", lightness);
    }else if(io_num == BX_DONGLE_LED2_G) {
        app_pwm_set_time(&bxpwm.inst, PWM_CHANNEL_2, lightness, 0xffff - lightness);
        //LOG(3,"hal_set_lightness_led2 lightness:%x \n", lightness);
    }
}

void hal_set_hsl_led(uint8_t io_num, uint16_t lightness)
{
    LOG(3, "io_num:%d lightness:%x\n", io_num, lightness);
    hal_set_lightness_led(io_num, lightness);
}

void hal_app_pwm_init(void)
{
    bxpwm.channel[PWM_CHANNEL_0].pin_num = BX_DONGLE_LED1_B;
    bxpwm.channel[PWM_CHANNEL_1].pin_num = BX_DONGLE_LED2_R;
    bxpwm.channel[PWM_CHANNEL_2].pin_num = BX_DONGLE_LED2_G;
    bxpwm.channel[PWM_CHANNEL_3].pin_num = BX_DONGLE_LED1_G;
    bxpwm.channel[PWM_CHANNEL_4].pin_num = BX_DONGLE_LED1_R;

    app_pwm_init(&bxpwm.inst);

    app_pwm_set_time(&bxpwm.inst, PWM_CHANNEL_0, 0, 0xffff);
    app_pwm_set_time(&bxpwm.inst, PWM_CHANNEL_1, 0, 0xffff);
    app_pwm_set_time(&bxpwm.inst, PWM_CHANNEL_2, 0, 0xffff);
    app_pwm_set_time(&bxpwm.inst, PWM_CHANNEL_3, 0, 0xffff);
    app_pwm_set_time(&bxpwm.inst, PWM_CHANNEL_4, 0, 0xffff);
}

void hal_init_leds(void)
{
#if 1
    //1. io dir
    io_cfg_output(BX_DONGLE_LED1_B);
    io_cfg_output(BX_DONGLE_LED1_G);
    io_cfg_output(BX_DONGLE_LED1_R);
    io_cfg_output(BX_DONGLE_LED2_R);
    io_cfg_output(BX_DONGLE_LED2_G);
    //io_cfg_output(RESET_PIN_NUM_MINI);
    //io_cfg_output(GATT_BEACON_PIN_NUM_MINI);
    //io_cfg_output(PKT_FULL_PIN_NUM_MINI);

    //2. set output
    io_pin_clear(BX_DONGLE_LED1_R);
    io_pin_clear(BX_DONGLE_LED1_G);
    io_pin_clear(BX_DONGLE_LED1_B);

    io_pin_set(BX_DONGLE_LED2_R);
    io_pin_clear(BX_DONGLE_LED2_G);
    //io_pin_set(RESET_PIN_NUM_MINI);
    //io_pin_set(PKT_FULL_PIN_NUM_MINI);
    //io_pin_set(GATT_BEACON_PIN_NUM_MINI);

    hal_app_pwm_init();
#endif
}

void hal_init_buttons(void)
{   
#if 0
    //1. io dir
    io_cfg_input(BTN3_PIN_NUM);
    io_cfg_input(BTN4_PIN_NUM);
    //2. pull
    io_pin_pull_write(BTN3_PIN_NUM,IO_PULL_UP);
    io_pin_pull_write(BTN4_PIN_NUM,IO_PULL_UP);
    //3. int cfg
    io_ext_int_cfg(BTN3_PIN_NUM,EXT_INT_TRIGGER_NEG_EDGE,hal_button3_cb);
    io_ext_int_cfg(BTN4_PIN_NUM,EXT_INT_TRIGGER_NEG_EDGE,hal_button4_cb);
    //4. en
    io_ext_int_en(BTN3_PIN_NUM,true);
    io_ext_int_en(BTN4_PIN_NUM,true);
#endif
}
static void halTimeoutCallback(mesh_timer_t xTimer)
{
}

void hal_timer_init(void)
{
     gal_heartbeat_Timer = xTimerCreate("gal_heartbeat_Timer",pdMS_TO_TICKS(HAL_TIMER_TICK),pdTRUE,(void *)0,halTimeoutCallback);
#if 0
     if(gal_heartbeat_Timer != NULL)
     {
         xTimerStart(gal_heartbeat_Timer,0);
     }
#endif
}

void hal_heartbeat_init(void)
{
    hal_timer_init();
}

#define RESET_TIMER_TIME_S 6
#define ONOFF_TIMER_TIME_MS 500
#define UMPRO_DEVICE_ONOFF_LIGHT_TIME_MS 60 * 1000

static mesh_timer_t onoff_timer;
static mesh_timer_t reset_timer;

void delete_onoff_timer(void)
{
    LOG(3, "delete_onoff_timer:%x\n", onoff_timer);
    if(onoff_timer) {
        mesh_timer_stop(onoff_timer);
        mesh_timer_delete(onoff_timer);
        onoff_timer = NULL;
    }

    app_pwm_set_time(&bxpwm.inst, PWM_CHANNEL_2, 0xffff, 0);
}

void delete_reset_timer(void)
{
    LOG(3, "delete_reset_timer:%x\n", reset_timer);
    if(reset_timer) {
        mesh_timer_stop(reset_timer);
        mesh_timer_delete(reset_timer);
        reset_timer = NULL;
    }

}

static void user_onoff_timer_callback(mesh_timer_t xTimer)
{
    static uint8_t onoff = 0;
    static uint8_t count = 0;
    LOG(3,"user_onoff_timer_callback onoff:%d count:%d\n", onoff, count);
    if(onoff) {
        app_pwm_set_time(&bxpwm.inst, PWM_CHANNEL_2, 0xffff, 0);
    }else {
        app_pwm_set_time(&bxpwm.inst, PWM_CHANNEL_2, 0, 0xffff);
    }
    onoff = onoff ? 0 : 1;

    count++;

    if(count > UMPRO_DEVICE_ONOFF_LIGHT_TIME_MS / ONOFF_TIMER_TIME_MS) {
        delete_onoff_timer();
    }
}

void user_reset_timer_init(void)
{
     reset_timer = mesh_timer_create("reset_timer", \
             pdS_TO_TICKS(RESET_TIMER_TIME_S),pdFALSE,(void *)0, user_reset_timer_callback);

     if(reset_timer != NULL)
     {
         mesh_timer_start(reset_timer);
     }
}

void user_onoff_timer_init(void)
{
     onoff_timer = mesh_timer_create("onoff_timer", \
             pdMS_TO_TICKS(ONOFF_TIMER_TIME_MS), pdTRUE, onoff_timer, user_onoff_timer_callback);

     if(onoff_timer != NULL)

     {
         mesh_timer_start(onoff_timer);
     }
}

