/**
 ****************************************************************************************
 *
 * @file   light_hsl_common.h
 *
 * @brief  .
 *
 * @author  ZHAOYUNLIU
 * @date    2018-11-27 18:47
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
 * @addtogroup MESH_light_hsl_common_API Mesh light_hsl_common API
 * @ingroup MESH_API
 * @brief Mesh light_hsl_common  API
 *
 * @{
 ****************************************************************************************
 */

#ifndef APP_FREERTOS_MESH_MODEL_LIGHT_MODE_LIGHT_HSL_LIGHT_HSL_COMMON_H_
#define APP_FREERTOS_MESH_MODEL_LIGHT_MODE_LIGHT_HSL_LIGHT_HSL_COMMON_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "mesh_errors.h"
#include "sdk_mesh_config.h"
#include "sdk_mesh_definitions.h"
#include "light_hsl_server.h"
#include "light_hsl_client.h"
#include "light_hsl_msg_handler.h"
#include "model_msg.h"

typedef struct
{
    uint16_t hsl_lightness;
    uint16_t hsl_hue;
    uint16_t hsl_saturation;
    uint8_t tid;
    uint8_t trans_time;
    uint8_t delay;
} __attribute((packed))light_hsl_set_t;

typedef struct
{
    uint16_t hsl_lightness;
    uint16_t hsl_hue;
    uint16_t hsl_saturation;
    uint8_t tid;
} __attribute((packed))light_hsl_set_default_t;

typedef struct
{
    uint16_t hsl_lightness;
    uint16_t hsl_hue;
    uint16_t hsl_saturation;
    uint8_t  remaining_time;
} __attribute((packed))light_hsl_status_t;

typedef struct
{
    uint16_t hsl_lightness;
    uint16_t hsl_hue;
    uint16_t hsl_saturation;
} __attribute((packed))light_hsl_default_status_t;

typedef struct
{
    uint16_t hsl_hue;
    uint8_t tid;
    uint8_t trans_time;
    uint8_t delay;
} __attribute((packed))light_hsl_hue_set_t;

typedef struct
{
    uint16_t hsl_hue;
    uint8_t tid;
} __attribute((packed))light_hsl_hue_set_default_t;

typedef struct
{
    uint16_t present_hsl_hue;
    uint16_t target_hsl_hue;
    uint8_t  remaining_time;
} __attribute((packed))light_hsl_hue_status_t;

typedef struct
{
    uint16_t present_hsl_hue;
} __attribute((packed))light_hsl_hue_default_status_t;

typedef struct
{
    uint16_t hsl_saturation;
    uint8_t tid;
    uint8_t trans_time;
    uint8_t delay;
} __attribute((packed))light_hsl_saturation_set_t;

typedef struct
{
    uint16_t hsl_saturation;
    uint8_t tid;
} __attribute((packed))light_hsl_saturation_set_default_t;

typedef struct
{
    uint16_t present_hsl_saturation;
    uint16_t target_hsl_saturation;
    uint8_t  remaining_time;
} __attribute((packed))light_hsl_saturation_status_t;

typedef struct
{
    uint16_t present_hsl_saturation;
} __attribute((packed))light_hsl_saturation_default_status_t;


typedef struct
{
    uint16_t hsl_lightness;
    uint16_t hsl_hue;
    uint16_t hsl_saturation;
} __attribute((packed))light_hsl_default_set_t;

typedef struct
{
    uint16_t hue_range_min;
    uint16_t hue_range_max;
    uint16_t saturation_range_min;
    uint16_t saturation_range_max;
} __attribute((packed))light_hsl_range_set_t;

typedef struct
{
    uint8_t status_code;
    uint16_t hue_range_min;
    uint16_t hue_range_max;
    uint16_t saturation_range_min;
    uint16_t saturation_range_max;
} __attribute((packed))light_hsl_range_status_t;
#endif /* APP_FREERTOS_MESH_MODEL_LIGHT_MODE_LIGHT_HSL_LIGHT_HSL_COMMON_H_ */ 
/// @} MESH_light_hsl_common_API

