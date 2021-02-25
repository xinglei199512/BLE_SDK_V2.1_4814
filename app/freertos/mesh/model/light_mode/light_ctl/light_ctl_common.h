/**
 ****************************************************************************************
 *
 * @file   light_ctl_common.h
 *
 * @brief  .
 *
 * @author  ZHAOYUNLIU
 * @date    2018-11-27 18:38
 * @version <0.0.0.1>
 * * @license 
 *              Copyright (C) Apollo 2018
 *                         ALL Right Reserved.
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup MESH_light_ctl_common_API Mesh light_ctl_common API
 * @ingroup MESH_API
 * @brief Mesh light_ctl_common  API
 *
 * @{
 ****************************************************************************************
 */

#ifndef APP_FREERTOS_MESH_MODEL_LIGHT_MODE_LIGHT_CTL_LIGHT_CTL_COMMON_H_
#define APP_FREERTOS_MESH_MODEL_LIGHT_MODE_LIGHT_CTL_LIGHT_CTL_COMMON_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "mesh_errors.h"
#include "sdk_mesh_config.h"
#include "sdk_mesh_definitions.h"
#include "light_ctl_client.h"
#include "light_ctl_server.h"
#include "light_ctl_msg_handler.h"
#include "model_msg.h"

typedef struct
{
    uint16_t ctl_lightness;
    uint16_t ctl_temperature;
    uint16_t ctl_delta_uv;
    uint8_t tid;
} __attribute((packed))light_ctl_set_default_t;

typedef struct
{
    uint16_t ctl_lightness;
    uint16_t ctl_temperature;
    uint16_t ctl_delta_uv;
    uint8_t tid;
    uint8_t trans_time;
    uint8_t delay;
} __attribute((packed))light_ctl_set_t;

typedef struct
{
    uint16_t present_ctl_lightness;
    uint16_t present_ctl_temperature;
    uint16_t target_ctl_lightness;
    uint16_t target_ctl_temperature;
    uint8_t remaining_time;
}__attribute((packed))light_ctl_status_t;

typedef struct
{
    uint16_t present_ctl_lightness;
    uint16_t present_ctl_temperature;
}__attribute((packed))light_ctl_default_status_t;


typedef struct
{
    uint16_t ctl_temperature;
    uint16_t ctl_delta_uv;
    uint8_t tid;
} __attribute((packed))light_ctl_temperature_default_t;

typedef struct
{
    uint16_t ctl_temperature;
    uint16_t ctl_delta_uv;
    uint8_t tid;
    uint8_t trans_time;
    uint8_t delay;
} __attribute((packed))light_ctl_temperature_set_t;

typedef struct
{
    uint16_t present_ctl_temperature;
    uint16_t present_ctl_delta_uv;
    uint16_t target_ctl_temperature;
    uint16_t target_ctl_delta_uv;
    uint8_t remaining_time;
}__attribute((packed))light_ctl_temperature_status_t;

typedef struct
{
    uint16_t present_ctl_temperature;
    uint16_t present_ctl_delta_uv;
}__attribute((packed))light_ctl_temperature_default_status_t;

typedef struct
{
    uint16_t range_min;
    uint16_t range_max;
} __attribute((packed))light_ctl_temperature_range_set_t;

typedef struct
{
    uint8_t  status_code;
    uint16_t range_min;
    uint16_t range_max;
}__attribute((packed))light_ctl_temperature_range_status_t;


typedef struct
{
    uint16_t ctl_lightness;
    uint16_t ctl_temperature;
    uint16_t ctl_delta_uv;
} __attribute((packed))light_ctl_default_set_t;

typedef struct
{
    uint16_t ctl_lightness;
    uint16_t ctl_temperature;
    uint16_t ctl_delta_uv;
} __attribute((packed))light_ctl_default_default_status_t;

#endif /* APP_FREERTOS_MESH_MODEL_LIGHT_MODE_LIGHT_CTL_LIGHT_CTL_COMMON_H_ */ 
/// @} MESH_light_ctl_common_API

