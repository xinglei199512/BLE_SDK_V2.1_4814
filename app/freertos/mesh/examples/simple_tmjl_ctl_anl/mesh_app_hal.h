/**
 ****************************************************************************************
 *
 * @file   mesh_app_hal.h
 *
 * @brief  .
 *
 * @author  liuzy
 * @date    2018-09-25 17:29
 * @version <0.0.0.1>
 *
 * @license 
 *              Copyright (C) Apollo 2018
 *                         ALL Right Reserved.
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup MESH_mesh_app_hal_API Mesh mesh_app_hal API
 * @ingroup MESH_API
 * @brief Mesh mesh_app_hal  API
 *
 * @{
 ****************************************************************************************
 */

#ifndef FREERTOS_APP_MESH_EXAMPLES_SIMPLE_GENERIC_ONOFF_SERVER_MESH_APP_HAL_H_
#define FREERTOS_APP_MESH_EXAMPLES_SIMPLE_GENERIC_ONOFF_SERVER_MESH_APP_HAL_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "mesh_errors.h"
#include "sdk_mesh_config.h"
#include "sdk_mesh_definitions.h"

/*
 * MACROS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */
/* Apollo dongle board pin num */
//led
#if 1
#define BX_DONGLE_LED1_R    17
#define BX_DONGLE_LED1_G    4
#define BX_DONGLE_LED1_B    5
#define BX_DONGLE_LED2_R    6
#define BX_DONGLE_LED2_G    15
#define BX_DONGLE_LED2_B    21
#else
#define BX_DONGLE_LED1_R    4
#define BX_DONGLE_LED1_G    2
#define BX_DONGLE_LED1_B    3
#define BX_DONGLE_LED2_R    22
#define BX_DONGLE_LED2_G    20
#define BX_DONGLE_LED2_B    21
#endif
//button
#define BX_DONGLE_BTN3      21
#define BX_DONGLE_BTN4      21

/*demo use */
#define LED1_PIN_NUM        8
#define LED2_PIN_NUM        9
#define LED1_PIN_NUM_MINI        BX_DONGLE_LED1_B
#define RESET_PIN_NUM_MINI       BX_DONGLE_LED2_B
#define GATT_BEACON_PIN_NUM_MINI BX_DONGLE_LED1_G
#define PKT_FULL_PIN_NUM_MINI    BX_DONGLE_LED1_R


#define RELAY_PIN_INPUT         7
#define RELAY_PIN_OUTPUT        8
#define GATT_BEACON_PIN_INPUT   10
#define GATT_BEACON_PIN_OUTPUT  11



#define BTN3_PIN_NUM        BX_DONGLE_BTN3
#define BTN4_PIN_NUM        BX_DONGLE_BTN4

#define HAL_TIMER_TICK   200               // unit: ms
#define HEARTBEAT_BLINKY_TICK   1000               // unit: ms
#define HAL_RESET_TICK   3000               // unit: ms
#define HAL_RESET_DELAY_TICK   3000               // unit: ms
#define HAL_GATT_DELAY_TICK   60000               // unit: ms
#define HAL_USER_INPUT_TICK  10000               // unit: ms
/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
void hal_set_hsl_led(uint8_t io_num, uint16_t lightness);
void hal_init_leds(void);
void hal_init_buttons(void);
void hal_init_heartbeat_led(void);
void hal_heartbeat_init(void);
void make_light_blink(uint8_t count);
void make_user_attention(void);
void user_reset_timer_init(void);
void user_onoff_timer_init(void);
void hal_start_warm_led(void);
void delete_reset_timer(void);
void delete_onoff_timer(void);

#endif /* FREERTOS_APP_MESH_EXAMPLES_SIMPLE_GENERIC_ONOFF_SERVER_MESH_APP_HAL_H_ */ 
/// @} MESH_mesh_app_hal_API

