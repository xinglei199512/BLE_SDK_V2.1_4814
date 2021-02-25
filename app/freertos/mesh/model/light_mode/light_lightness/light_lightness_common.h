/**
 ****************************************************************************************
 *
 * @file   light_lightness_common.h
 *
 * @brief  .
 *
 * @author  ZHAOYUNLIU
 * @date    2018-11-21 17:15
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
 * @addtogroup MESH_light_lightness_common_API Mesh light_lightness_common API
 * @ingroup MESH_API
 * @brief Mesh light_lightness_common  API
 *
 * @{
 ****************************************************************************************
 */

#ifndef APP_FREERTOS_MESH_MODEL_LIGHT_MODE_LIGHT_LIGHTNESS_LIGHT_LIGHTNESS_COMMON_H_
#define APP_FREERTOS_MESH_MODEL_LIGHT_MODE_LIGHT_LIGHTNESS_LIGHT_LIGHTNESS_COMMON_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "mesh_errors.h"
#include "sdk_mesh_config.h"
#include "sdk_mesh_definitions.h"
#include "light_lightness_client.h"
#include "light_lightness_server.h"
#include "light_lightness_msg_handler.h"
#include "mesh_model.h"
#include "model_msg.h"

typedef struct
{
    uint16_t lightness_actual;
    uint8_t tid;                                                             
} __attribute((packed))light_lightness_actual_default_t;

typedef struct
{
    uint16_t lightness_actual;
    uint8_t tid;                                                             
    uint8_t trans_time;
    uint8_t delay;                            
} __attribute((packed))light_lightness_actual_set_t;

typedef struct
{
    uint16_t present_lightness_actual;
    uint16_t target_lightness_actual;
    uint8_t remaining_time;
}__attribute((packed))light_lightness_actual_status_t;

typedef struct
{
    uint16_t lightness_actual;
} __attribute((packed))light_lightness_actual_default_status_t;


typedef struct
{
    uint16_t lightness_linear;
    uint8_t tid;                                                             
} __attribute((packed))light_lightness_linear_default_t;

typedef struct
{
    uint16_t lightness_linear;
    uint8_t tid;                                                             
    uint8_t trans_time;
    uint8_t delay;                            
} __attribute((packed))light_lightness_linear_set_t;

typedef struct
{
    uint16_t present_lightness_linear;
    uint16_t target_lightness_linear;
    uint8_t remaining_time;
}__attribute((packed))light_lightness_linear_status_t;

typedef struct
{
    uint16_t lightness_linear;
} __attribute((packed))light_lightness_linear_default_status_t;

typedef struct
{
    uint16_t lightness;
} __attribute((packed))light_lightness_publish_status_t;

typedef struct
{
    uint16_t lightness_range_min;
    uint16_t lightness_range_max;
} __attribute((packed))light_lightness_range_t;

typedef struct
{
    uint8_t  status_code;
    uint16_t lightness_range_min;
    uint16_t lightness_range_max;
} __attribute((packed))light_lightness_range_status_t;

#endif /* APP_FREERTOS_MESH_MODEL_LIGHT_MODE_LIGHT_LIGHTNESS_LIGHT_LIGHTNESS_COMMON_H_ */ 
/// @} MESH_light_lightness_common_API

