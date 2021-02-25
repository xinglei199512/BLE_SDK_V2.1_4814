#include "osapp_config.h"


#include "sdk_mesh_definitions.h"
#include "mesh_env.h"
#include "beacon.h"
#include "co_endian.h"
#include "mesh_iv_operation.h"
#include "mesh_iv_operation_ex.h"
#include "mesh_model.h"
#include "sdk_mesh_config_pro.h"

#include "device_keys_dm.h"
#include "network_keys_dm.h"
#include "app_keys_dm.h"
#include "config_server.h"
#include "mesh_kr_comm.h"
#include "mesh_kr_server.h"
#include "mesh_core_api.h"
#include "friend.h"
#include "low_power.h"
#ifdef MESH_UART_DEBUG_TEST_CMD
#include "uart_debug_test.h"
#endif /*MESH_UART_DEBUG_TEST_CMD*/

void net_beacon_relay_sec_beacon(uint8_t *network_id)
{
     mesh_core_params_t param_feature_relay ;
     mesh_core_params_get(MESH_CORE_PARAM_FEATURE_RELAY           , &param_feature_relay);
     if(param_feature_relay.relay == MESH_FEATURE_ENABLED)
     {
         beacon_send_by_net(network_id);
     }
}

uint32_t mesh_kr_netkey_update(dm_netkey_handle_t netkey_handle, const uint8_t * p_key)
{
    mesh_key_refresh_phase_t key_refresh_phase = MESH_KEY_REFRESH_PHASE_INVALID;
    net_key_box_t* keyinfo = NULL;
    if (NULL == p_key)
    {
        return MESH_CORE_ERROR_INVALID_PARAM;
    }
    if(MESH_CORE_SUCCESS != dm_netkey_get_update_info(netkey_handle, &keyinfo))
    {
        return MESH_CORE_ERROR_INVALID_PARAM;
    }
    if(MESH_CORE_SUCCESS != dm_netkey_kr_phase_get(netkey_handle,&key_refresh_phase))
    {
        return MESH_CORE_ERROR_INVALID_PARAM;
    }
    
    if (key_refresh_phase == MESH_KEY_REFRESH_PHASE_0)
    {
        void (*generate_credentials)(void*) = NULL;
        mesh_core_params_t low_power;
        mesh_core_params_get(MESH_CORE_PARAM_FEATURE_LOW_POWER   , &low_power);

        if(MESH_FEATURE_ENABLED !=low_power.low_power)
        {
            generate_credentials = generate_friend_credentials;
        }
        else
        {
            generate_credentials = low_power_generate_credentials;
        }
        dm_netkey_update(netkey_handle,p_key,generate_credentials);
        dm_netkey_kr_phase_set(netkey_handle,MESH_KEY_REFRESH_PHASE_1);
    }
    else
    {

       net_key_box_t *keyinfo =  NULL;
       if(MESH_CORE_SUCCESS != dm_netkey_get_update_info(netkey_handle, &keyinfo))
       {
           return  MESH_CORE_ERROR_INVALID_PARAM; 
       }
       if (NULL == keyinfo || (!(key_refresh_phase == MESH_KEY_REFRESH_PHASE_1 &&
            memcmp(keyinfo->netkey, p_key, MESH_KEY_LENGTH) == 0) ))
       {
            /* Network keys can only be updated if no key refresh is in progress for the new key
             * or Phase 1 for the same key.  messh ps :3.10. 4 */
            return MESH_CORE_ERROR_FORBIDDEN;

       }
   
    }
    return MESH_CORE_SUCCESS;
}


void net_beacon_kr_handle(uint8_t *network_id, bool key_refresh)
{
    err_t err = MESH_CORE_SUCCESS;
    dm_netkey_handle_t netkey_handle;
    net_key_box_t *keyinfo = NULL;
    mesh_key_refresh_phase_t key_refresh_phase = MESH_KEY_REFRESH_PHASE_INVALID;
    mesh_core_params_t param;
    mesh_core_params_get(MESH_CORE_PARAM_MESH_ROLES , &param);
    if(MESH_ROLE_CONFIG_SERVER != param.role)//config server
    {
        return ;
    }
    if(MESH_CORE_SUCCESS != dm_netkey_network_id_to_handle(network_id, &netkey_handle))
    {
        return;
    }
    if(MESH_CORE_SUCCESS != dm_netkey_get_update_info(netkey_handle, &keyinfo))
    {
        return;
    }
    err = dm_netkey_kr_phase_get(netkey_handle,&key_refresh_phase);
    if(MESH_CORE_SUCCESS != err)
    {
        return ;
    }
    net_beacon_relay_sec_beacon(network_id);
    if (memcmp(network_id,keyinfo->network_id,NETWORK_ID_LENGTH) == 0)
    {
        if (key_refresh)
        {
            if (key_refresh_phase == MESH_KEY_REFRESH_PHASE_1)
            {
                 dm_netkey_update_swap_keys(netkey_handle);
                 LOG(LOG_LVL_INFO,"kr_beacon  3\n");
                 friend_update_add_to_q_for_all(network_id);
            }
        }
        else
        {
            if (key_refresh_phase != MESH_KEY_REFRESH_PHASE_0)
            {
                dm_netkey_update_commit(netkey_handle);
                LOG(LOG_LVL_INFO,"kr_beacon done 0\n");
                friend_update_add_to_q_for_all(network_id);

            }
        }
    }
}


err_t mesh_kr_config_netkey_update(mesh_global_idx_t netkey_index, uint8_t *netkey)
{
     
     LOG(LOG_LVL_INFO,"mesh_kr_config_netkey_update\n");
     dm_netkey_handle_t nethandle;
     if(MESH_CORE_SUCCESS != dm_netkey_index_to_netkey_handle(netkey_index,&nethandle))
     {
         return MESH_CORE_ERROR_NOT_FOUND;
     }
     return mesh_kr_netkey_update(nethandle, netkey);
}


err_t mesh_kr_config_get_phase(mesh_global_idx_t netkey_index, mesh_key_refresh_phase_t  * p_phase)
{
    dm_netkey_handle_t netkey_handle;
    if(MESH_CORE_SUCCESS !=  dm_netkey_index_to_netkey_handle(netkey_index,&netkey_handle))
    {
        return MESH_CORE_ERROR_NOT_FOUND;
    }
    return  dm_netkey_kr_phase_get(netkey_handle,p_phase);
}

err_t mesh_kr_netkey_action(dm_netkey_handle_t p_nethandle, mesh_key_refresh_transition_t  transiton)
{
    mesh_key_refresh_phase_t key_refresh_phase = MESH_KEY_REFRESH_PHASE_INVALID;
    err_t err = dm_netkey_kr_phase_get(p_nethandle,&key_refresh_phase);
    if(MESH_CORE_SUCCESS != err)
    {
        return err;
    }
    switch (transiton)
    {
        case MESH_KEY_TRANSITION_TO_PHASE_2:
            if (key_refresh_phase == MESH_KEY_REFRESH_PHASE_2 || key_refresh_phase == MESH_KEY_REFRESH_PHASE_1)
            {
                if (key_refresh_phase == MESH_KEY_REFRESH_PHASE_1)
                {
                    dm_netkey_update_swap_keys(p_nethandle);
                    LOG(LOG_LVL_INFO,"kr_invoke_key 2\n");
                }
            break;
            }
           return MESH_CORE_ERROR_INVALID_PARAM;    
        case MESH_KEY_TRANSITION_TO_PHASE_3:
            if (key_refresh_phase != MESH_KEY_REFRESH_PHASE_3)
            {
                if (key_refresh_phase != MESH_KEY_REFRESH_PHASE_0)
                {
                    dm_netkey_update_commit(p_nethandle);
                    LOG(LOG_LVL_INFO,"kr_done_key 3\n");                
                 }
             break;
            }
            return MESH_CORE_ERROR_INVALID_PARAM;    
        default:
            return MESH_CORE_ERROR_INVALID_PARAM;    
   }
   return MESH_CORE_SUCCESS;
}


err_t mesh_kr_config_netkey_phase(mesh_global_idx_t netkey_index, mesh_key_refresh_transition_t  transiton)
{
    dm_netkey_handle_t nethandle;
    if(MESH_CORE_SUCCESS != dm_netkey_index_to_netkey_handle(netkey_index,&nethandle))
    {
        return MESH_CORE_ERROR_NOT_FOUND;
    }
    return mesh_kr_netkey_action(nethandle,transiton);
}


void mesh_provision_kr_handle(mesh_global_idx_t netkey_index, uint8_t *netkey,bool key_refresh)
{
    if(!key_refresh)
    {
        return;
    }
    mesh_kr_config_netkey_update(netkey_index,netkey);
    mesh_kr_config_netkey_phase(netkey_index,MESH_KEY_TRANSITION_TO_PHASE_2);
}

void mesh_kr_output_env(dm_netkey_handle_t * netkeylist,  uint32_t * p_counter)
{

     
     dm_get_all_netkey_handles((dm_netkey_handle_t *)netkeylist,p_counter);
     if(NULL == netkeylist[0])
     {
         return;
     }

#ifdef MESH_UART_DEBUG_TEST_CMD
     for (int i = 0; i < *p_counter; i++)
     {
         mesh_debug_uart_test_tx(netkeylist[i]->key[0].network_id,NETWORK_ID_LENGTH);
         mesh_debug_uart_test_tx(netkeylist[i]->key[0].netkey,MESH_KEY_LENGTH);
         
         mesh_debug_uart_test_tx(netkeylist[i]->key[1].network_id,NETWORK_ID_LENGTH);
         mesh_debug_uart_test_tx(netkeylist[i]->key[1].netkey,MESH_KEY_LENGTH);
     }
#endif/* MESH_UART_DEBUG_TEST_CMD */
}


