/**
 ****************************************************************************************
 *
 * @file   light_hsl_msg_handler.h
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
#ifndef APP_FREERTOS_MESH_MODEL_LIGHT_MODE_LIGHT_HSL_LIGHT_HSL_MSG_HANDLER_H_
#define APP_FREERTOS_MESH_MODEL_LIGHT_MODE_LIGHT_HSL_LIGHT_HSL_MSG_HANDLER_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "mesh_errors.h"
#include "sdk_mesh_config.h"
#include "sdk_mesh_definitions.h"
#include "light_hsl_server.h"
#include "light_hsl_client.h"
#include "model_msg.h"

#define LIGHT_HSL_SERVER_MODEL_ID             0x1307
#define LIGHT_HSL_SETUP_SERVER_MODEL_ID       0x1308
#define LIGHT_HSL_CLIENT_MODEL_ID             0x1309
#define LIGHT_HSL_HUE_SERVER_MODEL_ID         0x130A
#define LIGHT_HSL_SATURATION_SERVER_MODEL_ID  0x130B
#define LIGHT_HSL_TWO_OCTETS_OPCODE_OFFSET    0x26D
enum Light_HSL_Two_Octets_Opcode
{
    Light_HSL_Get,
    Light_HSL_Hue_Get,
    Light_HSL_Hue_Set,
    Light_HSL_Hue_Set_Unacknowledged,
    Light_HSL_Hue_Status,
    Light_HSL_Saturation_Get,
    Light_HSL_Saturation_Set,
    Light_HSL_Saturation_Set_Unacknowledged,
    Light_HSL_Saturation_Status,
    Light_HSL_Set,
    Light_HSL_Set_Unacknowledged,
    Light_HSL_Status,
    Light_HSL_Target_Get,
    Light_HSL_Target_Status,
    Light_HSL_Default_Get,
    Light_HSL_Default_Status,
    Light_HSL_Range_Get,
    Light_HSL_Range_Status,

    Light_HSL_Default_Set,
    Light_HSL_Default_Set_Unacknowledged,
    Light_HSL_Range_Set,
    Light_HSL_Range_Set_Unacknowledged,
    Light_HSL_Opcode_MAX,
};

extern msg_handler_model_t light_HSL_model_msg[Light_HSL_Opcode_MAX];
#endif /* APP_FREERTOS_MESH_MODEL_LIGHT_MODE_LIGHT_HSL_LIGHT_HSL_MSG_HANDLER_H_ */ 

