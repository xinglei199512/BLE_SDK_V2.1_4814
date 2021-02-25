/**
 ****************************************************************************************
 *
 * @file   light_lightness_msg_handler.h
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

#ifndef APP_FREERTOS_MESH_MODEL_LIGHT_MODE_LIGHT_LIGHTNESS_LIGHT_LIGHTNESS_MSG_HANDLER_H_
#define APP_FREERTOS_MESH_MODEL_LIGHT_MODE_LIGHT_LIGHTNESS_LIGHT_LIGHTNESS_MSG_HANDLER_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "mesh_errors.h"
#include "sdk_mesh_config.h"
#include "sdk_mesh_definitions.h"
#include "light_lightness_client.h"
#include "light_lightness_server.h"
//#include "access.h"
#include "mesh_model.h"
#include "model_msg.h"

#define LIGHT_LIGHTNESS_SERVER_MODEL_ID          0x1300
#define LIGHT_LIGHTNESS_SETUP_SERVER_MODEL_ID    0x1301
#define LIGHT_LIGHTNESS_CLIENT_MODEL_ID          0x1302
#define LIGHT_LIGHTNESS_TWO_OCTETS_OPCODE_OFFSET 0x24B
enum Light_Lightness_Two_Octets_Opcode
{
    Light_Lightness_Get,
    Light_Lightness_Set,
    Light_Lightness_Set_Unacknowledged,
    Light_Lightness_Status,

    Light_Lightness_Linear_Get,
    Light_Lightness_Linear_Set,
    Light_Lightness_Linear_Set_Unacknowledged,
    Light_Lightness_Linear_Status,

    Light_Lightness_Last_Get,
    Light_Lightness_Last_Status,
    Light_Lightness_Default_Get,
    Light_Lightness_Default_Status,

    Light_Lightness_Range_Get,
    Light_Lightness_Range_Status,

    Light_Lightness_Default_Set,
    Light_Lightness_Default_Set_Unacknowledged,
    Light_Lightness_Range_Set,
    Light_Lightness_Range_Set_Unacknowledged,
    Light_Lightness_Opcode_MAX,
};

extern msg_handler_model_t light_lightness_model_msg[Light_Lightness_Opcode_MAX];

#endif /* APP_FREERTOS_MESH_MODEL_LIGHT_MODE_LIGHT_LIGHTNESS_LIGHT_LIGHTNESS_MSG_HANDLER_H_ */ 

