/**
 ****************************************************************************************
 *
 * @file   light_ctl_msg_handler.h
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


#ifndef APP_FREERTOS_MESH_MODEL_LIGHT_MODE_LIGHT_CTL_LIGHT_CTL_MSG_HANDLER_H_
#define APP_FREERTOS_MESH_MODEL_LIGHT_MODE_LIGHT_CTL_LIGHT_CTL_MSG_HANDLER_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "mesh_errors.h"
#include "sdk_mesh_config.h"
#include "sdk_mesh_definitions.h"
#include "light_ctl_client.h"
#include "light_ctl_server.h"
#include "model_msg.h"

#define LIGHT_CTL_SERVER_MODEL_ID             0x1303
#define LIGHT_CTL_SETUP_SERVER_MODEL_ID       0x1304
#define LIGHT_CTL_CLIENT_MODEL_ID             0x1305
#define LIGHT_CTL_TEMPERATURE_SERVER_MODEL_ID 0x1306
#define LIGHT_CTL_TWO_OCTETS_OPCODE_OFFSET    0x25D
enum Light_CTL_Two_Octets_Opcode
{
    Light_CTL_Get,
    Light_CTL_Set,
    Light_CTL_Set_Unacknowledged,
    Light_CTL_Status,
    Light_CTL_Temperature_Get,
    Light_CTL_Temperature_Range_Get,
    Light_CTL_Temperature_Range_Status,
    Light_CTL_Temperature_Set,
    Light_CTL_Temperature_Set_Unacknowledged,
    Light_CTL_Temperature_Status,
    Light_CTL_Default_Get,
    Light_CTL_Default_Status,

    Light_CTL_Default_Set,
    Light_CTL_Default_Set_Unacknowledged,
    Light_CTL_Temperature_Range_Set,
    Light_CTL_Temperature_Range_Set_Unacknowledged,
    Light_CTL_Opcode_MAX,
};

extern msg_handler_model_t light_CTL_model_msg[Light_CTL_Opcode_MAX];

#endif /* APP_FREERTOS_MESH_MODEL_LIGHT_MODE_LIGHT_CTL_LIGHT_CTL_MSG_HANDLER_H_ */ 

