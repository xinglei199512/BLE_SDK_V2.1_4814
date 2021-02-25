/**
 ****************************************************************************************
 *
 * @file   time_msg_handler.h
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

#ifndef APP_FREERTOS_MESH_MODEL_TIME_MSG_HANDLER_H_
#define APP_FREERTOS_MESH_MODEL_TIME_MSG_HANDLER_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "mesh_errors.h"
#include "sdk_mesh_config.h"
#include "sdk_mesh_definitions.h"
//#include "time_server.h"
#include "time_client.h"
#include "model_msg.h"

#define TIME_SERVER_MODEL_ID            0x1200
#define TIME_SETUP_SERVER_MODEL_ID      0x1201
#define TIME_CLIENT_MODEL_ID             0x1202
#define SCENE_SERVER_MODEL_ID           0x1203
#define SCENE_SETUP_SERVER_MODEL_ID     0x1204
#define SCENE_CLIENT_MODEL_ID            0x1205
#define SCHEDULER_SERVER_MODEL_ID       0x1206
#define SCHEDULER_SETUP_SERVER_MODEL_ID 0x1207
#define SCHEDULER_CLIENT_MODEL_ID        0x1208

#define TIME_AND_SCENE_ONE_OCTET_OPCODE_OFFSET   0x5C
enum Time_And_Scene_One_Octets_Opcode
{
    Time_Set,
    Time_Status,
    Scene_Status,
    Scheduler_Action_Status,
    Scheduler_Action_Set,
    Scheduler_Action_Set_Unacknowledged,
    Time_And_Scene_One_Opcode_MAX,
};
#define TIME_AND_SCENE_TWO_OCTETS_OPCODE_OFFSET 0x237
enum Time_And_Scene_Two_Octets_Opcode
{
    Time_Get,
    Time_Role_Get,
    Time_Role_Set,
    Time_Role_Status,
    Time_Zone_Get,
    Time_Zone_Set,
    Time_Zone_Status,
    TAI_UTC_Delta_Get,
    TAI_UTC_Delta_Set,
    TAI_UTC_Delta_Status,
    Scene_Get,
    Scene_Recall,
    Scene_Recall_Unacknowledged,
    Scene_Register_Get,
    Scene_Register_Status,
    Scene_Store,
    Scene_Store_Unacknowledged,
    Scheduler_Action_Get,
    Scheduler_Get,
    Scheduler_Status,
    Time_And_Scene_Two_Opcode_MAX,
};
#define SCENE_SETUP_TWO_OCTETS_OPCODE_OFFSET 0x29E
enum Scene_Setup_Two_Octets_Opcode
{
    Scene_Delete,
    Scene_Delete_Unacknowledged,
    Scene_Setup_Two_Opcode_MAX,
};

extern msg_handler_model_t time_and_scene_one_octet_model_msg[Time_And_Scene_One_Opcode_MAX];
extern msg_handler_model_t time_and_scene_two_octet_model_msg[Time_And_Scene_Two_Opcode_MAX];
extern msg_handler_model_t scene_setup_two_octet_model_msg[Scene_Setup_Two_Opcode_MAX];

#endif /* APP_FREERTOS_MESH_MODEL_TIME_MSG_HANDLER_H_ */ 

