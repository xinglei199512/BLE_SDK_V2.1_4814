/**
 ****************************************************************************************
 *
 * @file   model_common.c
 *
 * @brief  .
 *
 * @author  ZHAOYUNLIU
 * @date    2018-12-10 14:28
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

#include "model_common.h"

/*
 * MACROS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */




/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
static model_state_bound_field_t state_bound[ELEMENT_NUM];
static generic_delay_trans_param_t delay_trans_timer[ELEMENT_NUM];

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/*
 * FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief   Func miaoshu
 *
 * @param[in] xxx1     Id of the message received.
 * @param[in] xxx2     Pointer to the parameters of the message.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
generic_delay_trans_param_t* model_common_delay_trans_timer_get_from_element_id(uint8_t element_id)
{
    if(element_id < ELEMENT_NUM) {
        return &delay_trans_timer[element_id];
    }

    return NULL;
}

int model_common_state_bound_field_set(uint8_t element_id, model_state_bound_type bound_type, mesh_state_bound_cb_t cb)
{
    if(element_id < ELEMENT_NUM) {
        if(bound_type >= state_bound[element_id].bound_type) {
            state_bound[element_id].bound_type = bound_type;
            state_bound[element_id].bound_cb = cb;
            return 0;
        }
    }
    return -1;
}

int model_common_state_bound_leds_num_set(uint8_t element_id, uint16_t led_r, uint16_t led_g, uint16_t led_b)
{
    if(element_id < ELEMENT_NUM) {
        state_bound[element_id].led_r = led_r;
        state_bound[element_id].led_g = led_g;
        state_bound[element_id].led_b = led_b;
        return 0;
    }
    return -1;
}

model_state_bound_field_t* model_common_state_bound_get_from_element_id(uint8_t element_id)
{
    if(element_id < ELEMENT_NUM) {
        return &state_bound[element_id];
    }

    return NULL;
}

