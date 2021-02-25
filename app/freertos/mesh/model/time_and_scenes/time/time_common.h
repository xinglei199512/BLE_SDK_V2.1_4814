/**
 ****************************************************************************************
 *
 * @file   time_common.h
 *
 * @brief  .
 *
 * @author  ZHAOYUNLIU
 * @date    2018-12-17 11:31
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
 * @addtogroup MESH_time_common_API Mesh time_common API
 * @ingroup MESH_API
 * @brief Mesh time_common  API
 *
 * @{
 ****************************************************************************************
 */

#ifndef APP_FREERTOS_MESH_MODEL_TIME_COMMON_H_
#define APP_FREERTOS_MESH_MODEL_TIME_COMMON_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "mesh_errors.h"
#include "sdk_mesh_config.h"
#include "sdk_mesh_definitions.h"
#include "time_msg_handler.h"
#include "time_client.h"
#include "model_msg.h"

typedef struct
{
    uint64_t tai_seconds:40;
    uint64_t subsecond  :8;
    uint64_t uncertainty:8;
    uint64_t time_authority:1;
    uint64_t tai_utc_delta:15;
    uint64_t time_zone_offset:8;
}__attribute((packed))time_set_t;

typedef struct
{
    uint64_t time_zone_offset_new:8;
    uint64_t tai_of_zone_change:40;
}__attribute((packed))time_zone_set_t;

typedef struct
{
    uint8_t time_role;
}__attribute((packed))time_role_set_t;

typedef struct
{
    uint64_t tai_utc_delta_new:15;
    uint64_t padding:1;
    uint64_t tai_of_delta_change:40;
}__attribute((packed))tai_utc_delta_set_t;

typedef time_set_t  time_status_t;

typedef struct
{
    uint64_t tai_seconds:40;
}__attribute((packed))time_status_default_t;


typedef struct
{
    uint64_t time_zone_offset_current:8;
    uint64_t time_zone_offset_new:8;
    uint64_t tai_of_zone_change:40;
}__attribute((packed))time_zone_status_t;


typedef struct
{
    uint64_t tai_utc_delta_current:15;
    uint64_t padding1:1;
    uint64_t tai_utc_delta_new:15;
    uint64_t padding2:1;
    uint64_t tai_of_delta_change:40;
}__attribute((packed))tai_utc_delta_status_t;

typedef time_role_set_t  time_role_status_t;

#endif /* APP_FREERTOS_MESH_MODEL_TIME_COMMON_H_ */ 
/// @} MESH_time_common_API

