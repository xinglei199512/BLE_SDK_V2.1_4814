/**
 ****************************************************************************************
 *
 * @file   model_common.h
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

/**
 ****************************************************************************************
 * @addtogroup MESH_model_common_API Mesh model_common API
 * @ingroup MESH_API
 * @brief Mesh model_common  API
 *
 * @{
 ****************************************************************************************
 */

#ifndef APP_FREERTOS_MESH_MODEL_MODEL_COMMON_H_
#define APP_FREERTOS_MESH_MODEL_MODEL_COMMON_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "mesh_errors.h"
#include "sdk_mesh_config.h"
#include "sdk_mesh_config_pro.h"
#include "sdk_mesh_definitions.h"
#include "generic_common.h"
#include "generic_transition_server.h"

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
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

typedef void(*mesh_state_bound_cb_t)(uint8_t, uint16_t);

typedef enum
{
    MODEL_STATE_BOUND_ONOFF = 0,
    MODEL_STATE_BOUND_LEVEL,
    MODEL_STATE_BOUND_POWERUP,
    MODEL_STATE_BOUND_TRANSITION_TIME,
    MODEL_STATE_BOUND_LIGHT_LIGHTNESS,
    MODEL_STATE_BOUND_LIGHT_CTL,
    MODEL_STATE_BOUND_LIGHT_HSL,
    MODEL_STATE_BOUND_LIGHT_xyL,
    MODEL_STATE_BOUND_LIGHT_LC,
    MODEL_STATE_BOUND_MAX,
}model_state_bound_type;

typedef struct
{
    mesh_state_bound_cb_t bound_cb;
    model_state_bound_type bound_type;
    uint16_t led_r:5;
    uint16_t led_g:5;
    uint16_t led_b:6;
}model_state_bound_field_t;

int model_common_state_bound_field_set(uint8_t element_id, model_state_bound_type bound_type, mesh_state_bound_cb_t cb);
int model_common_state_bound_leds_num_set(uint8_t element_id, uint16_t led_r, uint16_t led_g, uint16_t led_b);
model_state_bound_field_t* model_common_state_bound_get_from_element_id(uint8_t element_id);
generic_delay_trans_param_t* model_common_delay_trans_timer_get_from_element_id(uint8_t element_id);
#endif /* APP_FREERTOS_MESH_MODEL_MODEL_COMMON_H_ */ 
/// @} MESH_model_common_API

