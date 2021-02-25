/**
 ****************************************************************************************
 *
 * @file   scheduler_common.h
 *
 * @brief  .
 *
 * @author  ZHAOYUNLIU
 * @date    2018-12-21 10:38
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
 * @addtogroup MESH_scheduler_common_API Mesh scheduler_common API
 * @ingroup MESH_API
 * @brief Mesh scheduler_common  API
 *
 * @{
 ****************************************************************************************
 */

#ifndef APP_FREERTOS_MESH_MODEL_TIME_AND_SCENES_SCHEDULER_SCHEDULER_COMMON_H_
#define APP_FREERTOS_MESH_MODEL_TIME_AND_SCENES_SCHEDULER_SCHEDULER_COMMON_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "mesh_errors.h"
#include "sdk_mesh_config.h"
#include "sdk_mesh_definitions.h"
#include "scheduler_client.h"

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


typedef struct
{
    uint16_t schedules;
}__attribute((packed))scheduler_status_t;

typedef struct
{
    uint8_t index;
}__attribute((packed))scheduler_action_get_t;

typedef struct
{
    uint64_t index       :4;
    uint64_t year        :7;
    uint64_t month       :12;
    uint64_t day         :5;
    uint64_t hour        :5;
    uint64_t minute      :6;
    uint64_t second      :6;
    uint64_t dayofweek   :7;
    uint64_t action      :4;
    uint64_t trans_time  :8;
    uint64_t scene_number:16;
}__attribute((packed))scheduler_action_set_t;

typedef scheduler_action_set_t  scheduler_action_status_t;


#endif /* APP_FREERTOS_MESH_MODEL_TIME_AND_SCENES_SCHEDULER_SCHEDULER_COMMON_H_ */ 
/// @} MESH_scheduler_common_API

