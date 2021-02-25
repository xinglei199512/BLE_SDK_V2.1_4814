/**
 ****************************************************************************************
 *
 * @file   scene_common.h
 *
 * @brief  .
 *
 * @author  ZHAOYUNLIU
 * @date    2018-12-21 10:24
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
 * @addtogroup MESH_scene_common_API Mesh scene_common API
 * @ingroup MESH_API
 * @brief Mesh scene_common  API
 *
 * @{
 ****************************************************************************************
 */

#ifndef APP_FREERTOS_MESH_MODEL_TIME_AND_SCENES_SCENE_SCENE_COMMON_H_
#define APP_FREERTOS_MESH_MODEL_TIME_AND_SCENES_SCENE_SCENE_COMMON_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "mesh_errors.h"
#include "sdk_mesh_config.h"
#include "sdk_mesh_definitions.h"
#include "scene_client.h"


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
    uint16_t scene_number;
}__attribute((packed))scence_store_t;

typedef struct
{
    uint16_t scene_number;
    uint8_t tid;
    uint8_t trans_time;
    uint8_t delay;
}__attribute((packed))scene_recall_t;

typedef struct
{
    uint16_t scene_number;
    uint8_t tid;
}__attribute((packed))scene_default_recall_t;

typedef struct
{
    uint8_t status_code;
    uint16_t current_scene;
    uint16_t target_scene;
    uint8_t remaining_time;
}__attribute((packed))scene_status_t;

typedef struct
{
    uint8_t status_code;
    uint16_t current_scene;
}__attribute((packed))scene_default_status_t;

typedef struct
{
    uint8_t status_code;
    uint16_t current_scene;
    uint16_t scenes[];
}__attribute((packed))scene_register_status_t;

typedef struct
{
    uint16_t scene_number;
}__attribute((packed))scene_delete_t;


#endif /* APP_FREERTOS_MESH_MODEL_TIME_AND_SCENES_SCENE_SCENE_COMMON_H_ */ 
/// @} MESH_scene_common_API

