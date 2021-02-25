#include "foundation_msg_handler.h"
#include "health_server.h"
//#include "health_client.h"
#include "config_server.h"
#include "config_client.h"

msg_handler_model_t fd_model_msg_one_opcode[One_Octet_Opcode_Msg_Num_Max]=
{
    #if (MESH_MODEL_CONFIG_SERVER)
    [Config_AppKey_Add]                     = {config_appkey_add_rx,                CONFIGURATION_SERVER_MODEL_ID},
    [Config_AppKey_Update]                  = {config_appkey_update_rx,             CONFIGURATION_SERVER_MODEL_ID},
    [Config_Model_Publication_Set]          = {config_model_publication_set_rx,     CONFIGURATION_SERVER_MODEL_ID},
    #endif
    #if (MESH_MODEL_CONFIG_CLIENT)
    [Config_Composition_Data_Status]        = {config_composition_data_status_rx,   CONFIGURATION_CLIENT_MODEL_ID},
    [Config_Heartbeat_Publication_Status]   = {config_heartbeat_publication_status_rx, CONFIGURATION_CLIENT_MODEL_ID},
    #endif
    #if (MESH_MODEL_HEALTH_CLIENT)
    [Health_Current_Status]                 = {health_current_status_rx, HEALTH_CLIENT_MODEL_ID},
    [Health_Fault_Status]                   = {health_fault_status_rx, HEALTH_CLIENT_MODEL_ID},
    #endif
};

msg_handler_model_t fd_model_msg_two_opcode[Two_Octets_Opcode_Msg_Num_Max]=
{
    #if (MESH_MODEL_CONFIG_SERVER)
    [Config_AppKey_Delete]                  = {config_appkey_delete_rx,             CONFIGURATION_SERVER_MODEL_ID},
    [Config_AppKey_Get]                  = {config_appkey_get_rx,             CONFIGURATION_SERVER_MODEL_ID},
    [Config_Model_Publication_Get]          = {config_model_publication_get_rx,     CONFIGURATION_SERVER_MODEL_ID},
    [Config_Model_App_Bind]                 = {config_model_app_bind_rx,            CONFIGURATION_SERVER_MODEL_ID},
    [Config_Model_App_Unbind]               = {config_model_app_unbind_rx,          CONFIGURATION_SERVER_MODEL_ID},
    [Config_Model_Subscription_Add]         = {config_model_subscription_update_rx, CONFIGURATION_SERVER_MODEL_ID},
    [Config_Model_Subscription_Delete]      = {config_model_subscription_update_rx, CONFIGURATION_SERVER_MODEL_ID},
    [Config_Model_Subscription_Overwrite]   = {config_model_subscription_update_rx, CONFIGURATION_SERVER_MODEL_ID},
    [Config_Model_Subscription_Delete_All]  = {config_model_subscription_delete_all_rx,CONFIGURATION_SERVER_MODEL_ID},    
    [Config_Heartbeat_Subscription_Get]     = {config_heartbeat_subscription_get_rx, CONFIGURATION_SERVER_MODEL_ID},
    [Config_Heartbeat_Subscription_Set]     = {config_heartbeat_subscription_set_rx, CONFIGURATION_SERVER_MODEL_ID},
    [Config_Heartbeat_Publication_Get]     = {config_heartbeat_publication_get_rx, CONFIGURATION_SERVER_MODEL_ID},
    [Config_Heartbeat_Publication_Set]     = {config_heartbeat_publication_set_rx, CONFIGURATION_SERVER_MODEL_ID},
    [Config_Model_Publication_Virtual_Address_Set]          = {config_model_publication_vir_set_rx,      CONFIGURATION_SERVER_MODEL_ID},
    [Config_Model_Subscription_Virtual_Address_Add]         = {config_model_subscription_virt_update_rx, CONFIGURATION_SERVER_MODEL_ID},
    [Config_Model_Subscription_Virtual_Address_Delete]      = {config_model_subscription_virt_update_rx, CONFIGURATION_SERVER_MODEL_ID},
    [Config_Model_Subscription_Virtual_Address_Overwrite]   = {config_model_subscription_virt_update_rx, CONFIGURATION_SERVER_MODEL_ID},
    [Config_Relay_Get]                      = {config_relay_get_rx,                 CONFIGURATION_SERVER_MODEL_ID},
    [Config_Relay_Set]                      = {config_relay_set_rx,                 CONFIGURATION_SERVER_MODEL_ID},
    [Config_Friend_Get]                      = {config_friend_get_rx,                 CONFIGURATION_SERVER_MODEL_ID},
    [Config_Friend_Set]                      = {config_friend_set_rx,                 CONFIGURATION_SERVER_MODEL_ID},
    [Config_Low_Power_Node_PollTimeout_Get] = {config_low_power_node_polltimeout_get_rx , CONFIGURATION_SERVER_MODEL_ID},
    [Config_GATT_Proxy_Get]                      = {config_proxy_get_rx,                 CONFIGURATION_SERVER_MODEL_ID},
    [Config_GATT_Proxy_Set]                      = {config_proxy_set_rx,                 CONFIGURATION_SERVER_MODEL_ID},    
    [Config_NetKey_Update]                  = {config_server_update_netkey_rx,      CONFIGURATION_SERVER_MODEL_ID},
    [Config_NetKey_Add]                     = {config_netkey_add_rx,      CONFIGURATION_SERVER_MODEL_ID},
    [Config_NetKey_Delete]                     = {config_netkey_delete_rx,      CONFIGURATION_SERVER_MODEL_ID},
    [Config_NetKey_Get]                     = {config_netkey_get_rx,      CONFIGURATION_SERVER_MODEL_ID},
    [Config_Network_Transmit_Get]                  = {config_network_transmit_get_rx,      CONFIGURATION_SERVER_MODEL_ID},
    [Config_Network_Transmit_Set]                  = {config_network_transmit_set_rx,      CONFIGURATION_SERVER_MODEL_ID},
    [Config_Key_Refresh_Phase_Get]          = {handle_config_key_refresh_phase_get_rx,    CONFIGURATION_SERVER_MODEL_ID},
    [Config_Key_Refresh_Phase_Set]          = {handle_config_key_refresh_phase_set_rx,    CONFIGURATION_SERVER_MODEL_ID},
    [Config_Composition_Data_Get]           = {config_composition_data_get_rx,      CONFIGURATION_SERVER_MODEL_ID},
    [Config_Beacon_Get]           = {config_beacon_get_rx,      CONFIGURATION_SERVER_MODEL_ID},
    [Config_Beacon_Set]           = {config_beacon_set_rx,      CONFIGURATION_SERVER_MODEL_ID},    
    [Config_Default_TTL_Get]           = {config_default_ttl_get_rx,      CONFIGURATION_SERVER_MODEL_ID},
    [Config_Default_TTL_Set]           = {config_default_ttl_set_rx,      CONFIGURATION_SERVER_MODEL_ID},
    [Config_SIG_Model_App_Get]           = {config_SIG_model_app_get_rx,      CONFIGURATION_SERVER_MODEL_ID},
    [Config_Vendor_Model_App_Get]           = {config_vendor_model_app_get_rx,      CONFIGURATION_SERVER_MODEL_ID},
    [Config_SIG_Model_Subscription_Get]           = {config_SIG_model_subscription_get_rx,      CONFIGURATION_SERVER_MODEL_ID},
    [Config_Vendor_Model_Subscription_Get]           = {config_vendor_model_subscription_get_rx,      CONFIGURATION_SERVER_MODEL_ID},
    [Config_Node_Reset]                     = {config_server_node_reset_rx,         CONFIGURATION_SERVER_MODEL_ID},
    [Config_Node_Identity_Get]              = {config_server_node_identity_get_rx,         CONFIGURATION_SERVER_MODEL_ID},
    [Config_Node_Identity_Set]              = {config_server_node_identity_set_rx,         CONFIGURATION_SERVER_MODEL_ID},    
    #endif
    #if (MESH_MODEL_CONFIG_CLIENT)    
    [Config_AppKey_List]                  = {config_appkey_list_rx,             CONFIGURATION_SERVER_MODEL_ID},
    [Config_AppKey_Status]                  = {config_appkey_status_rx,             CONFIGURATION_CLIENT_MODEL_ID},
    [Config_Model_Publication_Status]       = {config_model_publication_status_rx,  CONFIGURATION_CLIENT_MODEL_ID},
    [Config_Model_App_Status]               = {config_model_app_status_rx,          CONFIGURATION_CLIENT_MODEL_ID},
    [Config_Model_Subscription_Status]      = {config_model_subscription_status_rx, CONFIGURATION_CLIENT_MODEL_ID},
    [Config_Heartbeat_Subscription_Status]  = {config_heartbeat_subscription_status_rx, CONFIGURATION_CLIENT_MODEL_ID},
    [Config_Relay_Status]                   = {config_relay_status_rx,              CONFIGURATION_CLIENT_MODEL_ID},
    [Config_Friend_Status]                   = {config_friend_status_rx,              CONFIGURATION_CLIENT_MODEL_ID},
    [Config_GATT_Proxy_Status]                   = {config_proxy_status_rx,              CONFIGURATION_CLIENT_MODEL_ID},
    [Config_NetKey_Status]                  = {config_client_net_key_status_rx,     CONFIGURATION_CLIENT_MODEL_ID},
    [Config_Network_Transmit_Status]               = {config_network_transmit_status_rx,   CONFIGURATION_CLIENT_MODEL_ID},
    [Config_Key_Refresh_Phase_Status]       = {config_key_refresh_phase_status_rx,        CONFIGURATION_CLIENT_MODEL_ID},
    [Config_Beacon_Status]           = {config_beacon_status_rx,      CONFIGURATION_CLIENT_MODEL_ID},
    [Config_Default_TTL_Status]        = {config_default_ttl_status_rx,      CONFIGURATION_CLIENT_MODEL_ID},
    [Config_Node_Identity_Status]           = {config_server_node_identity_status_rx,         CONFIGURATION_CLIENT_MODEL_ID},
    [Config_SIG_Model_App_List]           = {config_SIG_model_app_list_rx,      CONFIGURATION_SERVER_MODEL_ID},
    [Config_Vendor_Model_App_List]           = {config_vendor_model_app_list_rx,      CONFIGURATION_SERVER_MODEL_ID},
    [Config_SIG_Model_Subscription_List]           = {config_SIG_model_subscription_list_rx,      CONFIGURATION_SERVER_MODEL_ID},
    [Config_Vendor_Model_Subscription_List]           = {config_vendor_model_subscription_list_rx,      CONFIGURATION_SERVER_MODEL_ID},
    [Config_NetKey_List]                     = {config_netkey_list_rx,      CONFIGURATION_SERVER_MODEL_ID},
    #endif
    #if (MESH_MODEL_HEALTH_SERVER)     
    [Health_Attention_Get]       = {health_fault_attention_get_rx,  HEALTH_SERVER_MODEL_ID},
    [Health_Attention_Set]       = {health_fault_attention_set_rx,  HEALTH_SERVER_MODEL_ID},
    [Health_Attention_Set_Unacknowledged]       = {health_fault_attention_set_unacknowledged_rx,  HEALTH_SERVER_MODEL_ID},
    [Health_Fault_Clear]       = {health_fault_clear_rx, HEALTH_SERVER_MODEL_ID},
    [Health_Fault_Clear_Unacknowledged]       = {health_fault_clear_unacknowledged_rx,  HEALTH_SERVER_MODEL_ID},
    [Health_Fault_Get]       = {health_fault_get_rx,  HEALTH_SERVER_MODEL_ID},
    [Health_Fault_Test]       = {health_fault_test_rx,  HEALTH_SERVER_MODEL_ID},
    [Health_Fault_Test_Unacknowledged]       = {health_fault_test_unacknowledged_rx,  HEALTH_SERVER_MODEL_ID},
    [Health_Period_Get]       = {health_fault_period_get_rx,  HEALTH_SERVER_MODEL_ID},
    [Health_Period_Set]       = {health_fault_period_set_rx,  HEALTH_SERVER_MODEL_ID},
    [Health_Period_Set_Unacknowledged]       = {health_fault_period_set_unacknowledged_rx,  HEALTH_SERVER_MODEL_ID},
    #endif
    #if (MESH_MODEL_HEALTH_CLIENT)    
    [Health_Period_Status]       = {health_period_status_rx,  HEALTH_CLIENT_MODEL_ID},
    [Health_Attention_Status]       = {health_attention_status_rx,  HEALTH_CLIENT_MODEL_ID},
    #endif
};
