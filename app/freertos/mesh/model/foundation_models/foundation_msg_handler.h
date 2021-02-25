#ifndef FOUNDATION_MSG_HANDLER_H_
#define FOUNDATION_MSG_HANDLER_H_

//#include "config_server.h"
//#include "config_client.h"
//#include "health_server.h"
//#include "health_client.h"
#include "model_msg.h"

#define FOUNDATION_MODELS_ONE_OCTET_OPCODE_OFFSET 0x0

enum Foundation_Models_One_Octet_Opcode
{
    Config_AppKey_Add = 0,
    Config_AppKey_Update,
    Config_Composition_Data_Status,
    Config_Model_Publication_Set,
    Health_Current_Status,
    Health_Fault_Status,
    Config_Heartbeat_Publication_Status,
    One_Octet_Opcode_Msg_Num_Max,
};

#define FOUNDATION_MODELS_TWO_OCTETS_OPCODE_OFFSET 0x0
enum Foundation_Models_Two_Octets_Opcode
{
    Config_AppKey_Delete = 0x0,
    Config_AppKey_Get ,
    Config_AppKey_List,
    Config_AppKey_Status,
    Health_Attention_Get,
    Health_Attention_Set,
    Health_Attention_Set_Unacknowledged,
    Health_Attention_Status,
    Config_Composition_Data_Get,
    Config_Beacon_Get,
    Config_Beacon_Set,
    Config_Beacon_Status,
    Config_Default_TTL_Get,
    Config_Default_TTL_Set,
    Config_Default_TTL_Status,
    Config_Friend_Get,
    Config_Friend_Set = 0x10,
    Config_Friend_Status,
    Config_GATT_Proxy_Get,
    Config_GATT_Proxy_Set,
    Config_GATT_Proxy_Status,
    Config_Key_Refresh_Phase_Get,
    Config_Key_Refresh_Phase_Set,
    Config_Key_Refresh_Phase_Status,
    Config_Model_Publication_Get,
    Config_Model_Publication_Status,
    Config_Model_Publication_Virtual_Address_Set,
    Config_Model_Subscription_Add,
    Config_Model_Subscription_Delete,
    Config_Model_Subscription_Delete_All,
    Config_Model_Subscription_Overwrite,
    Config_Model_Subscription_Status,
    Config_Model_Subscription_Virtual_Address_Add = 0x20,
    Config_Model_Subscription_Virtual_Address_Delete,
    Config_Model_Subscription_Virtual_Address_Overwrite,
    Config_Network_Transmit_Get,
    Config_Network_Transmit_Set,
    Config_Network_Transmit_Status,
    Config_Relay_Get,
    Config_Relay_Set,
    Config_Relay_Status,
    Config_SIG_Model_Subscription_Get,
    Config_SIG_Model_Subscription_List,
    Config_Vendor_Model_Subscription_Get,
    Config_Vendor_Model_Subscription_List,
    Config_Low_Power_Node_PollTimeout_Get,
    Config_Low_Power_Node_PollTimeout_Status,
    Health_Fault_Clear,
    Health_Fault_Clear_Unacknowledged = 0x30,
    Health_Fault_Get,
    Health_Fault_Test,
    Health_Fault_Test_Unacknowledged,
    Health_Period_Get,
    Health_Period_Set,
    Health_Period_Set_Unacknowledged,
    Health_Period_Status,
    Config_Heartbeat_Publication_Get,
    Config_Heartbeat_Publication_Set,
    Config_Heartbeat_Subscription_Get,
    Config_Heartbeat_Subscription_Set,
    Config_Heartbeat_Subscription_Status,
    Config_Model_App_Bind,
    Config_Model_App_Status,
    Config_Model_App_Unbind,
    Config_NetKey_Add = 0x40,
    Config_NetKey_Delete,
    Config_NetKey_Get,
    Config_NetKey_List,
    Config_NetKey_Status,
    Config_NetKey_Update,
    Config_Node_Identity_Get,
    Config_Node_Identity_Set,
    Config_Node_Identity_Status,
    Config_Node_Reset,
    Config_Node_Reset_Status,
    Config_SIG_Model_App_Get,
    Config_SIG_Model_App_List,
    Config_Vendor_Model_App_Get,
    Config_Vendor_Model_App_List,
    Two_Octets_Opcode_Msg_Num_Max
};

extern msg_handler_model_t fd_model_msg_one_opcode[One_Octet_Opcode_Msg_Num_Max];

extern msg_handler_model_t fd_model_msg_two_opcode[Two_Octets_Opcode_Msg_Num_Max];

#endif
